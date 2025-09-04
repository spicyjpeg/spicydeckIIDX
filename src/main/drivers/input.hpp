
#pragma once

#include <stdint.h>
#include "driver/i2c_types.h"
#include "driver/pulse_cnt.h"
#include "src/main/drivers/inputdefs.hpp"
#include "src/main/util/templates.hpp"

namespace drivers {

/* Encoder objects */

class AS5600Encoder {
	friend class InputDriver;

private:
	i2c_master_dev_handle_t device_;
	uint16_t                lastAngle_;

	inline AS5600Encoder(void) :
		device_(nullptr)
	{}
	inline ~AS5600Encoder(void) {
		release_();
	}

	bool init_(i2c_master_bus_handle_t i2c);
	void release_(void);

	int getDelta_(void);
};

class QuadratureEncoder {
	friend class InputDriver;

private:
	pcnt_unit_handle_t    unit_;
	pcnt_channel_handle_t channels_[2];

	inline QuadratureEncoder(void) :
		unit_(nullptr)
	{}
	inline ~QuadratureEncoder(void) {
		release_();
	}

	void init_(void);
	void release_(void);

	int getDelta_(void);
};

/* IOP object */

class IOP {
	friend class InputDriver;

private:
	i2c_master_dev_handle_t device_;
	uint8_t                 version_[17];

	inline IOP(void) :
		device_(nullptr)
	{}
	inline ~IOP(void) {
		release_();
	}

	bool init_(i2c_master_bus_handle_t i2c);
	void release_(void);

	bool poll_(IOPInputState &output);
};

/* Input manager class */

static constexpr size_t NUM_DECKS          = 2;
static constexpr int    DECK_STEPS_PER_REV = 1 << 12;

struct InputState {
public:
	// Time since last poll
	float dt;

	// Encoder inputs
	int16_t decks[NUM_DECKS];
	int16_t selector;

	// IOP inputs
	ButtonMask buttonsPressed, buttonsReleased, buttonsHeld;
	uint8_t    analog[NUM_ANALOG_INPUTS];
};

class InputDriver {
private:
	i2c_master_bus_handle_t i2c_[NUM_DECKS];

	AS5600Encoder     as5600_[NUM_DECKS];
	QuadratureEncoder selector_;
	IOP               iop_;

	int64_t    lastPoll_;
	ButtonMask lastButtons_;

	inline InputDriver(void) {
		util::clear(i2c_);
	}
	inline ~InputDriver(void) {
		release();
	}

public:
	static InputDriver &instance(void);
	bool init(void);
	void release(void);

	void poll(InputState &output);
};

}
