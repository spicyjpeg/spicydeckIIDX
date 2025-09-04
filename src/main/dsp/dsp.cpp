
#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include "src/main/util/templates.hpp"
#include "src/main/dsp/dsp.hpp"
#include "src/main/defs.hpp"

namespace dsp {

[[gnu::always_inline]] static inline int clampSample_(int value) {
	return util::clamp(value, INT16_MIN, INT16_MAX);
}

/* PID controller */

IRAM_ATTR void PIDController::reset(void) {
	accumulator_ = 0.0f;
	lastError_   = 0.0f;
}

IRAM_ATTR float PIDController::update(float error, float dt) {
	assert(dt > 0.0f);

	accumulator_ += error * dt;
	accumulator_  = util::clamp(accumulator_, -iclamp, iclamp);

	const auto delta = (error - lastError_) / dt;
	lastError_       = error;

	return 0
		+ kp * error
		+ ki * accumulator_
		+ kd * delta;
}

/* Gain control */

static constexpr int GAIN_UNIT_ = 1 << 14;

IRAM_ATTR void Gain::configure(float gain) {
	gain = util::clamp(gain, 0.0f, 1.0f);

	gain_ = int32_t(float(GAIN_UNIT_) * sinf(gain * (float(M_PI) / 2.0f)));
}

IRAM_ATTR void Gain::process(
	Sample       *output,
	const Sample *input,
	size_t       numSamples,
	size_t       outputStride,
	size_t       inputStride
) {
	const int gain = gain_;

	for (; numSamples > 0; numSamples--) {
		int mixed = gain * int(*input);
		mixed    /= GAIN_UNIT_;

		*output = Sample(clampSample_(mixed));
		output += outputStride;
		input  += inputStride;
	}
}

IRAM_ATTR void Mixer::configure(float gain1, float gain2) {
	gain1 = util::clamp(gain1, 0.0f, 1.0f);
	gain2 = util::clamp(gain2, 0.0f, 1.0f);

	a1_ = int32_t(float(GAIN_UNIT_) * sinf(gain1 * (float(M_PI) / 2.0f)));
	a2_ = int32_t(float(GAIN_UNIT_) * sinf(gain2 * (float(M_PI) / 2.0f)));
}

IRAM_ATTR void Mixer::process(
	Sample       *output,
	const Sample *input1,
	const Sample *input2,
	size_t       numSamples,
	size_t       outputStride,
	size_t       inputStride
) {
	const int a1 = a1_, a2 = a2_;

	for (; numSamples > 0; numSamples--) {
		int mixed = a1 * int(*input1);
		mixed    += a2 * int(*input2);
		mixed    /= GAIN_UNIT_;

		*output = Sample(clampSample_(mixed));
		output += outputStride;
		input1 += inputStride;
		input2 += inputStride;
	}
}

/* Simple bitcrusher */

static constexpr int BITCRUSHER_UNIT_ = 1 << 16;

IRAM_ATTR void Bitcrusher::configure(float ratio) {
	ratio = util::clamp(ratio, 0.001f, 1.0f);

	step_ = uint32_t(float(BITCRUSHER_UNIT_) / ratio);
}

IRAM_ATTR void Bitcrusher::reset(void) {
	accumulator_ = 0;
	lastSample_  = 0;
}

IRAM_ATTR void Bitcrusher::process(
	Sample       *output,
	const Sample *input,
	size_t       numSamples,
	size_t       outputStride,
	size_t       inputStride
) {
	int    accumulator = accumulator_;
	Sample lastSample  = lastSample_;

	for (; numSamples > 0; numSamples--) {
		if (accumulator >= step_) {
			accumulator -= step_;
			lastSample   = *input;
		}

		*output = lastSample;
		output += outputStride;
		input  += inputStride;
	}

	accumulator_ = uint32_t(accumulator);
	lastSample_  = lastSample;
}

/* Biquad filter */

static constexpr int FILTER_UNIT_ = 1 << 14;

IRAM_ATTR void BiquadFilter::configure(
	BiquadFilterType type,
	float            cutoff,
	float            resonance
) {
	cutoff    = util::clamp(cutoff,    0.001f, 0.999f);
	resonance = util::clamp(resonance, 0.01f,  10.0f);

	// See https://www.w3.org/TR/audio-eq-cookbook.
	const float omega    = cutoff * float(M_PI);
	const float cosOmega = cosf(omega);
	const float alpha    = sinf(omega) / (2.0f * resonance);

	const float a0 =  1.0f + alpha;
	const float a1 = -2.0f * cosOmega;
	const float a2 =  1.0f - alpha;

	float b0, b1, b2;

	switch (type) {
		case FILTER_LOWPASS:
			b0 = (1.0f - cosOmega) / 2.0f;
			b1 = (1.0f - cosOmega);
			b2 = (1.0f - cosOmega) / 2.0f;
			break;

		case FILTER_HIGHPASS:
			b0 =  (1.0f + cosOmega) / 2.0f;
			b1 = -(1.0f + cosOmega);
			b2 =  (1.0f + cosOmega) / 2.0f;
			break;

		case FILTER_BANDPASS:
			b0 =  resonance * alpha;
			b1 = 0.0f;
			b2 = -resonance * alpha;
			break;

		case FILTER_BANDPASS_ALT:
			b0 = alpha;
			b1 = 0.0f;
			b2 = alpha;
			break;

		case FILTER_ALLPASS:
			b0 = a2;
			b1 = a1;
			b2 = a0;
			break;

		case FILTER_NOTCH:
			b0 = 1.0f;
			b1 = a1;
			b2 = 1.0f;
			break;

		default:
			assert(false);
	}

	a1_ = int32_t(float(FILTER_UNIT_) * a1 / a0);
	a2_ = int32_t(float(FILTER_UNIT_) * a2 / a0);
	b0_ = int32_t(float(FILTER_UNIT_) * b0 / a0);
	b1_ = int32_t(float(FILTER_UNIT_) * b1 / a0);
	b2_ = int32_t(float(FILTER_UNIT_) * b2 / a0);
}

IRAM_ATTR void BiquadFilter::configurePeaking(
	float cutoff,
	float resonance,
	float gain
) {
	cutoff    = util::clamp(cutoff,    0.0f,   1.0f);
	resonance = util::clamp(resonance, 0.01f, 10.0f);

	const float omega    = cutoff * float(M_PI);
	const float cosOmega = cosf(omega);
	const float alpha    = sinf(omega) / (2.0f * resonance);
	const float amp      = powf(10.0f, gain / 40.0f);

	const float a0 =  1.0f + (alpha / amp);
	const float a1 = -2.0f * cosOmega;
	const float a2 =  1.0f - (alpha / amp);
	const float b0 =  1.0f + (alpha * amp);
	const float b1 = -2.0f * cosOmega;
	const float b2 =  1.0f - (alpha * amp);

	a1_ = int32_t(float(FILTER_UNIT_) * a1 / a0);
	a2_ = int32_t(float(FILTER_UNIT_) * a2 / a0);
	b0_ = int32_t(float(FILTER_UNIT_) * b0 / a0);
	b1_ = int32_t(float(FILTER_UNIT_) * b1 / a0);
	b2_ = int32_t(float(FILTER_UNIT_) * b2 / a0);
}

IRAM_ATTR void BiquadFilter::reset(void) {
	sa1_ = 0;
	sa2_ = 0;
	sb1_ = 0;
	sb2_ = 0;
}

IRAM_ATTR void BiquadFilter::process(
	Sample       *output,
	const Sample *input,
	size_t       numSamples,
	size_t       outputStride,
	size_t       inputStride
) {
	const int a1 = a1_, a2 = a2_;
	const int b0 = b0_, b1 = b1_, b2 = b2_;

	int sa1 = sa1_, sa2 = sa2_;
	int sb1 = sb1_, sb2 = sb2_;

	for (; numSamples > 0; numSamples--) {
		const int sample = *input;

		int filtered = b0 * sample;
		filtered    += b1 * sb1;
		filtered    += b2 * sb2;
		filtered    -= a1 * sa1;
		filtered    -= a2 * sa2;
		filtered    /= FILTER_UNIT_;

		*output = Sample(clampSample_(filtered));
		output += outputStride;
		input  += inputStride;

		sa2 = sa1;
		sa1 = filtered;
		sb2 = sb1;
		sb1 = sample;
	}

	sa1_ = int32_t(sa1);
	sa2_ = int32_t(sa2);
	sb1_ = int32_t(sb1);
	sb2_ = int32_t(sb2);
}

}
