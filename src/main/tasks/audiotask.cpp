
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include "src/main/drivers/audio.hpp"
#include "src/main/drivers/input.hpp"
#include "src/main/drivers/inputdefs.hpp"
#include "src/main/dsp/dsp.hpp"
#include "src/main/tasks/audiotask.hpp"
#include "src/main/tasks/iotask.hpp"
#include "src/main/tasks/streamtask.hpp"
#include "src/main/util/templates.hpp"
#include "src/main/sst.hpp"

namespace tasks {

/* Deck object */

// Allocate ~96 KB per deck for the sector streaming FIFOs.
static constexpr size_t NUM_QUEUED_SECTORS_ = 48;

void AudioTaskDeck::init_(void) {
	sampler_.setCallbacks(
		[](int chunk, void *arg) -> const sst::SSTSector * {
			auto deck = reinterpret_cast<AudioTaskDeck *>(arg);

			// Consume all sectors in the queue prior to the requested one.
			for (;;) {
				auto entry = deck->sectorQueue_.popItem();

				if (!entry) // Underrun
					return nullptr;

				if (entry->chunk == chunk)
					return &(entry->sector);
				else
					deck->sectorQueue_.finalizePop();
			}
		},
		[](const sst::SSTSector *sector, void *arg) {
			auto deck = reinterpret_cast<AudioTaskDeck *>(arg);

			deck->sectorQueue_.finalizePop();
		},
		this
	);
	filter_.reset();

	bool ok = sectorQueue_.allocate(NUM_QUEUED_SECTORS_);
	assert(ok);

	sampleRate_ = 0;
	flags_      = 0;

	offset_    = 0;
	cueOffset_ = 0;
	loopStart_ = INT_MIN;
	loopEnd_   = INT_MIN;
	step_      = 0;
}

void AudioTaskDeck::process_(void) {
	sampler_.process(audioBuffer_[0], offset_, step_, AUDIO_BUFFER_SIZE);

	for (int i = 0; i < sst::NUM_CHANNELS; i++) {
		filter_.process(
			&audioBuffer_[0][i],
			&audioBuffer_[0][i],
			AUDIO_BUFFER_SIZE,
			sst::NUM_CHANNELS,
			sst::NUM_CHANNELS
		);
	}

	// Update the current playback position.
	const int delta = step_ * AUDIO_BUFFER_SIZE;

	if ((-delta) > offset_)
		offset_  = 0;
	else
		offset_ += delta;

	if (flags_ & DECK_FLAG_LOOPING) {
		while (offset_ >= loopEnd_)
			offset_ -= loopEnd_ - loopStart_;
	}
}

void AudioTaskDeck::updateMeasuredSpeed_(int16_t value, float dt) {
	float speed = float(value) / dt;
	speed      /= float(drivers::DECK_STEPS_PER_REV);
	speed      /= DECK_TARGET_RPM / 60.0f;

	// TODO: apply a filter here to stabilize playback speed
	speed *= float(sampleRate_);
	speed *= float(sst::SAMPLE_OFFSET_UNIT);
	step_  = int(speed);
}

void AudioTaskDeck::updateFilter_(uint8_t value) {
	float                 cutoff = float(value) / 127.5f;
	dsp::BiquadFilterType type;

	if (cutoff < 1.0f) {
		type    = dsp::FILTER_LOWPASS;
	} else {
		cutoff -= 1.0f;
		type    = dsp::FILTER_HIGHPASS;
	}

	filter_.configure(type, cutoff * cutoff);
}

/* Main audio processing task */

[[noreturn]] void AudioTask::taskMain_(void) {
	auto &audioDriver = drivers::AudioDriver::instance();

	for (auto &deck : decks_)
		deck.init_();

	for (;;) {
		drivers::InputState inputs;

		while (inputQueue_.pop(inputs))
			handleInputs_(inputs);

		for (auto &deck : decks_)
			deck.process_();

		for (int i = 0; i < sst::NUM_CHANNELS; i++) {
			mainMixer_.process(
				&mainBuffer_[0][i],
				&decks_[0].audioBuffer_[0][i],
				&decks_[1].audioBuffer_[0][i],
				AUDIO_BUFFER_SIZE,
				sst::NUM_CHANNELS,
				sst::NUM_CHANNELS
			);
			monitorMixer_.process(
				&monitorBuffer_[0][i],
				&decks_[0].audioBuffer_[0][i],
				&decks_[1].audioBuffer_[0][i],
				AUDIO_BUFFER_SIZE,
				sst::NUM_CHANNELS,
				sst::NUM_CHANNELS
			);
			bitcrusher_.process(
				&mainBuffer_[0][i],
				&mainBuffer_[0][i],
				AUDIO_BUFFER_SIZE,
				sst::NUM_CHANNELS,
				sst::NUM_CHANNELS
			);
		}

		audioDriver.feed(mainBuffer_[0], monitorBuffer_[0], AUDIO_BUFFER_SIZE);
	}
}

void AudioTask::handleInputs_(const drivers::InputState &inputs) {
	decks_[0].updateMeasuredSpeed_(inputs.decks[0], inputs.dt);
	decks_[1].updateMeasuredSpeed_(inputs.decks[1], inputs.dt);
	decks_[0].updateFilter_(inputs.analog[drivers::ANALOG_LEFT_FILTER]);
	decks_[1].updateFilter_(inputs.analog[drivers::ANALOG_RIGHT_FILTER]);

	const float mainVolume    =
		float(inputs.analog[drivers::ANALOG_MAIN_VOLUME])    / 255.0f;
	const float monitorVolume =
		float(inputs.analog[drivers::ANALOG_MONITOR_VOLUME]) / 255.0f;
	const float crossfade     =
		float(inputs.analog[drivers::ANALOG_CROSSFADE])      / 255.0f;
	const float effectDepth   =
		float(inputs.analog[drivers::ANALOG_EFFECT_DEPTH])   / 255.0f;

	mainMixer_.configure(
		(1.0f - crossfade) * mainVolume,
		crossfade          * mainVolume
	);
	monitorMixer_.configure(
		(decks_[0].flags_ & DECK_FLAG_MONITORING) ? monitorVolume : 0.0f,
		(decks_[1].flags_ & DECK_FLAG_MONITORING) ? monitorVolume : 0.0f
	);
	bitcrusher_.configure(effectDepth);

	handleDeckButtons_(
		0,
		inputs.selector,
		(inputs.buttonsPressed  >> 0) & drivers::DECK_BTN_BITMASK,
		(inputs.buttonsReleased >> 0) & drivers::DECK_BTN_BITMASK,
		(inputs.buttonsHeld     >> 0) & drivers::DECK_BTN_BITMASK
	);
	handleDeckButtons_(
		1,
		inputs.selector,
		(inputs.buttonsPressed  >> 5) & drivers::DECK_BTN_BITMASK,
		(inputs.buttonsReleased >> 5) & drivers::DECK_BTN_BITMASK,
		(inputs.buttonsHeld     >> 5) & drivers::DECK_BTN_BITMASK
	);
}

void AudioTask::handleDeckButtons_(
	int                 index,
	int16_t             selector,
	drivers::ButtonMask pressed,
	drivers::ButtonMask released,
	drivers::ButtonMask held
) {
	auto &deck = decks_[index];

	if (held & drivers::DECK_BTN_SHIFT) {
		auto &streamTask = StreamTask::instance();

		if (selector < 0)
			streamTask.issueCommand(index, STREAM_CMD_PREV_VARIANT);
		else if (selector > 0)
			streamTask.issueCommand(index, STREAM_CMD_NEXT_VARIANT);

		if (pressed & drivers::DECK_BTN_RESTART)
			deck.offset_ = 0;

		if (pressed & drivers::DECK_BTN_CUE_JUMP)
			deck.offset_ = deck.cueOffset_;

		if (pressed & drivers::DECK_BTN_CUE_SET)
			deck.cueOffset_ = deck.offset_;

		if (pressed & drivers::DECK_BTN_REVERSE)
			deck.flags_ ^= DECK_FLAG_REVERSE;

		if (pressed & ~drivers::DECK_BTN_SHIFT)
			deck.flags_ |= DECK_FLAG_SHIFT_USED;
	} else {
		if (pressed & drivers::DECK_BTN_LOOP_IN) {
			const int length = deck.loopEnd_ - deck.loopStart_;
			deck.loopStart_  = deck.offset_;

			// Move the entire loop when attempting to move the start point past
			// the end.
			if ((deck.loopEnd_ >= 0) && (deck.loopEnd_ < deck.offset_))
				deck.loopEnd_ = deck.offset_ + length;
		}

		if (pressed & drivers::DECK_BTN_LOOP_OUT) {
			if ((deck.loopStart_ >= 0) && (deck.offset_ > deck.loopStart_)) {
				deck.loopEnd_ = deck.offset_;
				deck.flags_  |= DECK_FLAG_LOOPING;
			}
		}

		if (pressed & drivers::DECK_BTN_RELOOP) {
			if (
				(deck.loopStart_ >= 0) &&
				(deck.loopEnd_   >= 0) &&
				(deck.loopEnd_ > deck.loopStart_)
			)
				deck.flags_ ^= DECK_FLAG_LOOPING;
		}

		if (pressed & drivers::DECK_BTN_PLAY)
			deck.flags_ ^= DECK_FLAG_PLAYING;

		// As the monitor button doubles as a shift button, monitoring should
		// only be toggled when the button is released and no other button was
		// pressed while it was held down.
		if (released & drivers::DECK_BTN_MONITOR) {
			if (!(deck.flags_ & DECK_FLAG_SHIFT_USED))
				deck.flags_ ^= DECK_FLAG_MONITORING;
		}

		deck.flags_ &= ~DECK_FLAG_SHIFT_USED;
	}
}

AudioTask &AudioTask::instance(void) {
	static AudioTask task;

	return task;
}

}
