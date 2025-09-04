
#pragma once

#include <stddef.h>
#include <stdint.h>

/* I2C slave object */

using I2CReadCallback  = size_t (*)(
	const uint8_t *request,
	size_t        requestLength,
	uint8_t       *response
);
using I2CWriteCallback = void (*)(
	const uint8_t *request,
	size_t        requestLength
);

static constexpr uint8_t I2C_MAX_REQUEST_LENGTH  =  4;
static constexpr uint8_t I2C_MAX_RESPONSE_LENGTH = 16;

class I2CSlave {
private:
	uint8_t request_ [I2C_MAX_REQUEST_LENGTH];
	uint8_t response_[I2C_MAX_RESPONSE_LENGTH];
	uint8_t requestOffset_, responseOffset_, responseLength_;

	void reset_(void);

public:
	I2CReadCallback  readCallback;
	I2CWriteCallback writeCallback;

	I2CSlave(void);
	void init(int hclk, uint8_t address);
	void handleEventInterrupt(void);
	void handleErrorInterrupt(void);
};

extern I2CSlave i2cSlave;
