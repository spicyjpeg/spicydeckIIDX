
#pragma once

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "src/main/drivers/input.hpp"
#include "src/main/drivers/inputdefs.hpp"
#include "src/main/dsp/dsp.hpp"
#include "src/main/sst.hpp"
#include "src/main/taskbase.hpp"

static constexpr int    OUTPUT_SAMPLE_RATE = 44100;
static constexpr size_t AUDIO_BUFFER_SIZE  = 256;

/* Deck object */

enum DeckFlags : uint8_t {
	DECK_FLAG_READY      = 1 << 0,
	DECK_FLAG_PLAYING    = 1 << 1,
	DECK_FLAG_MONITORING = 1 << 2,
	DECK_FLAG_LOOPING    = 1 << 3,
	DECK_FLAG_REVERSE    = 1 << 4,
	DECK_FLAG_SHIFT_USED = 1 << 5
};

class DeckAudio {
	friend class AudioTask;

private:
	sst::Reader       reader_;
	sst::Sampler      sampler_;
	dsp::BiquadFilter filter_;

	dsp::Sample buffer_[AUDIO_BUFFER_SIZE][sst::NUM_CHANNELS];
	uint32_t    offset_, cueOffset_, loopStart_, loopEnd_;
	int         step_;

	uint8_t flags_;

	inline DeckAudio(void) {}

	void init_(void);
	void process_(void);
	void updateMeasuredSpeed_(int16_t value, float dt);
	void updateFilter_(uint8_t value);
};

/* Main audio processing task */

class AudioTask : public Task {
private:
	DeckAudio       decks_[drivers::NUM_DECKS];
	dsp::Mixer      mainMixer_, monitorMixer_;
	dsp::Bitcrusher bitcrusher_;

	dsp::Sample mainBuffer_[AUDIO_BUFFER_SIZE][sst::NUM_CHANNELS];
	dsp::Sample monitorBuffer_[AUDIO_BUFFER_SIZE][sst::NUM_CHANNELS];

	inline AudioTask(void) :
		Task("AudioTask", 1, configMAX_PRIORITIES - 1)
	{}

	void mainInit_(void);
	void mainLoop_(void);
	void handleMessage_(const TaskMessage &message);
	void handleInputs_(const drivers::InputState &inputs);
	void handleDeckButtons_(
		int                 index,
		int16_t             selector,
		drivers::ButtonMask pressed,
		drivers::ButtonMask released,
		drivers::ButtonMask held
	);

public:
	static AudioTask &instance(void);
};
