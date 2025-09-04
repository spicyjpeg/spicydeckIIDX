
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

namespace defs {

/* Macros */

#ifndef VERSION
#define VERSION "<unknown build>"
#endif

#ifdef NDEBUG
#define VERSION_STRING VERSION
#else
#define VERSION_STRING VERSION "-debug"
#endif

#ifdef ENABLE_LOGGING
#define LOG(fmt, ...) \
	printf( \
		"%s(%d): " fmt "\n", __func__, __LINE__ __VA_OPT__(,) __VA_ARGS__ \
	)
#else
#define LOG(fmt, ...)
#endif

/* GPIO pin definitions */

enum GPIOAPin {
	PA_ADC_IN1 = 1,
	PA_ADC_IN0 = 2
} ;

enum GPIOCPin {
	PC_MATRIX_ROW0 = 0,
	PC_I2C_SDA     = 1,
	PC_I2C_SCL     = 2,
	PC_MATRIX_ROW1 = 3,
	PC_ADC_IN4     = 4,
	PC_MATRIX_ROW2 = 5,
	PC_MATRIX_ROW3 = 6,
	PC_MATRIX_ROW4 = 7
};

enum GPIODPin {
	PD_MATRIX_COL0 = 0,
	PD_ADC_IN3     = 2,
	PD_ADC_IN4     = 3,
	PD_ADC_IN7     = 4,
	PD_ADC_IN5     = 5,
	PD_ADC_IN6     = 6,
	PD_MATRIX_COL1 = 7
};

/* IOP command definitions and data structures */

static constexpr uint8_t IOP_I2C_ADDRESS   = 0x10;
static constexpr size_t  NUM_ANALOG_INPUTS = 8;

enum IOPCommand : uint8_t {
	IOP_CMD_GET_LAST_INPUTS = 'l',
	IOP_CMD_POLL_INPUTS     = 'p',
	IOP_CMD_GET_VERSION     = 'v'
};

using ButtonMask = uint16_t;

struct [[gnu::packed]] IOPInputState {
public:
	ButtonMask buttons;
	uint8_t    analog[NUM_ANALOG_INPUTS];
};

}
