
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "src/main/renderer/font.hpp"
#include "src/main/renderer/renderer.hpp"
#include "src/main/util/hash.hpp"
#include "src/main/util/string.hpp"
#include "src/main/defs.hpp"

namespace renderer {

static const char TAG_[]{ "font" };

/* Font class */

// There is basically no way to leverage #embed or the .incbin directive when
// using the Arduino IDE due to the way its build system works, so the default
// font has to be included here as a C array.
static const uint8_t defaultFont[]{
#include "assets/font.inc"
};

static constexpr util::UTF8CodePoint FONT_INVALID_CHAR_ = 0xfffd;

void Font::initDefault(void) {
	destroy();

	// HACK: a const void * -> void * cast is needed here (but the default font
	// is not going to be modified nor deallocated)
	ptr          = reinterpret_cast<void *>(uintptr_t(defaultFont));
	length       = sizeof(defaultFont);
	destructible = false;

	assert(as<SFTHeader>()->validate());
}

bool Font::initFromFile(const char *path) {
	auto   file = fopen(path, "rb");
	size_t fileLength;

	if (!file) {
		ESP_LOGE(TAG_, "could not open .sft file: %s", path);
		return false;
	}
	if (fseek(file, 0, SEEK_END)) {
		ESP_LOGE(TAG_, "could not determine .sft file size: %s", path);
		goto cleanup;
	}

	fileLength = ftell(file);

	if (!allocate(fileLength)) {
		ESP_LOGE(TAG_, "could not allocate %u bytes for .sft file: %s", fileLength, path);
		goto cleanup;
	}
	if (fread(ptr, fileLength, 1, file)) {
		ESP_LOGE(TAG_, "could not read .sft file: %s", path);
		goto cleanup;
	}
	if (!as<SFTHeader>()->validate()) {
		ESP_LOGE(TAG_, "not a valid .sft file: %s", path);
		goto cleanup;
	}

	return true;

cleanup:
	fclose(file);
	destroy();
	return false;
}

IRAM_ATTR const SFTEntry *Font::get_(util::UTF8CodePoint ch) const {
	auto header = as<SFTHeader>();
	auto entry  = util::getHashTableEntry(
		reinterpret_cast<const SFTEntry *>(&header[1]),
		header->numBuckets,
		ch
	);

	if (entry)
		return entry;
	else if (ch != FONT_INVALID_CHAR_)
		return get_(FONT_INVALID_CHAR_);
	else
		return nullptr;
}

IRAM_ATTR void Font::draw(
	Renderer   &renderer,
	int        x,
	int        y,
	int        width,
	int        height,
	const char *str,
	Color      color,
	bool       wordWrap
) const {
	if (!str)
		return;

	auto header = as<SFTHeader>();

	int currentX  = x;
	int boundaryX = x + width;
	int clipX1    = renderer.getClipX1();
	int clipX2    = renderer.getClipX2();

	int currentY  = header->baselineOffset + y;
	int boundaryY = header->baselineOffset + (y + height) - header->lineHeight;
	int clipY1    = header->baselineOffset + renderer.getClipY1();
	int clipY2    = header->baselineOffset + renderer.getClipY2();

	for (;;) {
		auto ch   = util::parseUTF8Character(str);
		bool wrap = wordWrap;
		str      += ch.length;

		switch (ch.codePoint) {
			case 0:
				return;

			case '\t':
				currentX += header->tabWidth;
				currentX -= currentX % header->tabWidth;
				break;

			case '\n':
				currentX  = x;
				currentY += header->lineHeight;
				break;

			case '\r':
				currentX = x;
				break;

			case ' ':
				currentX += header->spaceWidth;
				break;

			default:
				if (y >= clipY2)
					return;

				auto entry = get_(ch.codePoint);

				if (entry) {
					auto glyph = getGlyph_(entry->offset);
					auto ptr   = glyph->columns;

					for (int i = glyph->width; i > 0; i--, currentX++) {
						if ((currentX < clipX1) || (currentX >= clipX2))
							continue;

						auto buffer = renderer.getBufferPtr(currentX, currentY);
						auto column = *(ptr++);
						int  lastY  = currentY + glyph->height;

						for (int j = currentY; j < lastY; j++) {
							if ((column & 1) && (j >= clipY1) && (j < clipY2))
								*buffer = color;

							buffer  += renderer.getWidth();
							column >>= 1;
						}
					}
				}

				wrap = false;
		}

		// Handle word wrapping by calculating the length of the next word and
		// checking if it can still fit in the current line.
		int bound = boundaryX;

		if (wrap)
			bound -= getStringWidth(str, true);

		if (currentX > bound) {
			currentX  = x;
			currentY += header->lineHeight;
		}
		if (currentY > boundaryY)
			return;
	}
}

IRAM_ATTR int Font::getCharacterWidth(util::UTF8CodePoint ch) const {
	switch (ch) {
		case 0:
		case '\n':
		case '\r':
			return 0;

		case '\t':
			return as<SFTHeader>()->tabWidth;

		case ' ':
			return as<SFTHeader>()->spaceWidth;

		default:
			auto entry = get_(ch);

			return entry ? getGlyph_(entry->offset)->width : 0;
	}
}

IRAM_ATTR int Font::getStringWidth(const char *str, bool breakOnSpace) const {
	if (!str)
		return 0;

	auto header = as<SFTHeader>();

	int width = 0, maxWidth = 0;

	for (;;) {
		auto ch = util::parseUTF8Character(str);
		str    += ch.length;

		switch (ch.codePoint) {
			case 0:
				goto _break;

			case '\t':
				if (breakOnSpace)
					goto _break;

				width += header->tabWidth;
				width -= width % header->tabWidth;
				break;

			case '\n':
			case '\r':
				if (breakOnSpace)
					goto _break;
				if (width > maxWidth)
					maxWidth = width;

				width = 0;
				break;

			case ' ':
				if (breakOnSpace)
					goto _break;

				width += header->spaceWidth;
				break;

			default:
				auto entry = get_(ch.codePoint);

				if (entry)
					width += getGlyph_(entry->offset)->width;
		}
	}

_break:
	if (width > maxWidth)
		maxWidth = width;

	return maxWidth;
}

}
