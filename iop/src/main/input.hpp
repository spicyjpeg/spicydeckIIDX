
#pragma once

#include <stdint.h>
#include "ch32v003/registers.h"
#include "main/util/bitfield.hpp"
#include "main/defs.hpp"

/* Analog inputs */

class AnalogInputs {
private:
	// Analog inputs are double buffered in order to allow getInputs() to always
	// return a valid result, even if called while a conversion is in progress.
	uint16_t buffers_[2][defs::NUM_ANALOG_INPUTS];

public:
	inline void update(void) const {
		ADC_CTLR2 |= ADC_CTLR2_SWSTART;
	}

	AnalogInputs(void);
	void init(void);
	void getInputs(uint8_t *output) const;
};

extern AnalogInputs analogInputs;

/* Button matrix */

static constexpr int NUM_MATRIX_ROWS    = 5;
static constexpr int NUM_MATRIX_COLUMNS = 2;

class ButtonMatrix {
private:
	defs::ButtonMask states_[3];

public:
	inline defs::ButtonMask getButtons(void) const {
		// This is a rather poor debouncing strategy, but it should be good
		// enough.
		return util::reduceAND(states_, sizeof(states_));
	}

	ButtonMatrix(void);
	void init(void);
	void update(void);
};

extern ButtonMatrix buttonMatrix;
