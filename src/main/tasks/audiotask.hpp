
#pragma once

#include <stddef.h>
#include <stdint.h>
#include "src/main/drivers/input.hpp"
#include "src/main/drivers/inputdefs.hpp"
#include "src/main/dsp/dsp.hpp"
#include "src/main/util/rtos.hpp"
#include "src/main/util/templates.hpp"
#include "src/main/sst.hpp"

namespace tasks {

static constexpr int    OUTPUT_SAMPLE_RATE = 44100;
static constexpr size_t AUDIO_BUFFER_SIZE  = 256;

/* Deck object */

enum DeckFlag : uint8_t {
	DECK_FLAG_PLAYING    = 1 << 0,
	DECK_FLAG_MONITORING = 1 << 1,
	DECK_FLAG_LOOPING    = 1 << 2,
	DECK_FLAG_REVERSE    = 1 << 3,
	DECK_FLAG_SHIFT_USED = 1 << 4
};

struct DeckState {
public:
	int playbackOffset, playbackStep;
	int cueOffset, loopStart, loopEnd;

	int     sampleRate;
	uint8_t flags;

	inline DeckState(void) {
		reset();
	}
	inline float getCurrentTime(void) const {
		if (!sampleRate)
			return 0.0f;

		return
			float(playbackOffset) / float(sampleRate * sst::SAMPLE_OFFSET_UNIT);
	}

	void reset(void);
};

struct SectorQueueEntry {
public:
	int            chunk;
	sst::SSTSector sector;
};

class AudioTaskDeck {
	friend class AudioTask;

private:
	sst::Sampler      sampler_;
	dsp::BiquadFilter filter_;
	dsp::Sample       audioBuffer_[AUDIO_BUFFER_SIZE][sst::NUM_CHANNELS];

	dsp::FloatBiquadFilter smoothingFilter_;

	DeckState                            state_;
	util::InPlaceQueue<SectorQueueEntry> sectorQueue_;

	void init_(void);
	void process_(void);
	void updateMeasuredSpeed_(int16_t value, float dt);
	void updateFilter_(uint8_t value);
};

/* Main audio processing task */

class AudioTask : public util::Task {
private:
	AudioTaskDeck   decks_[drivers::NUM_DECKS];
	dsp::Mixer      mainMixer_, monitorMixer_;
	dsp::Bitcrusher bitcrusher_;

	dsp::Sample mainBuffer_[AUDIO_BUFFER_SIZE][sst::NUM_CHANNELS];
	dsp::Sample monitorBuffer_[AUDIO_BUFFER_SIZE][sst::NUM_CHANNELS];

	util::Queue<drivers::InputState> inputQueue_;

	inline AudioTask(void) :
		Task("AudioTask", 0x1000)
	{}

	[[noreturn]] void taskMain_(void) override;
	void handleInputs_(const drivers::InputState &inputs);
	void handleDeckButtons_(
		int                 index,
		int16_t             selector,
		drivers::ButtonMask pressed,
		drivers::ButtonMask released,
		drivers::ButtonMask held
	);

public:
	inline void updateInputs(const drivers::InputState &inputs) {
		inputQueue_.push(inputs);
	}
	inline SectorQueueEntry *feedSector(int deck) {
		return decks_[deck].sectorQueue_.pushItem();
	}
	inline void finalizeFeed(int deck) {
		decks_[deck].sectorQueue_.finalizePush();
	}
	inline size_t getQueueLength(int deck) const {
		return decks_[deck].sectorQueue_.getLength();
	}
	inline void getDeckState(DeckState &output, int index) const {
		// The DeckState struct is not properly locked for concurrent access.
		// This may result in this method running while the struct is being
		// updated by the audio task and thus returning a partial update, but
		// that should not be an issue as all other tasks are merely displaying
		// the decks' current state.
		util::copy(output, decks_[index].state_);
	}

	static AudioTask &instance(void);
};

}
