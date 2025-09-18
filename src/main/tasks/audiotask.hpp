
#pragma once

#include <stdint.h>
#include "src/main/drivers/input.hpp"
#include "src/main/drivers/inputdefs.hpp"
#include "src/main/dsp/dsp.hpp"
#include "src/main/util/rtos.hpp"
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

	util::InPlaceQueue<SectorQueueEntry> sectorQueue_;

	int     sampleRate_;
	uint8_t flags_;

	int offset_, cueOffset_, loopStart_, loopEnd_;
	int step_;

	inline AudioTaskDeck(void) :
		sampleRate_(0),
		flags_(0)
	{}

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

	static AudioTask &instance(void);
};

}
