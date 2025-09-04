
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "src/main/dsp/adpcm.hpp"
#include "src/main/dsp/dsp.hpp"
#include "src/main/util/templates.hpp"

namespace sst {

static constexpr size_t NUM_CHANNELS       = 2;
static constexpr size_t BLOCKS_PER_SECTOR  = 21;
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

struct [[gnu::packed]] SSTHeader {
public:
	uint32_t magic;
	uint32_t sampleRate, numChunks;
	uint8_t  numVariants, numChannels;

	uint8_t titleOffset, artistOffset, albumOffset, genreOffset;
	uint8_t trackNumber, trackCount, discNumber, discCount;

	SSTKeyScale keyScale;
	uint8_t     keyNote;
	int16_t     pitchOffsets[SST_MAX_VARIANTS];

	char strings[456];

	inline bool validate(void) const {
		return true
			&& (magic       == "SST1"_c)
			&& (sampleRate  >= 8000)
			&& (sampleRate  <= 192000)
			&& (numVariants >= 1)
			&& (numVariants <= SST_MAX_VARIANTS)
			&& (numChannels == NUM_CHANNELS);
	}
	inline const char *getTitle(void) const {
		return &strings[titleOffset * 2];
	}
	inline const char *getArtist(void) const {
		return &strings[artistOffset * 2];
	}
	inline const char *getAlbum(void) const {
		return &strings[albumOffset * 2];
	}
	inline const char *getGenre(void) const {
		return &strings[genreOffset * 2];
	}
};

struct [[gnu::packed]] SSTSector {
public:
	dsp::SSTChunk<BLOCKS_PER_SECTOR> channels[NUM_CHANNELS];
};

/* .sst file reader and sector cache */

// The sector cache emulates an 8-way set-associative cache, holding up to 128
// sectors. The caches for both decks take up around 128 KB of RAM.
static constexpr size_t NUM_CACHE_SETS    = 16;
static constexpr size_t CACHE_SET_SIZE    = 8;
static constexpr size_t NUM_CACHE_ENTRIES = NUM_CACHE_SETS * CACHE_SET_SIZE;

enum EvictionMode {
	// Evict sectors that are closest to the beginning of the file (i.e. the
	// least recently played sectors during normal playback)
	EVICT_LOWEST  = 0,
	// Evict sectors that are closest to the end of the file (i.e. the least
	// recently played sectors during reverse playback)
	EVICT_HIGHEST = 1,
	// Evict a randomly chosen sector
	EVICT_RANDOM  = 2
};

struct CachedSector {
public:
	uint32_t  chunk;
	SSTSector sector;
};

struct SectorCache {
public:
	CachedSector sets[NUM_CACHE_SETS][CACHE_SET_SIZE];
};

class Reader {
private:
	FILE    *file_;
	uint8_t variant_, evictionMode_;

	util::Data cache_;
	SSTHeader  header_;

	SSTSector *cacheSector_(uint32_t chunk);
	void flushCache_(void);

public:
	inline Reader(void) :
		file_(nullptr),
		variant_(0),
		evictionMode_(EVICT_LOWEST)
	{}
	inline ~Reader(void) {
		close();
	}
	inline const SSTHeader *getHeader(void) const {
		return file_ ? &header_ : nullptr;
	}
	inline int getVariant(void) const {
		return variant_;
	}
	inline void setVariant(int variant) {
		flushCache_();
		variant_ = variant;
	}
	inline void setEvictionMode(EvictionMode mode) {
		evictionMode_ = mode;
	}

	bool open(const char *path);
	void close(void);
	bool read(dsp::Sample *output, uint32_t chunk);

	size_t getKeyName(char *output) const;
};

/* .sst sampler */

static constexpr int SAMPLE_OFFSET_UNIT = 1 << 4;

struct DecodedSector {
public:
	uint32_t    chunk;
	dsp::Sample samples[SAMPLES_PER_SECTOR][NUM_CHANNELS];
};

class Sampler {
private:
	DecodedSector cache_[2];

public:
	inline Sampler(void) {
		flush();
	}

	void flush(void);
	void process(
		dsp::Sample *output,
		Reader      &reader,
		uint32_t    offset,
		int         step,
		size_t      numSamples
	);
};

}
