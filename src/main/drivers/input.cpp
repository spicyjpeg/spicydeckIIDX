
#include <assert.h>
#include <stdint.h>
#include "driver/i2c_master.h"
#include "driver/pulse_cnt.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "src/main/drivers/input.hpp"
#include "src/main/drivers/inputdefs.hpp"
#include "src/main/util/templates.hpp"
#include "src/main/defs.hpp"

namespace drivers {

static const char TAG_[]{ "input" };

/* AS5600 encoder object */

static constexpr int AS5600_I2C_BAUD_RATE_ = 400000;
static constexpr int AS5600_I2C_TIMEOUT_   = 500;

static const i2c_device_config_t AS5600_I2C_CONFIG_{
	.dev_addr_length = I2C_ADDR_BIT_LEN_7,
	.device_address  = AS5600_I2C_ADDRESS,
	.scl_speed_hz    = AS5600_I2C_BAUD_RATE_,
	.scl_wait_us     = 0,
	.flags           = { .disable_ack_check = false }
};

static const uint8_t A5600_INIT_[]{
	AS5600_CONF_H, // Address
	0              // CONF_H
		| AS5600_CONF_H_SF_16X
		| AS5600_CONF_H_FTH_9,
	0              // CONF_L
		| AS5600_CONF_L_PM_NOM
		| AS5600_CONF_L_HYST_3
		| AS5600_CONF_L_OUTS_ANALOG_0_100
};

bool AS5600Encoder::init_(i2c_master_bus_handle_t i2c) {
	if (device_)
		release_();

	i2c_master_bus_add_device(i2c, &AS5600_I2C_CONFIG_, &device_);

	if (i2c_master_transmit(
		device_,
		A5600_INIT_,
		sizeof(A5600_INIT_),
		AS5600_I2C_TIMEOUT_ / portTICK_PERIOD_MS
	) != ESP_OK) {
		ESP_LOGE(TAG_, "AS5600 initialization failed");

		release_();
		return false;
	}

	// Initialize lastAngle_ by probing the current angle.
	getDelta_();
	return true;
}

void AS5600Encoder::release_(void) {
	if (!device_)
		return;

	i2c_master_bus_rm_device(device_);
	device_ = nullptr;
}

int AS5600Encoder::getDelta_(void) {
	assert(device_);

	const uint8_t request[]{ AS5600_RAWANGLE_H };
	uint8_t       response[2];

	if (i2c_master_transmit_receive(
		device_,
		request,
		sizeof(request),
		response,
		sizeof(response),
		AS5600_I2C_TIMEOUT_ / portTICK_PERIOD_MS
	) != ESP_OK) {
		ESP_LOGE(TAG_, "AS5600 is unresponsive");
		return 0;
	}

	auto angle = util::concat2(response[1], response[0]);
	int  delta = int(angle) - int(lastAngle_);
	lastAngle_ = angle;

	// Handle wrapping. This assumes the difference between two measurements is
	// always less than half a revolution.
	delta += DECK_STEPS_PER_REV; // Workaround for % operator sign behavior
	delta += DECK_STEPS_PER_REV / 2;
	delta %= DECK_STEPS_PER_REV;
	delta -= DECK_STEPS_PER_REV / 2;
	return delta;
}

/* Quadrature encoder object */

static constexpr int PCNT_MIN_PULSE_TIME_     = 1500;
static constexpr int PCNT_MAX_STEPS_PER_POLL_ = 32;

static const pcnt_unit_config_t PCNT_UNIT_CONFIG_{
	.low_limit     = -PCNT_MAX_STEPS_PER_POLL_,
	.high_limit    = PCNT_MAX_STEPS_PER_POLL_,
	.intr_priority = 0,
	.flags         = {
		.accum_count = false
	}
};

static const pcnt_glitch_filter_config_t GLITCH_FILTER_CONFIG_{
	.max_glitch_ns = PCNT_MIN_PULSE_TIME_
};

static const pcnt_chan_config_t PCNT_CHANNEL_CONFIG_[]{
	{
		.edge_gpio_num  = defs::IO_SELECTOR_A,
		.level_gpio_num = defs::IO_SELECTOR_B,
		.flags          = {
			.invert_edge_input   = true,
			.invert_level_input  = true,
			.virt_edge_io_level  = false,
			.virt_level_io_level = false,
			.io_loop_back        = false
		}
	}, {
		.edge_gpio_num  = defs::IO_SELECTOR_B,
		.level_gpio_num = defs::IO_SELECTOR_A,
		.flags          = {
			.invert_edge_input   = true,
			.invert_level_input  = true,
			.virt_edge_io_level  = false,
			.virt_level_io_level = false,
			.io_loop_back        = false
		}
	}
};

void QuadratureEncoder::init_(void) {
	if (unit_)
		release_();

	pcnt_new_unit(&PCNT_UNIT_CONFIG_, &unit_);
	pcnt_unit_set_glitch_filter(unit_, &GLITCH_FILTER_CONFIG_);

	for (int i = 0; i < 2; i++) {
		pcnt_new_channel(unit_, &PCNT_CHANNEL_CONFIG_[i], &channels_[i]);
		pcnt_channel_set_level_action(
			channels_[i],
			PCNT_CHANNEL_LEVEL_ACTION_KEEP,
			PCNT_CHANNEL_LEVEL_ACTION_INVERSE
		);
	}

	pcnt_channel_set_edge_action(
		channels_[0],
		PCNT_CHANNEL_EDGE_ACTION_INCREASE,
		PCNT_CHANNEL_EDGE_ACTION_DECREASE
	);
	pcnt_channel_set_edge_action(
		channels_[1],
		PCNT_CHANNEL_EDGE_ACTION_DECREASE,
		PCNT_CHANNEL_EDGE_ACTION_INCREASE
	);

	pcnt_unit_enable(unit_);
	pcnt_unit_clear_count(unit_);
	pcnt_unit_start(unit_);
}

void QuadratureEncoder::release_(void) {
	if (!unit_)
		return;

	pcnt_unit_stop(unit_);
	pcnt_unit_disable(unit_);

	for (auto channel : channels_)
		pcnt_del_channel(channel);

	pcnt_del_unit(unit_);
	unit_ = nullptr;
}

int QuadratureEncoder::getDelta_(void) {
	assert(unit_);

	int value;
	pcnt_unit_get_count(unit_, &value);
	pcnt_unit_clear_count(unit_);

	return value;
}

/* IOP object */

static constexpr int IOP_I2C_BAUD_RATE_ = 400000;
static constexpr int IOP_I2C_TIMEOUT_   = 500;

static const i2c_device_config_t IOP_I2C_CONFIG_{
	.dev_addr_length = I2C_ADDR_BIT_LEN_7,
	.device_address  = IOP_I2C_ADDRESS,
	.scl_speed_hz    = IOP_I2C_BAUD_RATE_,
	.scl_wait_us     = 0,
	.flags           = { .disable_ack_check = false }
};

bool IOP::init_(i2c_master_bus_handle_t i2c) {
	if (device_)
		release_();

	i2c_master_bus_add_device(i2c, &IOP_I2C_CONFIG_, &device_);
	util::clear(version_);

	const uint8_t request[2]{ IOP_CMD_GET_VERSION, 0 };

	if (i2c_master_transmit_receive(
		device_,
		request,
		sizeof(request),
		version_,
		sizeof(version_) - 1,
		IOP_I2C_TIMEOUT_ / portTICK_PERIOD_MS
	) != ESP_OK) {
		ESP_LOGE(TAG_, "IOP initialization failed");
		return false;
	}

	ESP_LOGI(TAG_, "IOP firmware version: %s", version_);
	return true;
}

void IOP::release_(void) {
	if (!device_)
		return;

	i2c_master_bus_rm_device(device_);
	device_ = nullptr;
}

bool IOP::poll_(IOPInputState &output) {
	assert(device_);

	const uint8_t request[2]{ IOP_CMD_POLL_INPUTS, 0 };

	if (i2c_master_transmit_receive(
		device_,
		request,
		sizeof(request),
		reinterpret_cast<uint8_t *>(&output),
		sizeof(output),
		IOP_I2C_TIMEOUT_ / portTICK_PERIOD_MS
	) != ESP_OK) {
		ESP_LOGE(TAG_, "IOP is unresponsive");
		return false;
	}

	return true;
}

/* Input manager class */

static const i2c_master_bus_config_t I2C_CONFIG_[NUM_DECKS]{
	{
		.i2c_port          = defs::LEFT_DECK_I2C_PORT,
		.sda_io_num        = gpio_num_t(defs::IO_LEFT_DECK_SDA),
		.scl_io_num        = gpio_num_t(defs::IO_LEFT_DECK_SCL),
		.clk_source        = I2C_CLK_SRC_DEFAULT,
		.glitch_ignore_cnt = 7,
		.intr_priority     = 0,
		.trans_queue_depth = 0,
		.flags             = {
			.enable_internal_pullup = true,
			.allow_pd               = false
		}
	}, {
		.i2c_port          = defs::RIGHT_DECK_I2C_PORT,
		.sda_io_num        = gpio_num_t(defs::IO_RIGHT_DECK_SDA),
		.scl_io_num        = gpio_num_t(defs::IO_RIGHT_DECK_SCL),
		.clk_source        = I2C_CLK_SRC_DEFAULT,
		.glitch_ignore_cnt = 7,
		.intr_priority     = 0,
		.trans_queue_depth = 0,
		.flags             = {
			.enable_internal_pullup = true,
			.allow_pd               = false
		}
	}
};

InputDriver &InputDriver::instance(void) {
	static InputDriver driver;

	return driver;
}

bool InputDriver::init(void) {
	if (i2c_[0] || i2c_[1])
		release();

	for (int i = 0; i < NUM_DECKS; i++) {
		i2c_new_master_bus(&I2C_CONFIG_[i], &i2c_[i]);

		if (!as5600_[i].init_(i2c_[i])) {
			release();
			return false;
		}
	}

	// The IOP shares the I2C bus with the right deck's AS5600.
	if (!iop_.init_(i2c_[1])) {
		release();
		return false;
	}

	selector_.init_();
	lastButtons_ = 0;
	return true;
}

void InputDriver::release(void) {
	if (!i2c_[0] && !i2c_[1])
		return;

	selector_.release_();
	iop_.release_();

	for (int i = 0; i < NUM_DECKS; i++) {
		if (!i2c_[i])
			continue;

		as5600_[i].release_();
		i2c_del_master_bus(i2c_[i]);
		i2c_[i] = nullptr;
	}
}

void InputDriver::poll(InputState &output) {
	for (int i = 0; i < NUM_DECKS; i++)
		output.decks[i] = as5600_[i].getDelta_();

	output.selector = selector_.getDelta_();

	IOPInputState iopState;

	if (iop_.poll_(iopState)) {
		output.buttonsPressed  =  iopState.buttons & ~lastButtons_;
		output.buttonsReleased = ~iopState.buttons &  lastButtons_;
		output.buttonsHeld     =  iopState.buttons;
		util::copy(output.analog, iopState.analog);

		lastButtons_ = iopState.buttons;
	} else {
		output.buttonsPressed  = 0;
		output.buttonsReleased = 0;
		output.buttonsHeld     = 0;
		util::clear(output.analog);

		lastButtons_ = 0;
	}

	auto time = esp_timer_get_time();
	output.dt = float(time - lastPoll_) / 1000000.0f;
	lastPoll_ = time;
}

}
