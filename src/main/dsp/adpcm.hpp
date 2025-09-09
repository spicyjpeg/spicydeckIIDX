
#pragma once

#include <stddef.h>
#include <stdint.h>
#include "src/main/dsp/dsp.hpp"

namespace dsp {

/* 12-byte .sst ADPCM encoder and decoder */

class [[gnu::packed]] SSTBlock {
public:
	uint8_t header;
	uint8_t samples[11];

	inline int getGain(void) const {
		return header & 15;
	}
	inline int getFilterIndex(void) const {
		return header >> 4;
	}
};

class [[gnu::packed]] SSTChunkBase {
public:
	int16_t s1, s2;

	inline const SSTBlock *getBlocks(void) const {
		return reinterpret_cast<const SSTBlock *>(&this[1]);
	}
	inline SSTBlock *getBlocks(void) {
		return reinterpret_cast<SSTBlock *>(&this[1]);
	}
};

template<size_t N> class [[gnu::packed]] SSTChunk : public SSTChunkBase {
public:
	SSTBlock blocks[N];
};

static constexpr size_t SST_SAMPLES_PER_BLOCK = sizeof(SSTBlock::samples) * 2;

class SSTEncoder {
private:
	Sample s1_, s2_;

	int estimateBlockGain_(
		const Sample *input,
		int          filterIndex,
		size_t       inputStride = 1
	);
	int64_t tryEncodeBlock_(
		SSTChunk<1>  &output,
		const Sample *input,
		int          gain,
		int          filterIndex,
		size_t       inputStride = 1
	);
	void encodeBlock_(
		SSTBlock     &output,
		const Sample *input,
		size_t       inputStride = 1
	);

public:
	inline SSTEncoder(void) {
		reset();
	}

	void reset(void);
	size_t encode(
		SSTChunkBase &output,
		const Sample *input,
		size_t       numSamples,
		size_t       inputStride = 1
	);
};

size_t decodeSST(
	Sample             *output,
	const SSTChunkBase &input,
	size_t             numBlocks,
	size_t             outputStride = 1
);

template<size_t N> static inline size_t decodeSST(
	Sample            *output,
	const SSTChunk<N> &input,
	size_t            outputStride = 1
) {
	return decodeSST(output, input, N, outputStride);
}

/* 16-byte BRR ADPCM decoder (unused) */

class [[gnu::packed]] BRRBlock {
public:
	uint8_t header, loopFlags;
	uint8_t samples[14];

	inline int getGain(void) const {
		return 12 - (header & 15);
	}
	inline int getFilterIndex(void) const {
		return (header >> 4) & 7;
	}
};

static constexpr size_t BRR_SAMPLES_PER_BLOCK = sizeof(BRRBlock::samples) * 2;

class BRRDecoder {
private:
	int32_t s1_, s2_;

public:
	inline BRRDecoder(void) {
		reset();
	}

	void reset(void);
	size_t decode(
		Sample         *output,
		const BRRBlock *input,
		size_t         numBlocks,
		size_t         outputStride = 1,
		size_t         inputStride  = 1
	);
};

}
