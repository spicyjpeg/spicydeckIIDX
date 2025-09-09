
#pragma once

#include <stddef.h>
#include <stdint.h>

namespace dsp {

using Sample = int16_t;

/* PID controller */

class PIDController {
private:
	float accumulator_;
	float lastError_;

public:
	float kp, ki, kd;
	float iclamp;

	inline PIDController(void) :
		kp(1.0f),
		ki(1.0f),
		kd(1.0f),
		iclamp(1.0f)
	{
		reset();
	}

	void reset(void);
	float update(float error, float dt);
};

/* Gain control */

class Gain {
private:
	int32_t gain_;

public:
	inline Gain(void) {
		configure(1.0f);
	}

	void configure(float gain);
	void process(
		Sample       *output,
		const Sample *input,
		size_t       numSamples,
		size_t       outputStride = 1,
		size_t       inputStride  = 1
	);
};

class Mixer {
private:
	int32_t a1_, a2_;

public:
	inline Mixer(void) {
		configure(0.5f, 0.5f);
	}

	void configure(float gain1, float gain2);
	void process(
		Sample       *output,
		const Sample *input1,
		const Sample *input2,
		size_t       numSamples,
		size_t       outputStride = 1,
		size_t       inputStride  = 1
	);
};

/* Simple bitcrusher */

class Bitcrusher {
private:
	uint32_t step_;

	uint32_t accumulator_;
	Sample   lastSample_;

public:
	inline Bitcrusher(void) {
		configure(1.0f);
		reset();
	}

	// Ratio must be specified as (output sample rate / input sample rate)
	void configure(float ratio);
	void reset(void);
	void process(
		Sample       *output,
		const Sample *input,
		size_t       numSamples,
		size_t       outputStride = 1,
		size_t       inputStride  = 1
	);
};

/* Biquad filter */

enum BiquadFilterType {
	FILTER_LOWPASS      = 0,
	FILTER_HIGHPASS     = 1,
	FILTER_BANDPASS     = 2,
	FILTER_BANDPASS_ALT = 3,
	FILTER_ALLPASS      = 4,
	FILTER_NOTCH        = 5
};

class BiquadFilter {
private:
	int32_t a1_, a2_;
	int32_t b0_, b1_, b2_;

	int32_t sa1_, sa2_;
	int32_t sb1_, sb2_;

public:
	inline BiquadFilter(void) {
		configure(FILTER_LOWPASS, 1.0f);
		reset();
	}

	// Cutoff must be specified as (cutoff frequency / sample rate * 2) ratio
	void configure(
		BiquadFilterType type,
		float            cutoff,
		float            resonance = 1.0f
	);
	void configurePeaking(
		float cutoff,
		float resonance = 1.0f,
		float gain      = 1.0f
	);
	void reset(void);
	void process(
		Sample       *output,
		const Sample *input,
		size_t       numSamples,
		size_t       outputStride = 1,
		size_t       inputStride  = 1
	);
};

/* 4-bit waveform data generator */

static constexpr int     WAVEFORM_SAMPLE_RATE = 32;
static constexpr uint8_t WAVEFORM_RANGE       = 12;

class WaveformEncoder {
private:
	int    accumulator_;
	Sample currentPeak_;
	int8_t lastNibble_;

public:
	inline WaveformEncoder(void) {
		reset();
	}

	void reset(void);
	size_t encode(
		uint8_t      *output,
		const Sample *input,
		int          sampleRate,
		size_t       numSamples,
		size_t       inputStride = 1
	);
};

}
