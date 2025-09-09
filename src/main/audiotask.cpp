
#include <stdint.h>
#include "src/main/drivers/audio.hpp"
#include "src/main/drivers/input.hpp"
#include "src/main/drivers/inputdefs.hpp"
#include "src/main/dsp/dsp.hpp"
#include "src/main/util/templates.hpp"
#include "src/main/audiotask.hpp"
#include "src/main/iotask.hpp"
#include "src/main/sst.hpp"
#include "src/main/taskbase.hpp"

/* Deck object */


void DeckAudio::init_(void) {
	offset_    = 0;
	cueOffset_ = 0;
	loopStart_ = 0;
	loopEnd_   = 0xffffffff;
	step_      = 0;

	flags_ = 0;
	filter_.reset();
}

void DeckAudio::process_(void) {
	// Do nothing and output silence if the deck is not yet ready to play. This
	// flag acts as a lock for the .sst reader; when cleared, a new file can be
	// safely loaded (even by another task).
	if (!(flags_ & DECK_FLAG_READY)) {
		util::clear(buffer_);
		return;
	}

	sampler_.process(
		buffer_[0],
		reader_,
		offset_,
		step_,
		AUDIO_BUFFER_SIZE
	);

	for (int i = 0; i < sst::NUM_CHANNELS; i++) {
		filter_.process(
			&buffer_[0][i],
			&buffer_[0][i],
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

void DeckAudio::updateMeasuredSpeed_(int16_t value, float dt) {
	float speed = float(value) / dt;
	speed      /= float(drivers::DECK_STEPS_PER_REV);
	speed      /= DECK_TARGET_RPM / 60.0f;

	// TODO: apply a filter here to stabilize playback speed
	speed *= float(reader_.getHeader()->info.sampleRate);
	speed *= float(sst::SAMPLE_OFFSET_UNIT);
	step_  = int(speed);

	// Adjust the cache eviction strategy depending on whether the track is
	// being played in reverse.
	reader_.setEvictionMode(
		(value >= 0) ? sst::EVICT_LOWEST : sst::EVICT_HIGHEST
	);
}

void DeckAudio::updateFilter_(uint8_t value) {
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

void AudioTask::mainInit_(void) {
	for (auto &deck : decks_)
		deck.init_();
}

void AudioTask::mainLoop_(void) {
	auto &audio = drivers::AudioDriver::instance();

	for (auto &deck : decks_)
		deck.process_();

	for (int i = 0; i < sst::NUM_CHANNELS; i++) {
		mainMixer_.process(
			&mainBuffer_[0][i],
			&decks_[0].buffer_[0][i],
			&decks_[1].buffer_[0][i],
			AUDIO_BUFFER_SIZE,
			sst::NUM_CHANNELS,
			sst::NUM_CHANNELS
		);
		monitorMixer_.process(
			&monitorBuffer_[0][i],
			&decks_[0].buffer_[0][i],
			&decks_[1].buffer_[0][i],
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

	// TODO: send deck state messages periodically here
	audio.feed(mainBuffer_[0], monitorBuffer_[0], AUDIO_BUFFER_SIZE);
}

void AudioTask::handleMessage_(const TaskMessage &message) {
	switch (message.type) {
		case MESSAGE_INPUTS:
			handleInputs_(message.inputs);
			break;

		case MESSAGE_LOAD_START:
			// Lock the deck while a new .sst file is being loaded into it by
			// another task.
			decks_[message.deck].flags_ &= ~DECK_FLAG_READY;
			break;

		case MESSAGE_LOAD_END:
			decks_[message.deck].flags_ |= DECK_FLAG_READY;
			break;

		default:
			break;
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
		if (selector && (deck.flags_ & DECK_FLAG_READY)) {
			const int variant    = deck.reader_.getVariant();
			const int maxVariant =
				deck.reader_.getHeader()->info.numVariants - 1;

			if ((selector < 0) && (variant > 0))
				deck.reader_.setVariant(variant - 1);
			else if ((selector > 0) && (variant < maxVariant))
				deck.reader_.setVariant(variant + 1);
		}

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
			if (deck.offset_ > deck.loopEnd_)
				deck.loopEnd_ = deck.offset_ + length;
		}

		if (pressed & drivers::DECK_BTN_LOOP_OUT) {
			if (deck.offset_ > deck.loopStart_) {
				deck.loopEnd_ = deck.offset_;
				deck.flags_  |= DECK_FLAG_LOOPING;
			}
		}

		if (pressed & drivers::DECK_BTN_RELOOP) {
			if (deck.loopEnd_ > deck.loopStart_)
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
