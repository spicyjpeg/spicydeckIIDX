/*
 * 573in1 - Copyright (C) 2022-2024 spicyjpeg
 *
 * 573in1 is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * 573in1 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * 573in1. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

namespace util {

/* String manipulation */

extern const char HEX_CHARSET[];

size_t hexValueToString(char *output, uint32_t value, size_t numDigits = 8);
size_t hexToString(
	char          *output,
	const uint8_t *input,
	size_t        length,
	char          separator = 0
);

/* UTF-8 parser */

using UTF8CodePoint = uint32_t;

struct UTF8Character {
public:
	UTF8CodePoint codePoint;
	size_t        length;
};

UTF8Character parseUTF8Character(const char *ch);
size_t getUTF8StringLength(const char *str);

/* LZ4 decompressor */

static inline size_t getLZ4InPlaceMargin(size_t inputLength) {
	return (inputLength >> 8) + 32;
}

void decompressLZ4(
	uint8_t       *output,
	const uint8_t *input,
	size_t        maxOutputLength,
	size_t        inputLength
);

}
