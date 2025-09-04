
#pragma once

#include <assert.h>
#include <stdint.h>
#include "src/main/util/templates.hpp"

namespace renderer {

using Color = uint16_t;

static constexpr inline Color rgb565(int red, int green, int blue) {
	return 0
#if 0
		// Without endianness swap
		| ((red   & 31) <<  0)
		| ((green & 63) <<  5)
		| ((blue  & 31) << 11);
#else
		// With endianness swap
		| ((green & 63) >>  3)
		| ((red   & 31) <<  3)
		| ((blue  & 31) <<  8)
		| ((green & 63) >> 13);
#endif
}

static constexpr inline Color rgb(int red, int green, int blue) {
	return rgb565(
		((red   * 31) + 127) / 255,
		((green * 63) + 127) / 255,
		((blue  * 31) + 127) / 255
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

	inline Color *getBufferPtr(int x, int y) {
		assert((x >= 0) && (x < width_));
		assert((y >= 0) && (y < height_));

		auto ptr = buffers_[currentBuffer_].as<Color>();

		return &ptr[width_ * y + x];
	}

	void init(int width, int height);
	void release(void);
	void setClip(int x, int y, int width, int height);
	void resetClip(void);
	const Color *flip(void);

	void clear(Color color);
	void horizontalLine(int x, int y, int width, Color color);
	void verticalLine(int x, int y, int height, Color color);
	void fill(int x, int y, int width, int height, Color color);
};

}
