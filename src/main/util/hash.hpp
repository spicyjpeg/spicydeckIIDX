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

#pragma once

#include <stddef.h>
#include <stdint.h>

namespace util {

/* String hashing (http://www.cse.yorku.ca/~oz/hash.html) */

using Hash = uint32_t;

Hash hash(const char *str, char terminator = 0, Hash value = 0);
Hash hash(const uint8_t *data, size_t length, Hash value = 0);

/* Hash table parser */

template<typename T> static inline const T *getHashTableEntry(
	const T *table, size_t numBuckets, Hash id
) {
#if 0
	auto index = id % numBuckets;
#else
	auto index = id & (numBuckets - 1);
#endif

	do {
		auto entry = &table[index];
		index      = entry->getChained();

		if (entry->getHash() == id)
			return entry;
	} while (index);

	return nullptr;
}

}

/* String hashing operator */

static consteval inline util::Hash operator""_h(
	const char *literal,
	size_t     length
) {
	util::Hash value = 0;

	for (; length > 0; length--)
		value = util::Hash(*(literal++)) + (value << 6) + (value << 16) - value;

	return value;
}
