
#pragma once

#include <stdint.h>
#include "src/main/util/templates.hpp"

namespace renderer {

/* Color handling */

using RGB888 = uint32_t;
using RGB565 = uint16_t;

static constexpr inline RGB888 rgb(int r, int g, int b) {
	return 0
		| ((r & 255) <<  0)
		| ((g & 255) <<  8)
		| ((b & 255) << 16);
}

static constexpr inline RGB565 rgb565(int r, int g, int b) {
	return 0
#if 0
		// Without endianness swap
		| ((r & 31) <<  0)
		| ((g & 63) <<  5)
		| ((b & 31) << 11);
#else
		// With endianness swap
		| ((g & 63) >>  3)
		| ((r & 31) <<  3)
		| ((b & 31) <<  8)
		| ((g & 63) >> 13);
#endif
}

static constexpr inline RGB565 rgb888to565(RGB888 color) {
	const int r = (color >>  0) & 255;
	const int g = (color >>  8) & 255;
	const int b = (color >> 16) & 255;

	return rgb565(
		((r * 31) + 127) / 255,
		((g * 63) + 127) / 255,
		((b * 31) + 127) / 255
	);
}

/* Simple software renderer */

class Renderer {
private:
	uint16_t width_, height_;
	uint16_t clipX1_, clipX2_, clipY1_, clipY2_;

	util::Data buffers_[2];
	int        currentBuffer_;

public:
	inline Renderer(void) :
		width_(0),
		height_(0),
		currentBuffer_(0)
	{}
	inline ~Renderer(void) {
		release();
	}

	inline int getWidth(void)  const { return width_; }
	inline int getHeight(void) const { return height_; }
	inline int getClipX1(void) const { return clipX1_; }
	inline int getClipX2(void) const { return clipX2_; }
	inline int getClipY1(void) const { return clipY1_; }
	inline int getClipY2(void) const { return clipY2_; }
	inline bool isDrawable(int x, int y) const {
		return true
			&& (x >= clipX1_)
			&& (x <  clipX2_)
			&& (y >= clipY1_)
			&& (y <  clipY2_);
	}

	inline void clear(RGB888 color) {
		clear(rgb888to565(color));
	}
	inline void horizontalLine(int x, int y, int w, RGB888 color) {
		horizontalLine(x, y, w, rgb888to565(color));
	}
	inline void verticalLine(int x, int y, int h, RGB888 color) {
		verticalLine(x, y, h, rgb888to565(color));
	}
	inline void line(int x1, int y1, int x2, int y2, RGB888 color) {
		line(x1, y1, x2, y1, rgb888to565(color));
	}
	inline void fill(int x, int y, int w, int h, RGB888 color) {
		fill(x, y, w, h, rgb888to565(color));
	}

	void init(int width, int height);
	void release(void);
	void setClip(int x, int y, int w, int h);
	void resetClip(void);
	RGB565 *getBufferPtr(int x, int y);
	const RGB565 *flip(void);

	void clear(RGB565 color);
	void horizontalLine(int x, int y, int w, RGB565 color);
	void verticalLine(int x, int y, int h, RGB565 color);
	void line(int x1, int y1, int x2, int y2, RGB565 color);
	void fill(int x, int y, int w, int h, RGB565 color);
};

}
