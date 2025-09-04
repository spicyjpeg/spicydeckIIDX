
#pragma once

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

namespace util {

/* Misc. template utilities */

template<typename T> static constexpr inline uint32_t sum(
	const T *data,
	size_t  length
) {
	uint32_t value = 0;

	for (; length > 0; length--)
		value += uint32_t(*(data++));

	return value;
}

template<typename T> static constexpr inline T bitwiseXOR(
	const T *data,
	size_t  length
) {
	T value = 0;

	for (; length > 0; length--)
		value ^= *(data++);

	return value;
}

template<typename T> static constexpr inline bool isEmpty(
	const T *data,
	size_t  length,
	T       value = 0
) {
	for (; length > 0; length--) {
		if (*(data++) != value)
			return false;
	}

	return true;
}

template<typename T> static constexpr inline T min(T a, T b) {
	return (a < b) ? a : b;
}

template<typename T> static constexpr inline T max(T a, T b) {
	return (a > b) ? a : b;
}

template<typename T> static constexpr inline T clamp(
	T value,
	T minValue,
	T maxValue
) {
	if (value < minValue)
		return minValue;
	if (value > maxValue)
		return maxValue;

	return value;
}

template<typename T> static constexpr inline T rotateLeft(T value, int amount) {
	return T((value << amount) | (value >> (sizeof(T) * 8 - amount)));
}

template<typename T> static constexpr inline T rotateRight(T value, int amount) {
	return T((value >> amount) | (value << (sizeof(T) * 8 - amount)));
}

template<typename T> static constexpr inline T modulo(T num, T den) {
	T value = num % den;

	if (value < 0)
		value += den;

	return value;
}

template<typename T> static constexpr inline T truncateToMultiple(
	T value,
	T length
) {
	return value - modulo(value, length);
}

template<typename T> static constexpr inline T roundUpToMultiple(
	T value,
	T length
) {
	T diff = modulo(value, length);

	if (diff)
		value += length - diff;

	return value;
}

template<typename T, typename X> static inline void assertAligned(X *ptr) {
	assert(!(reinterpret_cast<uintptr_t>(ptr) % alignof(T)));
}

template<typename T> static inline void clear(T &obj, uint8_t value = 0) {
	__builtin_memset(&obj, value, sizeof(obj));
}

template<typename T> static inline void copy(T &dest, const T &source) {
	__builtin_memcpy(&dest, &source, sizeof(source));
}

template<typename T> static inline void copy(
	T       *dest,
	const T *source,
	size_t  count
) {
	__builtin_memcpy(dest, source, sizeof(T) * count);
}

template<typename T> static constexpr inline size_t countOf(T &array) {
	return sizeof(array) / sizeof(array[0]);
}

template<typename T> static constexpr inline auto *endOf(T &array) {
	return &array[countOf(array)];
}

}
