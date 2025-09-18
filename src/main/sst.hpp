
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "src/main/dsp/adpcm.hpp"
#include "src/main/dsp/dsp.hpp"
#include "src/main/util/templates.hpp"

namespace sst {

static constexpr size_t NUM_CHANNELS       = 2;
static constexpr size_t BLOCKS_PER_SECTOR  = 85;
static constexpr size_t SAMPLES_PER_SECTOR =
	dsp::SST_SAMPLES_PER_BLOCK * BLOCKS_PER_SECTOR;

/* .sst file structures */

static constexpr size_t SST_MAX_VARIANTS      = 16;
static constexpr int    SST_PITCH_OFFSET_UNIT = 1 << 4;

enum SSTKeyScale : uint8_t {
	SCALE_UNKNOWN = 0,
	SCALE_MAJOR   = 1,
	SCALE_MINOR   = 2
};

struct [[gnu::packed]] SSTHeaderInfo {
public:
	uint32_t magic;
	uint32_t sampleRate, numChunks, waveformLength;
	uint8_t  numVariants, numChannels;

	uint8_t keyScale, keyNote;
	int16_t pitchOffsets[SST_MAX_VARIANTS];

	uint16_t titleOffset, artistOffset, albumOffset, genreOffset;
	uint8_t  trackNumber, trackCount, discNumber, discCount;
};

union [[gnu::packed]] SSTHeader {
public:
	SSTHeaderInfo info;
	char          strings[2048];

	inline bool validate(void) const {
		return true
			&& (info.magic       == "SST1"_c)
			&& (info.sampleRate  >= 8000)
			&& (info.sampleRate  <= 192000)
			&& (info.numVariants >= 1)
			&& (info.numVariants <= SST_MAX_VARIANTS)
			&& (info.numChannels == NUM_CHANNELS);
	}
	inline const char *getTitle(void) const {
		return &strings[info.titleOffset];
	}
	inline const char *getArtist(void) const {
		return &strings[info.artistOffset];
	}
	inline const char *getAlbum(void) const {
		return &strings[info.albumOffset];
	}
	inline const char *getGenre(void) const {
		return &strings[info.genreOffset];
	}
};

struct [[gnu::packed]] SSTSector {
public:
	dsp::SSTChunk<BLOCKS_PER_SECTOR> channels[NUM_CHANNELS];
};

/* .sst file reader */

class Reader {
private:
	FILE *file_;
	int  currentVariant_;

	SSTHeader  header_;
	util::Data waveform_;

public:
	inline Reader(void) :
		file_(nullptr),
		currentVariant_(0)
	{}
	inline ~Reader(void) {
		close();
	}
	inline const SSTHeader *getHeader(void) const {
		return file_ ? &header_ : nullptr;
	}
	inline const util::Data &getWaveform(void) const {
		return waveform_;
	}
	inline int getVariant(void) const {
		return currentVariant_;
	}
	inline void setVariant(int variant) {
		currentVariant_ = util::clamp(variant, 0, header_.info.numVariants - 1);
	}

	bool open(const char *path);
	void close(void);
	bool read(SSTSector &output, int chunk);

	void resetVariant(void);
	size_t getKeyName(char *output) const;
};

/* .sst sampler */

static constexpr int SAMPLE_OFFSET_BITS = 4;
static constexpr int SAMPLE_OFFSET_UNIT = 1 << SAMPLE_OFFSET_BITS;

using ReadCallback     = const SSTSector *(*)(int chunk, void *arg);
using ReadDoneCallback = void (*)(const SSTSector *sector, void *arg);

struct SamplerCacheEntry {
public:
	int         chunk;
	dsp::Sample samples[SAMPLES_PER_SECTOR][NUM_CHANNELS];
};

class Sampler {
private:
	SamplerCacheEntry cache_[2];
	int               currentCacheEntry_;

	ReadCallback     readCallback_;
	ReadDoneCallback readDoneCallback_;
	void             *arg_;

	const SamplerCacheEntry *loadChunk_(int chunk);

public:
	inline Sampler(void) :
		readCallback_(nullptr),
		readDoneCallback_(nullptr),
		arg_(nullptr)
	{
		flush();
	}
	inline void setCallbacks(
		ReadCallback     read,
		ReadDoneCallback readDone = nullptr,
		void             *arg     = nullptr
	) {
		readCallback_     = read;
		readDoneCallback_ = readDone;
		arg_              = arg;
	}

	void flush(void);
	void process(dsp::Sample *output, int offset, int step, size_t numSamples);
};

}
