
#include <cstddef>
#include <stdint.h>
#include <stdio.h>
#include "ch32v003/csr.h"
#include "ch32v003/registers.h"
#include "main/util/hardware.hpp"
#include "main/defs.hpp"
#include "main/i2c.hpp"
#include "main/input.hpp"

/* Interrupt handlers */

extern "C" [[gnu::interrupt]] void handleI2CEventInterrupt(void) {
	i2cSlave.handleEventInterrupt();
}

extern "C" [[gnu::interrupt]] void handleI2CErrorInterrupt(void) {
	i2cSlave.handleErrorInterrupt();
}

/* I2C command handler */

static size_t handleI2CRead_(
	const uint8_t *request,
	size_t        requestLength,
	uint8_t       *response
) {
	auto inputs  = reinterpret_cast<defs::IOPInputState *>(response);
	auto version = reinterpret_cast<char *>(response);

	switch (request[0]) {
		case defs::IOP_CMD_GET_LAST_INPUTS:
			inputs->buttons = buttonMatrix.getButtons();
			analogInputs.getInputs(inputs->analog);

			return sizeof(defs::IOPInputState);

		case defs::IOP_CMD_POLL_INPUTS:
			inputs->buttons = buttonMatrix.getButtons();
			analogInputs.getInputs(inputs->analog);

			buttonMatrix.update(); // Blocking (but fast)
			analogInputs.update(); // Non-blocking

			return sizeof(defs::IOPInputState);

		case defs::IOP_CMD_GET_VERSION:
			__builtin_strncpy(version, VERSION_STRING, I2C_MAX_RESPONSE_LENGTH);

			return I2C_MAX_RESPONSE_LENGTH;

		default:
			return 0;
	}
}

static void handleI2CWrite_(const uint8_t *request, size_t requestLength) {
	switch (request[0]) {
		case defs::IOP_CMD_POLL_INPUTS:
			buttonMatrix.update();
			analogInputs.update();
	}
}

/* Main */

extern const uint8_t _vectorJumpArea[];

int main(int argc, const char **argv) {
	csr_setVectorJumpArea(_vectorJumpArea);
	util::initClockHSI();

	i2cSlave.init(F_CPU_HSI, defs::IOP_I2C_ADDRESS);
	analogInputs.init();
	buttonMatrix.init();

#ifdef ENABLE_LOGGING
	// The serial port must be initialized after all other GPIO pins in order to
	// override them.
	initSerialIO(F_CPU_HSI, 115200);
	routeSerialIO(true, false);

	LOG(
		"spicydeckIIDX IOP firmware " VERSION_STRING
		" (" __DATE__ " " __TIME__ ")"
	);
	LOG("(C) 2025 spicyjpeg");
#endif

	i2cSlave.readCallback  = handleI2CRead_;
	i2cSlave.writeCallback = handleI2CWrite_;

	// Ensure the reset input is disabled by reflashing the option bytes if
	// needed.
	util::OptionBytes option;

	option.read();
	option.user = 0
		| OPTION_USER_IWDG_SW
		| OPTION_USER_STANDBY_RST
		| OPTION_USER_RST_MODE_OFF
		| OPTION_USER_START_MODE_BOOT
		| OPTION_USER_RESERVED;
	option.writeAndRestart();

	// Configure and enable interrupts then enter sleep mode indefinitely (only
	// interrupt handlers will be executed).
	util::enableIRQChannel(IRQ_I2C1_EV, true);
	util::enableIRQChannel(IRQ_I2C1_ER, true);
	csr_enableInterrupts();

	for (;;)
		__asm__ volatile("wfi\n");
}
