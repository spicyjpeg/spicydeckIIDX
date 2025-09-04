
#include <stddef.h>
#include <stdint.h>
#include "src/main/dsp/adpcm.hpp"
#include "src/main/dsp/dsp.hpp"
#include "src/main/util/templates.hpp"
#include "src/main/defs.hpp"

namespace dsp {

/* Filter coefficient table */

DRAM_ATTR static const int16_t ADPCM_FILTER_COEFFS_[][2]{
	// Standard BRR ADPCM coefficients
	{   0 * 4,   0 * 4 },
	{  60 * 4,   0 * 4 },
	{ 115 * 4, -52 * 4 },
	{  98 * 4, -55 * 4 },
	{ 122 * 4, -60 * 4 },

	// Extended coefficients
	{ 120,    0 },
	{ 230, -104 },
	{ 196, -110 },
	{ 244, -120 },
	{  60,    0 },
	{ 115,  -52 },
	{  98,  -55 },
	{ 122,  -60 },
	{ 128, -240 },
	{  60, -240 },
	{  28, -240 }
};

static constexpr int ADPCM_FILTER_BITS_ = 8;
static constexpr int ADPCM_FILTER_UNIT_ = 1 << ADPCM_FILTER_BITS_;

[[gnu::always_inline]] static inline int clampSample_(int value) {
	return util::clamp(value, INT16_MIN, INT16_MAX);
}

/* 12-byte .sst ADPCM encoder */

/*
 * Loosely based on psxavenc's BRR ADPCM encoder implementation:
 * https://github.com/WonderfulToolchain/psxavenc/blob/main/libpsxav/adpcm.c
 */

IRAM_ATTR int SSTEncoder::estimateBlockGain_(
	const Sample *input,
	int          filterIndex,
	size_t       inputStride
) {
	// The optimal gain of each block is estimated to be
	// max(ceil(log2(|decodedSample| + 1)), ...) +/- 1. A crude (filter-only)
	// approximation of the encoder is used here.
	auto filter = ADPCM_FILTER_COEFFS_[filterIndex];

	const int a1 = filter[0], a2 = filter[1];
	int       s1 = s1_,       s2 = s2_;

	int posPeak = 0, negPeak = 0;

	for (int i = SST_SAMPLES_PER_BLOCK; i > 0; i--) {
		const int sample = *input;
		input           += inputStride;

		int encoded = sample * ADPCM_FILTER_UNIT_;
		encoded    -= a1 * s1;
		encoded    -= a2 * s2;
		encoded    -= ADPCM_FILTER_UNIT_ / 2;
		encoded    /= ADPCM_FILTER_UNIT_;

		if (encoded > posPeak)
			posPeak = encoded;
		if (encoded < negPeak)
			negPeak = encoded;

		s2 = s1;
		s1 = sample;
	}

	int shift = 0;

	while ((posPeak >> shift) > 7)
		shift++;
	while ((negPeak >> shift) < -8)
		shift++;

	return util::clamp(shift, 1, 11);
}

IRAM_ATTR uint64_t SSTEncoder::tryEncodeBlock_(
	SSTChunk<1>  &output,
	const Sample *input,
	int          gain,
	int          filterIndex,
	size_t       inputStride
) {
	output.blocks[0].header  = uint8_t(gain        & 15);
	output.blocks[0].header |= uint8_t(filterIndex & 15) << 4;

	auto ptr    = output.blocks[0].samples;
	auto filter = ADPCM_FILTER_COEFFS_[filterIndex];

	const int a1 = filter[0], a2 = filter[1];
	int       s1 = s1_,       s2 = s2_;

	const int actualGain = gain + ADPCM_FILTER_BITS_;
	uint64_t  totalError = 0;

	for (int i = SST_SAMPLES_PER_BLOCK; i > 0; i--) {
		const int sample = *input;
		input           += inputStride;

		// Encode the sample by performing the same steps as the decoder in
		// reverse.
		int residual = a1 * s1;
		residual    += a2 * s2;
		residual    += ADPCM_FILTER_UNIT_ / 2;

		int encoded = sample * ADPCM_FILTER_UNIT_;
		encoded    -= residual;
		encoded   >>= actualGain;
		encoded     = util::clamp(encoded, -8, 7);

		if (!(i % 2))
			*ptr      = uint8_t(encoded + 8);
		else
			*(ptr++) |= uint8_t(encoded + 8) << 4;

		// Simulate the sample being decoded back in order to measure the error.
		int decoded = encoded << actualGain;
		decoded    += residual;
		decoded    /= ADPCM_FILTER_UNIT_;
		decoded     = clampSample_(decoded);

		int error   = sample - decoded;
		totalError += error * error;

		s2 = s1;
		s1 = decoded;
	}

	output.s1 = int16_t(s1);
	output.s2 = int16_t(s2);
	return totalError;
}

IRAM_ATTR void SSTEncoder::encodeBlock_(
	SSTBlock     &output,
	const Sample *input,
	size_t       inputStride
) {
	// Bruteforce 48 possible combinations of filter index and gain in order to
	// find the one that produces the lowest noise floor.
	SSTChunk<1> encodes[util::countOf(ADPCM_FILTER_COEFFS_)][3];

	uint64_t bestError  = UINT64_MAX;
	auto     bestEncode = &encodes[0][0];

	for (size_t i = 0; i < util::countOf(ADPCM_FILTER_COEFFS_); i++) {
		const int gainOffset = estimateBlockGain_(input, i, inputStride);

		for (int j = 0; j < 2; j++) {
			auto error = tryEncodeBlock_(
				encodes[i][j],
				input,
				j - 1 + gainOffset,
				i,
				inputStride
			);

			if (error < bestError) {
				bestError  = error;
				bestEncode = &encodes[i][j];
			}
		}
	}

	util::copy(output, bestEncode->blocks[0]);

	s1_ = bestEncode->s1;
	s2_ = bestEncode->s2;
}

IRAM_ATTR void SSTEncoder::reset(void) {
	s1_ = 0;
	s2_ = 0;
}

IRAM_ATTR size_t SSTEncoder::encode(
	SSTChunkBase &output,
	const Sample *input,
	size_t       numSamples,
	size_t       inputStride
) {
	auto         block     = output.getBlocks();
	const size_t numBlocks =
		(numSamples + SST_SAMPLES_PER_BLOCK - 1) / SST_SAMPLES_PER_BLOCK;

	// Before doing any encoding, dump the filter's current state so that the
	// chunk can be decoded correctly. The decoder is stateless in order to
	// allow for out-of-order chunk decoding.
	output.s1 = s1_;
	output.s2 = s2_;

	while (numSamples > 0) {
		// Pad the last block to 28 samples.
		if (numSamples < SST_SAMPLES_PER_BLOCK) {
			Sample buffer[SST_SAMPLES_PER_BLOCK]{ 0 };
			auto   ptr = buffer;

			for (; numSamples > 0; numSamples--) {
				*(ptr++) = *input;
				input   += inputStride;
			}

			encodeBlock_(*(block++), buffer);
		} else {
			encodeBlock_(*(block++), input, inputStride);

			numSamples -= SST_SAMPLES_PER_BLOCK;
			input      += inputStride * SST_SAMPLES_PER_BLOCK;
		}
	}

	return numBlocks;
}

/* 12-byte .sst ADPCM decoder */

IRAM_ATTR size_t decodeSST(
	Sample             *output,
	const SSTChunkBase &input,
	size_t             numBlocks,
	size_t             outputStride
) {
	auto   block      = input.getBlocks();
	size_t numSamples = 0;

	int s1 = input.s1, s2 = input.s2;

	for (; (numBlocks > 0) && !block->isTerminator(); numBlocks--) {
		auto ptr    = block->samples;
		auto filter = ADPCM_FILTER_COEFFS_[block->getFilterIndex()];

		const int a1 = filter[0], a2 = filter[1];

		const int gain = block->getGain() + ADPCM_FILTER_BITS_;

		for (int i = SST_SAMPLES_PER_BLOCK; i > 0; i -= 2) {
			const int nibble1 = int(*ptr & 15) - 8;
			const int nibble2 = int(*ptr >> 4) - 8;
			ptr++;

			int sample1 = nibble1 << gain;
			sample1    += a1 * s1;
			sample1    += a2 * s2;
			sample1    += ADPCM_FILTER_UNIT_ / 2;
			sample1    /= ADPCM_FILTER_UNIT_;
			sample1     = clampSample_(sample1);

			*output = Sample(sample1);
			output += outputStride;

			s2 = s1;
			s1 = sample1;

			int sample2 = nibble2 << gain;
			sample2    += a1 * s1;
			sample2    += a2 * s2;
			sample2    += ADPCM_FILTER_UNIT_ / 2;
			sample2    /= ADPCM_FILTER_UNIT_;
			sample2     = clampSample_(sample2);

			*output = Sample(sample2);
			output += outputStride;

			s2 = s1;
			s1 = sample2;
		}

		block++;
		numSamples += SST_SAMPLES_PER_BLOCK;
	}

	return numSamples;
}

/* 16-byte BRR ADPCM decoder (unused) */

DRAM_ATTR static const int8_t SIGN_EXTENSION_LUT_[]{
	 0,  1,  2,  3,  4,  5,  6,  7,
	-8, -7, -6, -5, -4, -3, -2, -1
};

IRAM_ATTR void BRRDecoder::reset(void) {
	s1_ = 0;
	s2_ = 0;
}

IRAM_ATTR size_t BRRDecoder::decode(
	Sample         *output,
	const BRRBlock *input,
	size_t         numBlocks,
	size_t         outputStride,
	size_t         inputStride
) {
	const size_t numSamples = numBlocks * BRR_SAMPLES_PER_BLOCK;

	int s1 = s1_, s2 = s2_;

	for (; numBlocks > 0; numBlocks--) {
		auto ptr    = input->samples;
		auto filter = ADPCM_FILTER_COEFFS_[input->getFilterIndex()];

		const int a1 = filter[0], a2 = filter[1];

		const int gain = input->getGain() + ADPCM_FILTER_BITS_;

		for (int i = BRR_SAMPLES_PER_BLOCK; i > 0; i -= 2) {
			const int nibble1 = SIGN_EXTENSION_LUT_[*ptr & 15];
			const int nibble2 = SIGN_EXTENSION_LUT_[*ptr >> 4];
			ptr++;

			int sample1 = nibble1 << gain;
			sample1    += a1 * s1;
			sample1    += a2 * s2;
			sample1    += ADPCM_FILTER_UNIT_ / 2;
			sample1    /= ADPCM_FILTER_UNIT_;

			*output = Sample(clampSample_(sample1));
			output += outputStride;

			s2 = s1;
			s1 = sample1;

			int sample2 = nibble2 << gain;
			sample2    += a1 * s1;
			sample2    += a2 * s2;
			sample2    += ADPCM_FILTER_UNIT_ / 2;
			sample2    /= ADPCM_FILTER_UNIT_;

			*output = Sample(clampSample_(sample2));
			output += outputStride;

			s2 = s1;
			s1 = sample2;
		}

		input += inputStride;
	}

	s1_ = int32_t(s1);
	s2_ = int32_t(s2);
	return numSamples;
}

}
