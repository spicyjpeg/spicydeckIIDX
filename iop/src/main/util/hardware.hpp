
#pragma once

#include <stdint.h>
#include "ch32v003/registers.h"
#include "main/util/bitfield.hpp"

namespace util {

/* Hardware utilities */

template<typename... A> static inline void enablePeripherals(A... flags) {
	uint32_t ahb  = 0;
	uint32_t apb1 = 0;
	uint32_t apb2 = 0;

	accumulateFlags3<
		RCCAHBPCENRFlag,
		RCCAPB1PCENRFlag,
		RCCAPB2PCENRFlag
	>(ahb, apb1, apb2, flags...);

	if (ahb)
		RCC_AHBPCENR  |= ahb;
	if (apb1)
		RCC_APB1PCENR |= apb1;
	if (apb2)
		RCC_APB2PCENR |= apb2;
}

template<typename... A> static inline void resetPeripherals(A... flags) {
	uint32_t apb1 = 0;
	uint32_t apb2 = 0;

	accumulateFlags2<
		RCCAPB1PRSTRFlag,
		RCCAPB2PRSTRFlag
	>(apb1, apb2, flags...);

	if (apb1) {
		auto prstr    = RCC_APB1PRSTR;
		RCC_APB1PRSTR = prstr |  apb1;
		RCC_APB1PRSTR = prstr & ~apb1;
	}
	if (apb2) {
		auto prstr    = RCC_APB2PRSTR;
		RCC_APB2PRSTR = prstr |  apb2;
		RCC_APB2PRSTR = prstr & ~apb2;
	}
}

static inline void enableIRQChannel(IRQChannel irq, bool enable) {
	if (enable)
		PFIC_IENR(irq / 32) = 1 << (irq % 32);
	else
		PFIC_IRER(irq / 32) = 1 << (irq % 32);
}

class OptionBytes {
public:
	uint8_t rdpr, user, data[2], wrpr[2];

	void read(void);
	bool compare(void) const;
	bool writeAndRestart(void) const;
};

[[noreturn]] void resetSystem(void);
void initClockHSI(void);
void initClockPLL(void);
void initSysTick(int hclk, int irqRate);

}
