
#pragma once

#include <stddef.h>
#include <stdint.h>

namespace drivers {

/* AS5600 register definitions */

static constexpr uint8_t AS5600_I2C_ADDRESS  = 0x36;
static constexpr uint8_t AS5600L_I2C_ADDRESS = 0x40;

enum AS5600Register : uint8_t {
	AS5600_ZMCO        = 0x00,
	AS5600_ZPOS_H      = 0x01,
	AS5600_ZPOS_L      = 0x02,
	AS5600_MPOS_H      = 0x03,
	AS5600_MPOS_L      = 0x04,
	AS5600_MANG_H      = 0x05,
	AS5600_MANG_L      = 0x06,
	AS5600_CONF_H      = 0x07,
	AS5600_CONF_L      = 0x08,
	AS5600_STATUS      = 0x0b,
	AS5600_RAWANGLE_H  = 0x0c,
	AS5600_RAWANGLE_L  = 0x0d,
	AS5600_ANGLE_H     = 0x0e,
	AS5600_ANGLE_L     = 0x0f,
	AS5600_AGC         = 0x1a,
	AS5600_MAGNITUDE_H = 0x1b,
	AS5600_MAGNITUDE_L = 0x1c,
	AS5600_I2CADDR     = 0x20, // AS5600L only
	AS5600_I2CUPDT     = 0x21, // AS5600L only
	AS5600_BURN        = 0xff
};

enum AS5600ConfHFlag : uint8_t {
	AS5600_CONF_H_SF_BITMASK  = 3 << 0, // Slow Filter
	AS5600_CONF_H_SF_16X      = 0 << 0,
	AS5600_CONF_H_SF_8X       = 1 << 0,
	AS5600_CONF_H_SF_4X       = 2 << 0,
	AS5600_CONF_H_SF_2X       = 3 << 0,
	AS5600_CONF_H_FTH_BITMASK = 7 << 2, // Fast Filter Threshold
	AS5600_CONF_H_FTH_OFF     = 0 << 2,
	AS5600_CONF_H_FTH_6       = 1 << 2,
	AS5600_CONF_H_FTH_7       = 2 << 2,
	AS5600_CONF_H_FTH_9       = 3 << 2,
	AS5600_CONF_H_FTH_18      = 4 << 2,
	AS5600_CONF_H_FTH_21      = 5 << 2,
	AS5600_CONF_H_FTH_24      = 6 << 2,
	AS5600_CONF_H_FTH_10      = 7 << 2,
	AS5600_CONF_H_WD          = 1 << 5  // Watchdog
};

enum AS5600ConfLFlag : uint8_t {
	AS5600_CONF_L_PM_BITMASK        = 3 << 0, // Power Mode
	AS5600_CONF_L_PM_NOM            = 0 << 0,
	AS5600_CONF_L_PM_LPM1           = 1 << 0,
	AS5600_CONF_L_PM_LPM2           = 2 << 0,
	AS5600_CONF_L_PM_LPM            = 3 << 0,
	AS5600_CONF_L_HYST_BITMASK      = 3 << 2, // Hysteresis
	AS5600_CONF_L_HYST_OFF          = 0 << 2,
	AS5600_CONF_L_HYST_1            = 1 << 2,
	AS5600_CONF_L_HYST_2            = 2 << 2,
	AS5600_CONF_L_HYST_3            = 3 << 2,
	AS5600_CONF_L_OUTS_BITMASK      = 3 << 4, // Output Stage
	AS5600_CONF_L_OUTS_ANALOG_0_100 = 0 << 4,
	AS5600_CONF_L_OUTS_ANALOG_10_90 = 1 << 4,
	AS5600_CONF_L_OUTS_PWM          = 2 << 4,
	AS5600_CONF_L_PWMF_BITMASK      = 3 << 6, // PWM Frequency
	AS5600_CONF_L_PWMF_115HZ        = 0 << 6,
	AS5600_CONF_L_PWMF_230HZ        = 1 << 6,
	AS5600_CONF_L_PWMF_460HZ        = 2 << 6,
	AS5600_CONF_L_PWMF_920HZ        = 3 << 6
};

enum AS5600StatusFlag : uint8_t {
	AS5600_STATUS_MH = 1 << 3, // AGC minimum gain overflow, magnet too strong
	AS5600_STATUS_ML = 1 << 4, // AGC maximum gain overflow, magnet too weak
	AS5600_STATUS_MD = 1 << 5  // Magnet was detected
};

enum AS5600BurnCommand : uint8_t {
	AS5600_BURN_SETTING = 1 << 6,
	AS5600_BURN_ANGLE   = 1 << 7
};

/* IOP command definitions and data structures */

static constexpr uint8_t IOP_I2C_ADDRESS   = 0x10;
static constexpr size_t  NUM_ANALOG_INPUTS = 8;

enum IOPCommand : uint8_t {
	IOP_CMD_GET_LAST_INPUTS = 'l',
	IOP_CMD_POLL_INPUTS     = 'p',
	IOP_CMD_GET_VERSION     = 'v'
};

enum DeckButtonFlag : uint16_t {
	DECK_BTN_BITMASK = 31 << 0,

	// Default actions
	DECK_BTN_LOOP_IN  = 1 << 0,
	DECK_BTN_LOOP_OUT = 1 << 1,
	DECK_BTN_RELOOP   = 1 << 2,
	DECK_BTN_PLAY     = 1 << 3,
	DECK_BTN_MONITOR  = 1 << 4,

	// Alternate actions (while the shift button is held)
	DECK_BTN_RESTART  = 1 << 0,
	DECK_BTN_CUE_JUMP = 1 << 1,
	DECK_BTN_CUE_SET  = 1 << 2,
	DECK_BTN_REVERSE  = 1 << 3,
	DECK_BTN_SHIFT    = 1 << 4
};

enum ButtonFlag : uint16_t {
	// IOP buttons
	BTN_LEFT_LOOP_IN   = DECK_BTN_LOOP_IN  << 0,
	BTN_LEFT_LOOP_OUT  = DECK_BTN_LOOP_OUT << 0,
	BTN_LEFT_RELOOP    = DECK_BTN_RELOOP   << 0,
	BTN_LEFT_PLAY      = DECK_BTN_PLAY     << 0,
	BTN_LEFT_MONITOR   = DECK_BTN_MONITOR  << 0,
	BTN_RIGHT_LOOP_IN  = DECK_BTN_LOOP_IN  << 5,
	BTN_RIGHT_LOOP_OUT = DECK_BTN_LOOP_OUT << 5,
	BTN_RIGHT_RELOOP   = DECK_BTN_RELOOP   << 5,
	BTN_RIGHT_PLAY     = DECK_BTN_PLAY     << 5,
	BTN_RIGHT_MONITOR  = DECK_BTN_MONITOR  << 5,

	// ESP32 buttons
	BTN_SELECTOR = 1 << 10
};

enum AnalogInput {
	ANALOG_LEFT_FILTER    = 0,
	ANALOG_RIGHT_FILTER   = 1,
	ANALOG_LEFT_SPEED     = 2,
	ANALOG_RIGHT_SPEED    = 3,
	ANALOG_MAIN_VOLUME    = 4,
	ANALOG_MONITOR_VOLUME = 5,
	ANALOG_CROSSFADE      = 6,
	ANALOG_EFFECT_DEPTH   = 7
};

using ButtonMask = uint16_t;

struct [[gnu::packed]] IOPInputState {
	ButtonMask buttons;
	uint8_t    analog[NUM_ANALOG_INPUTS];
};

}
