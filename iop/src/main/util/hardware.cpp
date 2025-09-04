
#include <stdint.h>
#include "ch32v003/registers.h"
#include "main/util/hardware.hpp"
#include "main/defs.hpp"

namespace util {

/* Hardware utilities */

static constexpr int FLASH_TIMEOUT_ = 100000;

static bool waitForFlashIdle_(void) {
	uint32_t statr;

	for (int i = FLASH_TIMEOUT_; i > 0; i--) {
		statr = FLASH_STATR;

		if (statr & FLASH_STATR_BUSY)
			continue;

		FLASH_STATR = statr; // Clear EOP and WRPRTERR

		if (statr & FLASH_STATR_WRPRTERR) {
			LOG("write protect error, statr=%08x", statr);
			return false;
		}
		if (!(statr & FLASH_STATR_EOP)) {
			LOG("EOP not set, statr=%08x", statr);
			return false;
		}

		return true;
	}

	LOG("timeout, statr=%08x", statr);
	return false;
}

void OptionBytes::read(void) {
	auto source = reinterpret_cast<const uint16_t *>(FLASH_OPTION_BASE);
	auto dest   = reinterpret_cast<uint8_t        *>(this);

	for (size_t i = sizeof(OptionBytes); i > 0; i--)
		*(dest++) = *(source++) & 0xff;
}

bool OptionBytes::compare(void) const {
	auto source = reinterpret_cast<const uint16_t *>(FLASH_OPTION_BASE);
	auto dest   = reinterpret_cast<const uint8_t  *>(this);

	for (size_t i = sizeof(OptionBytes); i > 0; i--) {
		if (*(dest++) != (*(source++) & 0xff))
			return false;
	}

	return true;
}

bool OptionBytes::writeAndRestart(void) const {
	if (compare())
		return true;

	auto ctlr = FLASH_CTLR;

	if (ctlr & FLASH_CTLR_LOCK) {
		FLASH_KEYR = FLASH_KEY1;
		FLASH_KEYR = FLASH_KEY2;
	}
	if (!(ctlr & FLASH_CTLR_OBWRE)) {
		FLASH_OBKEYR = FLASH_KEY1;
		FLASH_OBKEYR = FLASH_KEY2;
	}

	// Begin by erasing the entire option byte region.
	FLASH_CTLR = FLASH_CTLR_OBER | FLASH_CTLR_OBWRE;
	FLASH_CTLR = 0
		| FLASH_CTLR_OBER
		| FLASH_CTLR_STRT
		| FLASH_CTLR_OBWRE;

	if (!waitForFlashIdle_()) {
		FLASH_CTLR = FLASH_CTLR_LOCK;
		return false;
	}

	// Proceed to write it back one byte at a time.
	FLASH_CTLR = FLASH_CTLR_OBPG | FLASH_CTLR_OBWRE;

	auto source = &rdpr;
	auto dest   = reinterpret_cast<uint16_t *>(FLASH_OPTION_BASE);

	for (size_t i = sizeof(OptionBytes); i > 0; i--) {
		FLASH_CTLR = 0
			| FLASH_CTLR_OBPG
			| FLASH_CTLR_STRT
			| FLASH_CTLR_OBWRE;

#if 0
		auto value = *(source++);
		*(dest++)  = value | ((value ^ 0xff) << 8);
#else
		*(dest++) = *(source++);
#endif

		if (!waitForFlashIdle_()) {
			LOG("write error at %08x", dest);
			FLASH_CTLR = FLASH_CTLR_LOCK;
			return false;
		}
	}

	// Perform a system reset in order for the new settings to apply.
	LOG("write successful, rebooting");
	FLASH_CTLR = FLASH_CTLR_LOCK;
	resetSystem();
}

[[noreturn]] void resetSystem(void) {
	PFIC_CFGR = PFIC_CFGR_KEYCODE_KEY1;
	PFIC_CFGR = PFIC_CFGR_KEYCODE_KEY2;
	PFIC_CFGR = PFIC_CFGR_KEYCODE_KEY3 | PFIC_CFGR_RESETSYS;

	__builtin_unreachable();
}

void initClockHSI(void) {
	// Add one waitstate to flash access (as per the manual's recommendation for
	// higher clocks).
	FLASH_ACTLR = FLASH_ACTLR_LATENCY_1;

	// Run the core and peripherals at 24 MHz and the ADC at 12 MHz.
	RCC_CFGR0 = 0
		| RCC_CFGR0_SW_HSI
		| RCC_CFGR0_HPRE_DIV1
		| RCC_CFGR0_ADCPRE_DIV2
		| RCC_CFGR0_PLLSRC_HSI
		| RCC_CFGR0_MCO_NONE;

	while ((RCC_CFGR0 & RCC_CFGR0_SWS_BITMASK) != RCC_CFGR0_SWS_HSI)
		__asm__ volatile("");
}

void initClockPLL(void) {
	FLASH_ACTLR = FLASH_ACTLR_LATENCY_1;

	// Start the PLL and wait for it to stabilize, then switch over to it.
	RCC_CFGR0 = (RCC_CFGR0 & ~RCC_CFGR0_PLLSRC_BITMASK) | RCC_CFGR0_PLLSRC_HSI;
	RCC_CTLR |= RCC_CTLR_PLLON;

	while (!(RCC_CTLR & RCC_CTLR_PLLRDY))
		__asm__ volatile("");

	// Run the core and peripherals at 48 MHz and the ADC at 24 MHz.
	RCC_CFGR0 = 0
		| RCC_CFGR0_SW_PLL
		| RCC_CFGR0_HPRE_DIV1
		| RCC_CFGR0_ADCPRE_DIV2
		| RCC_CFGR0_PLLSRC_HSI
		| RCC_CFGR0_MCO_NONE;

	while ((RCC_CFGR0 & RCC_CFGR0_SWS_BITMASK) != RCC_CFGR0_SWS_PLL)
		__asm__ volatile("");
}

void initSysTick(int hclk, int irqRate) {
	STK_CTLR  = 0;
	STK_CNTL  = 0;
	STK_CMPLR = (hclk + irqRate / 2) / irqRate;
	STK_CTLR  = 0
		| STK_CTLR_STE
		| STK_CTLR_STIE
		| STK_CTLR_STCLK_DIV8
		| STK_CTLR_STRE;
}

}
