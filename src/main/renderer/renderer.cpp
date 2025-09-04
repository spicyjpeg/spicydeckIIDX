
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include "driver/spi_master.h"
#include "src/main/renderer/renderer.hpp"
#include "src/main/util/templates.hpp"
#include "src/main/defs.hpp"

namespace renderer {

/* Optimized memory filling */

static void memset16_(uint16_t *dest, uint16_t value, size_t count) {
	if (!count)
		return;

	// Ensure the pointer is aligned to 32 bits in order to set two pixels at a
	// time.
	if (uintptr_t(dest) % alignof(uint32_t)) {
		*(dest++) = value;
		count--;
	}

	auto dest32  = reinterpret_cast<uint32_t *>(dest);
	auto value32 = util::mirror4(value);

	for (; count >= 8; count -= 8, dest += 8) {
		dest[0] = value32;
		dest[1] = value32;
		dest[2] = value32;
		dest[3] = value32;
		dest[4] = value32;
		dest[5] = value32;
		dest[6] = value32;
		dest[7] = value32;
	}

	dest = reinterpret_cast<uint16_t *>(dest32);

	for (; count > 0; count--)
		*(dest++) = value;
}

/* Simple software renderer */

void Renderer::init(int width, int height) {
	assert((width > 0) && (height > 0));

	const size_t length = width * height * sizeof(Color);

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
	}

	currentBuffer_ = 0;
	resetClip();
}

void Renderer::release(void) {
	for (auto &buffer : buffers_)
		buffer.destroy();
}

void Renderer::setClip(int x, int y, int width, int height) {
	clipX1_ = uint16_t(util::clamp<int>(x,          0, width_));
	clipY1_ = uint16_t(util::clamp<int>(y,          0, height_));
	clipX2_ = uint16_t(util::clamp<int>(x + width,  0, width_));
	clipY2_ = uint16_t(util::clamp<int>(y + height, 0, height_));
}

void Renderer::resetClip(void) {
	clipX1_ = 0;
	clipY1_ = 0;
	clipX2_ = width_;
	clipY2_ = height_;
}

const Color *Renderer::flip(void) {
	auto oldBuffer  = getBufferPtr(0, 0);
	currentBuffer_ ^= 1;

	return oldBuffer;
}

void Renderer::clear(Color color) {
	memset16_(getBufferPtr(0, 0), color, width_ * height_);
}

void Renderer::horizontalLine(int x, int y, int width, Color color) {
	if ((y < clipY1_) || (y > clipY2_))
		return;

	int x1 = util::clamp<int>(x,         clipX1_, clipX2_);
	int x2 = util::clamp<int>(x + width, clipX1_, clipX2_);

	memset16_(getBufferPtr(x1, y), color, x2 - x1);
}

void Renderer::verticalLine(int x, int y, int height, Color color) {
	if ((x < clipX1_) || (x > clipX2_))
		return;

	int y1 = util::clamp<int>(x,          clipY1_, clipY2_);
	int y2 = util::clamp<int>(y + height, clipY1_, clipY2_);

	auto ptr = getBufferPtr(x, y1);

	for (; y1 < y2; y1++) {
		*ptr = color;
		ptr += width_;
	}
}

void Renderer::fill(int x, int y, int width, int height, Color color) {
	int x1 = util::clamp<int>(x,          clipX1_, clipX2_);
	int x2 = util::clamp<int>(x + width,  clipX1_, clipX2_);
	int y1 = util::clamp<int>(x,          clipY1_, clipY2_);
	int y2 = util::clamp<int>(y + height, clipY1_, clipY2_);

	auto ptr    = getBufferPtr(x1, y1);
	int  length = x2 - x1;

	for (; y1 < y2; y1++) {
		memset16_(ptr, color, length);
		ptr += width_;
	}
}

}
