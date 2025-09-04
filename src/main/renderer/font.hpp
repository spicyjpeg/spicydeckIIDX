
#pragma once

#include <stdint.h>
#include "src/main/renderer/renderer.hpp"
#include "src/main/util/hash.hpp"
#include "src/main/util/string.hpp"
#include "src/main/util/templates.hpp"

namespace renderer {

/* Font (.sft) file structures */

struct [[gnu::packed]] SFTHeader {
public:
	uint32_t magic;
	uint8_t  spaceWidth, tabWidth, lineHeight;
	int8_t   baselineOffset;
	uint16_t numBuckets, numEntries;

	inline bool validate(void) const {
		return (magic == "SFT1"_c);
	}
};

struct [[gnu::packed]] SFTEntry {
public:
	util::UTF8CodePoint codePoint;
	uint16_t            offset, chained;

	inline util::Hash getHash(void) const {
		return codePoint;
	}
	inline uint32_t getChained(void) const {
		return chained;
	}
};

struct SFTGlyph {
public:
	uint8_t  width, height;
	uint16_t columns[];
};

/* Font class */

class Font : private util::Data {
private:
	inline const SFTGlyph *getGlyph_(uint16_t offset) const {
		auto data = as<uint8_t>();

		return reinterpret_cast<const SFTGlyph *>(&data[offset]);
	}

	const SFTEntry *get_(util::UTF8CodePoint ch) const;

public:
	inline void release(void) {
		destroy();
	}

	void initDefault(void);
	bool initFromFile(const char *path);

	void draw(
		Renderer   &renderer,
		int        x,
		int        y,
		int        width,
		int        height,
		const char *str,
		Color      color,
		bool       wordWrap = false
	) const;
	int getCharacterWidth(util::UTF8CodePoint ch) const;
	int getStringWidth(const char *str, bool breakOnSpace = false) const;
};

}
