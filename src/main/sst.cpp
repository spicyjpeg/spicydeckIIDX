
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "src/main/dsp/adpcm.hpp"
#include "src/main/dsp/dsp.hpp"
#include "src/main/defs.hpp"
#include "src/main/sst.hpp"

namespace sst {

static const char TAG_[]{ "sst" };

static constexpr uint32_t INVALID_CHUNK_ = UINT32_MAX;

/* .sst file reader and sector cache */

static const char *const KEY_NAMES_[]{
	"C", "C#/Db",
	"D", "D#/Eb",
	"E",
	"F", "F#/Gb",
	"G", "G#/Ab",
	"A", "A#/Bb",
	"B"
};

SSTSector *Reader::cacheSector_(uint32_t chunk) {
	// Determine which cache entry to overwrite with the new sector.
	auto         &set = cache_.as<SectorCache>()->sets[chunk % NUM_CACHE_SETS];
	CachedSector *bestEntry;

	if (evictionMode_ == EVICT_RANDOM) {
		bestEntry = &set[rand() % NUM_CACHE_ENTRIES];
	} else {
		uint32_t bestChunk = UINT32_MAX;
		bestEntry          = &set[0];

		for (auto &entry : set) {
			const bool isBest = (evictionMode_ == EVICT_LOWEST)
				? (entry.chunk < bestChunk)
				: (entry.chunk > bestChunk);

			if (isBest) {
				bestChunk = entry.chunk;
				bestEntry = &entry;
			}
		}
	}

	// Read the sector into the chosen entry.
	uint32_t offset = (chunk * header_.numVariants) + variant_;
	offset         *= sizeof(SSTSector);

	if (fseek(file_, sizeof(SSTHeader) + offset, SEEK_SET)) {
		ESP_LOGE(TAG_, ".sst seek failed");
		return nullptr;
	}
	if (!fread(&bestEntry->sector, sizeof(SSTSector), 1, file_)) {
		ESP_LOGE(TAG_, ".sst read failed");
		return nullptr;
	}

	return &bestEntry->sector;
}

void Reader::flushCache_(void) {
	if (!file_)
		return;

	for (auto &set : cache_.as<SectorCache>()->sets) {
		for (auto &entry : set)
			entry.chunk = INVALID_CHUNK_;
	}
}

bool Reader::open(const char *path) {
	if (file_)
		close();

	file_ = fopen(path, "rb");

	if (!file_) {
		ESP_LOGE(TAG_, "could not open .sst file: %s", path);
		return false;
	}
	if (!fread(&header_, sizeof(SSTHeader), 1, file_)) {
		ESP_LOGE(TAG_, "could not read .sst header: %s", path);
		goto cleanup;
	}
	if (!header_.validate()) {
		ESP_LOGE(TAG_, "not a valid .sst file: %s", path);
		goto cleanup;
	}

	// By default, use the first variant whose pitch offset is zero. This also
	// flushes the sector cache.
	for (int i = 0; i < header_.numVariants; i++) {
		if (!header_.pitchOffsets[i]) {
			variant_ = i;
			break;
		}
	}

	// Fill up the cache with the first few sectors of the file.
	cache_.allocate<SectorCache>();
	flushCache_();

	for (int i = 0; i < NUM_CACHE_ENTRIES; i++)
		cacheSector_(i);

	return true;

cleanup:
	close();
	return false;
}

void Reader::close(void) {
	if (!file_)
		return;

	fclose(file_);
	cache_.destroy();
	file_ = nullptr;
}

bool Reader::read(dsp::Sample *output, uint32_t chunk) {
	assert(file_ && (chunk < header_.numChunks));

	// Check if the sector is cached.
	auto      &set    = cache_.as<SectorCache>()->sets[chunk % NUM_CACHE_SETS];
	SSTSector *sector = nullptr;

	for (auto &entry : set) {
		if (entry.chunk == chunk) {
			sector = &entry.sector;
			break;
		}
	}

	if (!sector)
		sector = cacheSector_(chunk);

	for (int i = 0; i < NUM_CHANNELS; i++)
		dsp::decodeSST(
			&output[i],
			sector->channels[i],
			BLOCKS_PER_SECTOR,
			NUM_CHANNELS
		);

	return true;
}

size_t Reader::getKeyName(char *output) const {
	assert(file_);

	if (!header_.keyScale) {
		output[0] = '-';
		output[1] = 0;
		return 1;
	}

	int key = header_.keyNote * SST_PITCH_OFFSET_UNIT;
	key    += header_.pitchOffsets[variant_];
	key    += SST_PITCH_OFFSET_UNIT	* 12; // Workaround for % sign behavior
	key    += SST_PITCH_OFFSET_UNIT / 2;
	key    /= SST_PITCH_OFFSET_UNIT;

	auto source = KEY_NAMES_[key % 12];
	auto dest   = output;

	while (*source)
		*(dest++) = *(source++);
	if (header_.keyScale == SCALE_MINOR)
		*(dest++) = 'm';

	*dest = 0;
	return dest - output;
}

/* .sst sampler */

static constexpr int STEP_THRESHOLD_ = SAMPLE_OFFSET_UNIT * 100;

IRAM_ATTR static int interpolate_(int sample1, int sample2, int alpha) {
	int diff = (sample2 - sample1) * alpha;
	diff    /= SAMPLE_OFFSET_UNIT;

	return sample1 + diff;
}

IRAM_ATTR void Sampler::flush(void) {
	cache_[0].chunk = INVALID_CHUNK_;
	cache_[1].chunk = INVALID_CHUNK_;
}

IRAM_ATTR void Sampler::process(
	dsp::Sample *output,
	Reader      &reader,
	uint32_t    offset,
	int         step,
	size_t      numSamples
) {
	// Output silence if the playback rate is too slow.
	if ((step > -STEP_THRESHOLD_) && (step < STEP_THRESHOLD_)) {
		memset(output, 0, numSamples * sizeof(dsp::Sample) * NUM_CHANNELS);
		return;
	}

	const int numChunks = reader.getHeader()->numChunks;

	int currentChunk = offset / (SAMPLE_OFFSET_UNIT * SAMPLES_PER_SECTOR);
	int currentSector;

	// Ensure a decoded copy of the first sector going to be sampled is in the
	// cache.
	if (currentChunk == cache_[0].chunk) {
		currentSector = 0;
	} else if (currentChunk == cache_[1].chunk) {
		currentSector = 1;
	} else if (currentChunk < numChunks) {
		reader.read(cache_[0].samples[0], currentChunk);
		currentSector = 0;
	}

	for (; numSamples > 0; numSamples--) {
		const int sampleOffset = offset / SAMPLE_OFFSET_UNIT;
		const int alpha        = offset % SAMPLE_OFFSET_UNIT;

		// In order to perform linear interpolation, both the sample preceding
		// the current offset and the one after it are required. This two-sample
		// window may span two sectors, so a few different cases must be handled
		// here:
		// - both samples are in the current sector
		//   -> sample the current sector only;
		// - both samples are in a new sector
		//   -> move onto the next sector and sample it;
		// - the second sample is in a new sector
		//   -> load the next sector, sample both sectors then move on;
		// - either sample is out of bounds
		//   -> output silence.
		const int chunk1  = sampleOffset       / SAMPLES_PER_SECTOR;
		const int offset1 = sampleOffset       % SAMPLES_PER_SECTOR;
		const int chunk2  = (sampleOffset + 1) / SAMPLES_PER_SECTOR;
		const int offset2 = (sampleOffset + 1) % SAMPLES_PER_SECTOR;

		if ((chunk1 >= numChunks) || (chunk2 >= numChunks)) {
			for (int i = 0; i < NUM_CHANNELS; i++)
				output[i] = 0;
		} else {
			auto &sector     = cache_[currentSector];
			auto &nextSector = cache_[currentSector ^ 1];

			DecodedSector *sector1, *sector2;

			if (chunk2 == currentChunk) {
				sector1 = &sector;
				sector2 = &sector;
			} else {
				sector1 = (chunk1 == currentChunk) ? &sector : &nextSector;
				sector2 = &nextSector;

				if (nextSector.chunk != chunk2)
					reader.read(nextSector.samples[0], chunk2);

				currentChunk   = chunk2;
				currentSector ^= 1;
			}

			for (int i = 0; i < NUM_CHANNELS; i++)
				output[i] = interpolate_(
					sector1->samples[offset1][i],
					sector2->samples[offset2][i],
					alpha
				);
		}

		output += NUM_CHANNELS;
		offset += step;
	}
}

}
