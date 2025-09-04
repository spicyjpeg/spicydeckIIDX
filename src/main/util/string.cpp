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

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "src/main/util/string.hpp"
#include "src/main/util/templates.hpp"

namespace util {

/* String manipulation */

const char HEX_CHARSET[]{ "0123456789ABCDEF" };

size_t hexValueToString(char *output, uint32_t value, size_t numDigits) {
	output += numDigits;
	*output = 0;

	for (size_t i = numDigits; i; i--, value >>= 4)
		*(--output) = HEX_CHARSET[value & 0xf];

	return numDigits;
}

size_t hexToString(
	char          *output,
	const uint8_t *input,
	size_t        length,
	char          separator
) {
	size_t outLength = 0;

	for (; length; length--) {
		uint8_t value = *(input++);

		*(output++) = HEX_CHARSET[value >> 4];
		*(output++) = HEX_CHARSET[value & 0xf];

		if (separator && (length > 1)) {
			*(output++) = separator;
			outLength  += 3;
		} else {
			outLength  += 2;
		}
	}

	*output = 0;
	return outLength;
}

/* UTF-8 parser */

#define L1(length)  (length)
#define L2(length)  L1(length), L1(length)
#define L4(length)  L2(length), L2(length)
#define L8(length)  L4(length), L4(length)
#define L16(length) L8(length), L8(length)

static const uint8_t _START_BYTE_LENGTHS[]{
	L16(1), // 0xxxx--- (1 byte)
	L8 (0), // 10xxx--- (invalid)
	L4 (2), // 110xx--- (2 bytes)
	L2 (3), // 1110x--- (3 bytes)
	L1 (4), // 11110--- (4 bytes)
	L1 (0)  // 11111--- (invalid)
};

static const uint8_t _START_BYTE_MASKS[]{
	0x00,
	0x7f, // 0xxxxxxx (1 byte)
	0x1f, // 110xxxxx (2 bytes)
	0x0f, // 1110xxxx (3 bytes)
	0x07  // 11110xxx (4 bytes)
};

UTF8Character parseUTF8Character(const char *ch) {
	auto start  = uint8_t(*(ch++));
	auto length = _START_BYTE_LENGTHS[start >> 3];
	auto mask   = _START_BYTE_MASKS[length];

	auto codePoint = UTF8CodePoint(start & mask);

	for (int i = length - 1; i > 0; i--) {
		codePoint <<= 6;
		codePoint  |= *(ch++) & 0x3f;
	}

	return { codePoint, length };
}

size_t getUTF8StringLength(const char *str) {
	for (size_t length = 0;; length++) {
		auto value = parseUTF8Character(str);

		if (!value.length) { // Invalid character
			str++;
			continue;
		}

		if (!value.codePoint) // Null character
			return length;

		str += value.length;
	}
}

/* LZ4 decompressor */

void decompressLZ4(
	uint8_t       *output,
	const uint8_t *input,
	size_t        maxOutputLength,
	size_t        inputLength
) {
	auto outputEnd = &output[maxOutputLength];
	auto inputEnd  = &input[inputLength];

	while (input < inputEnd) {
		uint8_t token = *(input++);

		// Copy literals from the input stream.
		int literalLength = token >> 4;

		if (literalLength == 0xf) {
			uint8_t addend;

			do {
				addend         = *(input++);
				literalLength += addend;
			} while (addend == 0xff);
		}

		for (; literalLength && (output < outputEnd); literalLength--)
			*(output++) = *(input++);
		if (input >= inputEnd)
			break;

		int offset = input[0] | (input[1] << 8);
		input     += 2;

		// Copy from previously decompressed data. Note that this *must* be done
		// one byte at a time, as the compressor relies on out-of-bounds copies
		// repeating the last byte.
		int copyLength = token & 0xf;

		if (copyLength == 0xf) {
			uint8_t addend;

			do {
				addend      = *(input++);
				copyLength += addend;
			} while (addend == 0xff);
		}

		auto copySource = output - offset;
		copyLength     += 4;

		for (; copyLength && (output < outputEnd); copyLength--)
			*(output++) = *(copySource++);
	}
}

}
