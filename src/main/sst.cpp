
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "src/main/dsp/adpcm.hpp"
#include "src/main/dsp/dsp.hpp"
#include "src/main/util/templates.hpp"
#include "src/main/defs.hpp"
#include "src/main/sst.hpp"

namespace sst {

static const char TAG_[]{ "sst" };

/* .sst file reader */

static const char *const KEY_NAMES_[]{
	"C", "C#/Db",
	"D", "D#/Eb",
	"E",
	"F", "F#/Gb",
	"G", "G#/Ab",
	"A", "A#/Bb",
	"B"
};

bool Reader::open(const char *path) {
	if (file_)
		close();

	size_t  waveformOffset;
	int16_t bestPitch = INT16_MAX;

	file_ = fopen(path, "rb");

	if (!file_) {
		ESP_LOGE(TAG_, "could not open .sst file: %s", path);
		return false;
	}
	if (
		!fread(&header_, sizeof(SSTHeader), 1, file_) ||
		!header_.validate()
	) {
		ESP_LOGE(TAG_, "not a valid .sst file: %s", path);
		goto cleanup;
	}

	// Preload the entire waveform (which is typically just a few kilobytes).
	waveform_.allocate((header_.info.waveformLength + 1) / 2);
	assert(waveform_.ptr);

	waveformOffset  = header_.info.numChunks * header_.info.numVariants;
	waveformOffset *= sizeof(SSTSector);
	waveformOffset += sizeof(SSTHeader);

	if (
		fseek(file_, waveformOffset, SEEK_SET) ||
		!fread(waveform_.ptr, waveform_.length, 1, file_)
	) {
		ESP_LOGE(TAG_, "could not load .sst waveform: %s", path);
		goto cleanup;
	}

	// By default, use the variant whose pitch offset is closest to zero.
	for (int i = 0; i < header_.info.numVariants; i++) {
		auto pitch = header_.info.pitchOffsets[i];

		if (pitch < 0)
			pitch = -pitch;

		if (pitch < bestPitch) {
			bestPitch       = pitch;
			currentVariant_ = i;
		}
	}

	ESP_LOGI(TAG_, "loaded .sst: %s (variant %d)", path, currentVariant_);
	return true;

cleanup:
	close();
	return false;
}

void Reader::close(void) {
	if (!file_)
		return;

	fclose(file_);
	waveform_.destroy();
	file_ = nullptr;
}

bool Reader::read(SSTSector &output, int chunk) {
	if (!file_)
		return false;
	if ((chunk < 0) || (chunk >= header_.info.numChunks))
		return false;

	size_t chunkOffset = chunk * header_.info.numVariants;
	chunkOffset       += currentVariant_;
	chunkOffset       *= sizeof(SSTSector);
	chunkOffset       += sizeof(SSTHeader);

	if (
		fseek(file_, chunkOffset, SEEK_SET) ||
		!fread(&output, sizeof(SSTSector), 1, file_)
	) {
		ESP_LOGE(TAG_, ".sst read failed, c=%d, v=%d", chunk, currentVariant_);
		return false;
	}

	return true;
}

size_t Reader::getKeyName(char *output) const {
	if (!file_)
		return 0;

	if (!header_.info.keyScale) {
		output[0] = '-';
		output[1] = 0;
		return 1;
	}

	int key = header_.info.keyNote * SST_PITCH_OFFSET_UNIT;
	key    += header_.info.pitchOffsets[currentVariant_];
	key    += SST_PITCH_OFFSET_UNIT	* 12; // Workaround for % sign behavior
	key    += SST_PITCH_OFFSET_UNIT / 2;
	key    /= SST_PITCH_OFFSET_UNIT;

	auto source = KEY_NAMES_[key % 12];
	auto dest   = output;

	while (*source)
		*(dest++) = *(source++);
	if (header_.info.keyScale == SCALE_MINOR)
		*(dest++) = 'm';

	*dest = 0;
	return dest - output;
}

/* .sst sampler */

static constexpr int CHUNK_INDEX_UNIT_ = SAMPLE_OFFSET_UNIT * SAMPLES_PER_SECTOR;
static constexpr int STEP_THRESHOLD_   = SAMPLE_OFFSET_UNIT * 100;

IRAM_ATTR static inline int interpolate_(int sample1, int sample2, int alpha) {
	int diff = (sample2 - sample1) * alpha;
	diff    /= SAMPLE_OFFSET_UNIT;

	return sample1 + diff;
}

IRAM_ATTR const SamplerCacheEntry *Sampler::loadChunk_(int chunk) {
	auto &oldEntry = cache_[currentCacheEntry_];

	if (oldEntry.chunk == chunk)
		return &oldEntry;

	currentCacheEntry_ ^= 1;
	auto &newEntry      = cache_[currentCacheEntry_];

	if (newEntry.chunk == chunk)
		return &newEntry;

	// Decode the sector returned by the callback, falling back to generating
	// silence if none was returned.
	if (readCallback_) {
		auto sector = readCallback_(chunk, arg_);

		if (sector) {
			for (int i = 0; i < NUM_CHANNELS; i++)
				dsp::decodeSST(
					&newEntry.samples[0][i],
					sector->channels[i],
					NUM_CHANNELS
				);

			if (readDoneCallback_)
				readDoneCallback_(sector, arg_);

			newEntry.chunk = chunk;
			return &newEntry;
		}
	}

	util::clear(newEntry.samples);
	return &newEntry;
}

IRAM_ATTR void Sampler::flush(void) {
	cache_[0].chunk = -1;
	cache_[1].chunk = -1;
}

IRAM_ATTR void Sampler::process(
	dsp::Sample *output,
	int         offset,
	int         step,
	size_t      numSamples
) {
	// Output silence if the playback rate is too slow.
	if ((step > -STEP_THRESHOLD_) && (step < STEP_THRESHOLD_)) {
		memset(output, 0, numSamples * sizeof(dsp::Sample) * NUM_CHANNELS);
		return;
	}

	int chunk = offset / CHUNK_INDEX_UNIT_;
	offset   %= CHUNK_INDEX_UNIT_;

	auto cacheEntry = loadChunk_(chunk);

	for (; numSamples > 0; numSamples--) {
		const int sample = offset >> SAMPLE_OFFSET_BITS;
		const int alpha  = offset & (SAMPLE_OFFSET_UNIT - 1);

		const dsp::Sample *sample1 = cacheEntry->samples[sample];
		const dsp::Sample *sample2;

		// In order to perform linear interpolation, both the sample preceding
		// the current offset and the one after it are required. This window may
		// span two sectors, so two different cases must be handled here:
		// - both samples are in the current sector
		//   -> sample the current sector only;
		// - the second sample is in the next sector
		//   -> load the next sector and sample both.
		if (sample != (SAMPLES_PER_SECTOR - 1))
			sample2 = cacheEntry->samples[sample + 1];
		else
			sample2 = loadChunk_(chunk + 1)->samples[0];

		for (int i = 0; i < NUM_CHANNELS; i++)
			*(output++) = interpolate_(sample1[i], sample2[i], alpha);

		// Use a DDA-like algorithm to update the chunk index incrementally
		// without performing division. As the step can be negative, overflows
		// in either direction must be taken into account.
		offset += step;

		if (offset >= CHUNK_INDEX_UNIT_) {
			cacheEntry = loadChunk_(--chunk);
			offset    -= CHUNK_INDEX_UNIT_;
		} else if (offset < 0) {
			cacheEntry = loadChunk_(++chunk);
			offset    += CHUNK_INDEX_UNIT_;
		}
	}
}

}
