/*
 * 573in1 - Copyright (C) 2022-2025 spicyjpeg
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
#include <string.h>
#include "src/main/util/hash.hpp"
#include "src/main/util/templates.hpp"

namespace util {

/* String hashing (http://www.cse.yorku.ca/~oz/hash.html) */

Hash hash(const char *str, char terminator, Hash value) {
	auto _str = reinterpret_cast<const uint8_t *>(str);

	while (*_str && (*_str != terminator))
		value = Hash(*(_str++)) + (value << 6) + (value << 16) - value;

	return value;
}

Hash hash(const uint8_t *data, size_t length, Hash value) {
	for (; length > 0; length--)
		value = Hash(*(data++)) + (value << 6) + (value << 16) - value;

	return value;
}

}
