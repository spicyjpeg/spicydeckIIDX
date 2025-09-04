
#include <stddef.h>
#include <stdint.h>
#include "ch32v003/registers.h"
#include "main/util/bitfield.hpp"
#include "main/util/hardware.hpp"
#include "main/defs.hpp"
#include "main/i2c.hpp"

/* I2C slave object */

static constexpr int PERIPH_CLOCK_ = 4000000;
static constexpr int BUS_CLOCK_    = 1000000 * 3;

I2CSlave i2cSlave;

I2CSlave::I2CSlave(void) :
	readCallback (nullptr),
	writeCallback(nullptr) {
	reset_();
}

void I2CSlave::reset_(void) {
	requestOffset_  = 0;
	responseOffset_ = 0;
	responseLength_ = 0;
}

void I2CSlave::init(int hclk, uint8_t address) {
	// Initialize the I2C interface and set it up as a slave device.
	util::enablePeripherals(
		RCC_APB1PCENR_I2C1EN,
		RCC_APB2PCENR_AFIOEN,
		RCC_APB2PCENR_IOPCEN
	);
	util::resetPeripherals(RCC_APB1PRSTR_I2C1RST);

	int periphDivider = (hclk + PERIPH_CLOCK_ / 2) / PERIPH_CLOCK_;
	int busDivider    = (hclk + BUS_CLOCK_    / 2) / BUS_CLOCK_;

	I2C_CTLR1  = 0;
	I2C_CTLR2  = 0
		| (periphDivider & I2C_CTLR2_FREQ_BITMASK)
		| I2C_CTLR2_ITERREN
		| I2C_CTLR2_ITEVTEN
		| I2C_CTLR2_ITBUFEN;
	I2C_OADDR1 = 0
		| ((address << 1) & I2C_OADDR1_ADD7_BITMASK)
		| I2C_OADDR1_ADDMODE_7;
	I2C_OADDR2 = 0;
	I2C_CKCFGR = 0
		| (busDivider & I2C_CKCFGR_CCR_BITMASK)
		| I2C_CKCFGR_FS;
	I2C_CTLR1  = 0
		| I2C_CTLR1_PE
		| I2C_CTLR1_ACK;

	// Set up the I2C GPIO pins.
	AFIO_PCFR1 = util::bitwiseTernary<uint32_t>(
		AFIO_PCFR1_I2C1_RM_BITMASK,
		AFIO_PCFR1_I2C1_RM_DEFAULT,
		AFIO_PCFR1
	);

	GPIOC_CFGLR = util::bitwiseTernary(
		util::repeatEachBit<uint32_t>(
			0
				| (1 << defs::PC_I2C_SDA)
				| (1 << defs::PC_I2C_SCL),
			4
		),
		util::repeatBitPattern<uint32_t>(
			0
				| GPIO_CFGLR_MODE_OUTPUT_10MHZ
				| GPIO_CFGLR_CNF_OUT_AF_OPEN_DRAIN,
			4
		),
		GPIOC_CFGLR
	);

	reset_();
}

void I2CSlave::handleEventInterrupt(void) {
	// Reading STAR2 after STAR1 is required in order to acknowledge some of the
	// interrupt flags checked below.
	auto star1 = I2C_STAR1;
	auto star2 = I2C_STAR2;

	if (star1 & I2C_STAR1_RXNE) {
		uint8_t value = I2C_DATAR & 0xff;

		LOG("RXNE %02x", value);

		// Receive the next request byte (if any) and stop sending ACKs once the
		// buffer is full. Note that RXNE has to be handled before ADDR in order
		// to properly acknowledge start conditions issued during a write (see
		// below).
		// BUG: at speeds higher than ~10 kHz, neither RXNE nor ADDR seem to be
		// fired for the last byte in a read request if directly followed by a
		// start-repeated condition; the last byte is thus always missed. This
		// is worked around at the ESP32 side by padding read requests with a
		// dummy byte.
		if (requestOffset_ < I2C_MAX_REQUEST_LENGTH)
			request_[requestOffset_++] = value;

		if (requestOffset_ >= I2C_MAX_REQUEST_LENGTH)
			I2C_CTLR1 = (I2C_CTLR1 & ~I2C_CTLR1_ACK) | I2C_CTLR1_STOP;
	}

	if (star1 & I2C_STAR1_ADDR) {
		auto isRead = star2 & I2C_STAR2_TRA;

		LOG("ADDR (%c)", isRead ? 'r' : 'w');

		// If an I2C read start condition is detected during a write, interpret
		// the bytes received so far as a read command and invoke the callback
		// to prepare the response.
		if (isRead && (requestOffset_ > 0)) {
			responseOffset_ = 1;
			responseLength_ = readCallback
				? readCallback(request_, requestOffset_, response_)
				: 0;

			// Load the first two bytes of the response (the second byte will be
			// loaded by the if statement below as TXE will also be set).
			if (responseLength_ > 0)
				I2C_DATAR = response_[0];

			LOG("read: %d -> %d bytes", requestOffset_, responseLength_);
		}

		requestOffset_ = 0;
	}

	if (star1 & I2C_STAR1_TXE) {
		uint8_t value = 0;

		// This can only occur during a read. Send the next response byte (if
		// any).
		if (responseOffset_ < responseLength_)
			value = response_[responseOffset_++];

		LOG("TXE %02x", value);

		I2C_DATAR = value;
	}

	if (star1 & I2C_STAR1_STOPF) {
		LOG("STOPF");

		// Dispatch the write (if it wasn't followed by a read, which would have
		// already cleared the request buffer).
		if (requestOffset_ > 0) {
			if (writeCallback)
				writeCallback(request_, requestOffset_);

			LOG("write: %d bytes", requestOffset_);
		}

		// Re-enable automatic ACK sending after each byte received for the next
		// transaction.
		reset_();

		I2C_CTLR1 = (I2C_CTLR1 & ~I2C_CTLR1_STOP) | I2C_CTLR1_ACK;
	}
}

void I2CSlave::handleErrorInterrupt(void) {
	auto star1 = I2C_STAR1;
	auto star2 = I2C_STAR2;

	LOG("star1=%04x, star2=%04x", star1, star2);
	(void) star1;
	(void) star2;

	// Abort the current transaction, clear all error flags and re-enable
	// automatic ACK sending.
	reset_();

	I2C_STAR1 = 0;
	I2C_CTLR1 = (I2C_CTLR1 & ~I2C_CTLR1_STOP) | I2C_CTLR1_ACK;
}
