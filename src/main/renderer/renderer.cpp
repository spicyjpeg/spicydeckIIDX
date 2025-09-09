
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include "driver/spi_master.h"
#include "src/main/renderer/renderer.hpp"
#include "src/main/util/templates.hpp"
#include "src/main/defs.hpp"

namespace renderer {

/* Optimized memory filling */

IRAM_ATTR static void memset16_(uint16_t *dest, uint16_t value, size_t count) {
	if (count >= 32) {
		// Ensure the pointer is aligned to 32 bits in order to set two pixels
		// at a time.
		if (uintptr_t(dest) % alignof(uint32_t)) {
			*(dest++) = value;
			count--;
		}

		auto dest32  = reinterpret_cast<uint32_t *>(dest);
		auto value32 = util::mirror4(value);

		for (; count >= 16; count -= 16, dest32 += 8) {
			dest32[0] = value32;
			dest32[1] = value32;
			dest32[2] = value32;
			dest32[3] = value32;
			dest32[4] = value32;
			dest32[5] = value32;
			dest32[6] = value32;
			dest32[7] = value32;
		}

		dest = reinterpret_cast<uint16_t *>(dest32);
	}

	for (; count > 0; count--)
		*(dest++) = value;
}

/* Simple software renderer */

void Renderer::init(int width, int height) {
	assert((width > 0) && (height > 0));

	const size_t length = width * height * sizeof(RGB565);

	width_  = uint16_t(width);
	height_ = uint16_t(height);

	for (auto &buffer : buffers_) {
#if 0
		buffer.allocate(length);
#else
		buffer.destroy();

		buffer.ptr          =
			spi_bus_dma_memory_alloc(defs::DISPLAY_SPI_HOST, length, 0);
		buffer.length       = length;
		buffer.destructible = true;
#endif
		assert(buffer.ptr);
	}

	currentBuffer_ = 0;
	resetClip();
}

void Renderer::release(void) {
	for (auto &buffer : buffers_)
		buffer.destroy();
}

void Renderer::setClip(int x, int y, int w, int h) {
	clipX1_ = uint16_t(util::clamp<int>(x,     0, width_));
	clipY1_ = uint16_t(util::clamp<int>(y,     0, height_));
	clipX2_ = uint16_t(util::clamp<int>(x + w, 0, width_));
	clipY2_ = uint16_t(util::clamp<int>(y + h, 0, height_));
}

void Renderer::resetClip(void) {
	clipX1_ = 0;
	clipY1_ = 0;
	clipX2_ = width_;
	clipY2_ = height_;
}

IRAM_ATTR RGB565 *Renderer::getBufferPtr(int x, int y) {
	assert((x >= 0) && (x < width_));
	assert((y >= 0) && (y < height_));

	auto ptr = buffers_[currentBuffer_].as<RGB565>();

	return &ptr[width_ * y + x];
}

IRAM_ATTR const RGB565 *Renderer::flip(void) {
	auto oldBuffer  = buffers_[currentBuffer_].as<RGB565>();
	currentBuffer_ ^= 1;

	return oldBuffer;
}

IRAM_ATTR void Renderer::clear(RGB565 color) {
	auto ptr = buffers_[currentBuffer_].as<RGB565>();

	memset16_(ptr, color, width_ * height_);
}

IRAM_ATTR void Renderer::horizontalLine(int x, int y, int w, RGB565 color) {
	if ((y < clipY1_) || (y >= clipY2_))
		return;

	const int x1 = util::clamp<int>(x,     clipX1_, clipX2_);
	const int x2 = util::clamp<int>(x + w, clipX1_, clipX2_);

	memset16_(getBufferPtr(x1, y), color, x2 - x1);
}

IRAM_ATTR void Renderer::verticalLine(int x, int y, int h, RGB565 color) {
	if ((x < clipX1_) || (x >= clipX2_))
		return;

	int       y1 = util::clamp<int>(x,     clipY1_, clipY2_);
	const int y2 = util::clamp<int>(y + h, clipY1_, clipY2_);

	auto ptr = getBufferPtr(x, y1);

	for (; y1 < y2; y1++) {
		*ptr = color;
		ptr += width_;
	}
}

IRAM_ATTR void Renderer::line(int x1, int y1, int x2, int y2, RGB565 color) {
	const int deltaX = x2 - x1;
	const int deltaY = y2 - y1;

	const int distX = (deltaX >= 0) ? deltaX : (-deltaX);
	const int stepX = (deltaX >= 0) ? 1 : -1;
	const int distY = (deltaY >= 0) ? deltaY : (-deltaY);
	const int stepY = (deltaY >= 0) ? 1 : -1;

	auto      ptr    = getBufferPtr(x1, y1);
	const int stride = width_ * stepY;

	if (distX >= distY) {
		for (int error = distX / 2;;) {
			if (isDrawable(x1, y1))
				*ptr = color;
			if (x1 == x2)
				break;

			ptr   += stepX;
			x1    += stepX;
			error -= distY;

			if (error < 0) {
				ptr   += stride;
				y1    += stepY;
				error += distX;
			}
		}
	} else {
		for (int error = distY / 2;;) {
			if (isDrawable(x1, y1))
				*ptr = color;
			if (y1 == y2)
				break;

			ptr   += stride;
			y1    += stepY;
			error -= distX;

			if (error < 0) {
				ptr   += stepX;
				x1    += stepX;
				error += distY;
			}
		}
	}
}

IRAM_ATTR void Renderer::fill(int x, int y, int w, int h, RGB565 color) {
	const int x1 = util::clamp<int>(x,     clipX1_, clipX2_);
	const int x2 = util::clamp<int>(x + w, clipX1_, clipX2_);
	int       y1 = util::clamp<int>(x,     clipY1_, clipY2_);
	const int y2 = util::clamp<int>(y + h, clipY1_, clipY2_);

	auto      ptr    = getBufferPtr(x1, y1);
	const int length = x2 - x1;

	for (; y1 < y2; y1++) {
		memset16_(ptr, color, length);
		ptr += width_;
	}
}

}
