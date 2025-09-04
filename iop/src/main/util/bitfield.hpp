
#pragma once

#include <stddef.h>

namespace util {

/* Bitfield manipulation utilities */

template<typename T> static constexpr inline T bitwiseTernary(
	T mask,
	T trueBits,
	T falseBits
) {
	return (trueBits & mask) | (falseBits & ~mask);
}

template<typename T> static constexpr inline T reduceAND(
	const T *data,
	size_t  length
) {
	T output = 0;

	for (; length > 0; length--)
		output &= *(data++);

	return output;
}

template<typename T> static constexpr inline T reduceOR(
	const T *data,
	size_t  length
) {
	T output = 0;

	for (; length > 0; length--)
		output |= *(data++);

	return output;
}

template<typename T> static constexpr inline T repeatBitPattern(
	T      pattern,
	size_t bitLength
) {
	T      output = pattern;
	size_t length = sizeof(T) * 8;

	for (; bitLength < length; bitLength *= 2)
		output |= output << bitLength;

	return output;
}

template<typename T> static constexpr inline T sequentialBitPattern(
	T      start,
	T      step,
	size_t bitLength
) {
	T      output = start;
	size_t length = sizeof(T) * 8;

	for (size_t i = bitLength; i < length; i += bitLength) {
		start  += step;
		output |= start << i;
	}

	return output;
}

template<typename T> static constexpr inline T repeatEachBit(
	T      value,
	size_t count
) {
	T output = 0;
	T mask   = (1 << count) - 1;

	for (; value; value >>= 1, mask <<= count) {
		if (value & 1)
			output |= mask;
	}

	return output;
}

template<
	typename    T0,
	typename    T1,
	typename    T,
	typename    V,
	typename... A
> static constexpr inline void accumulateFlags2(
	T    &output0,
	T    &output1,
	V    value,
	A... next
) {
	if constexpr (__is_same(V, T0))
		output0 |= T(value);
	else if constexpr (__is_same(V, T1))
		output1 |= T(value);
	else
		static_assert(false, "invalid type for flag");

	if constexpr (sizeof...(next) > 0)
		accumulateFlags2<T0, T1>(output0, output1, next...);
}

template<
	typename    T0,
	typename    T1,
	typename    T2,
	typename    T,
	typename    V,
	typename... A
> static constexpr inline void accumulateFlags3(
	T    &output0,
	T    &output1,
	T    &output2,
	V    value,
	A... next
) {
	if constexpr (__is_same(V, T0))
		output0 |= T(value);
	else if constexpr (__is_same(V, T1))
		output1 |= T(value);
	else if constexpr (__is_same(V, T2))
		output2 |= T(value);
	else
		static_assert(false, "invalid type for flag");

	if constexpr (sizeof...(next) > 0)
		accumulateFlags3<T0, T1, T2>(output0, output1, output2, next...);
}

}
