
#include <stdint.h>
#include "ch32v003/registers.h"
#include "main/util/bitfield.hpp"
#include "main/util/hardware.hpp"
#include "main/util/templates.hpp"
#include "main/defs.hpp"
#include "main/input.hpp"

/* Analog inputs */

static constexpr int DMA_CHANNEL_ = 0;

static constexpr uint16_t CENTER_DEAD_ZONE_ = 16;

AnalogInputs analogInputs;

AnalogInputs::AnalogInputs(void) {
	util::clear(buffers_);
}

void AnalogInputs::init(void) {
	// Enable the ADC and configure it for software-triggered scan mode.
	util::enablePeripherals(
		RCC_AHBPCENR_DMA1EN,
		RCC_APB2PCENR_IOPAEN,
		RCC_APB2PCENR_IOPCEN,
		RCC_APB2PCENR_IOPDEN,
		RCC_APB2PCENR_ADC1EN
	);
	util::resetPeripherals(RCC_APB2PRSTR_ADC1RST);

	ADC_SAMPTR1 =
		util::repeatBitPattern<uint32_t>(ADC_SAMPTR_SMP_15, 3) & 0x3ffff;
	ADC_SAMPTR2 =
		util::repeatBitPattern<uint32_t>(ADC_SAMPTR_SMP_15, 3) & 0x3fffffff;
	ADC_RSQR1   = 0
		| (util::sequentialBitPattern<uint32_t>(12, 1, 5) & 0xfffff)
		| ((defs::NUM_ANALOG_INPUTS - 1) << 20);
	ADC_RSQR2   = util::sequentialBitPattern<uint32_t>(6, 1, 5) & 0x3fffffff;
	ADC_RSQR3   = util::sequentialBitPattern<uint32_t>(0, 1, 5) & 0x3fffffff;

	const uint32_t ctlr2 = 0
		| ADC_CTLR2_ADON
		| ADC_CTLR2_DMA
		| ADC_CTLR2_JEXTSEL_MANUAL
		| ADC_CTLR2_EXTSEL_MANUAL;

	ADC_CTLR1 = 0
		| ADC_CTLR1_SCAN
		| ADC_CTLR1_CALVOL_50;
	ADC_CTLR2 = ctlr2;

	// Route all analog inputs to the ADC.
	const auto mode = util::repeatBitPattern<uint32_t>(
		0
			| GPIO_CFGLR_MODE_INPUT
			| GPIO_CFGLR_CNF_IN_ANALOG,
		4
	);

	GPIOA_CFGLR = util::bitwiseTernary(
		util::repeatEachBit<uint32_t>(
			0
				| (1 << defs::PA_ADC_IN0)
				| (1 << defs::PA_ADC_IN1),
			4
		),
		mode,
		GPIOA_CFGLR
	);
	GPIOC_CFGLR = util::bitwiseTernary(
		util::repeatEachBit<uint32_t>(1 << defs::PC_ADC_IN4, 4),
		mode,
		GPIOC_CFGLR
	);
	GPIOD_CFGLR = util::bitwiseTernary(
		util::repeatEachBit<uint32_t>(
			0
				| (1 << defs::PD_ADC_IN3)
				| (1 << defs::PD_ADC_IN4)
				| (1 << defs::PD_ADC_IN5)
				| (1 << defs::PD_ADC_IN6)
				| (1 << defs::PD_ADC_IN7),
			4
		),
		mode,
		GPIOD_CFGLR
	);

	// Perform ADC calibration. This must be done at least a few cycles after
	// the ADC is first turned on.
	ADC_CTLR2 = ctlr2 | ADC_CTLR2_RSTCAL;

	while (ADC_CTLR2 & ADC_CTLR2_RSTCAL)
		__asm__ volatile("");

	ADC_CTLR2 = ctlr2 | ADC_CTLR2_CAL;

	while (ADC_CTLR2 & ADC_CTLR2_CAL)
		__asm__ volatile("");

	// Configure DMA channel 1 to write each ADC conversion result to the double
	// buffer in memory. Circular (endless) mode is used to avoid having to
	// reconfigure the channel after each poll.
	DMA_PADDR(DMA_CHANNEL_) = uint32_t(&ADC_RDATAR);
	DMA_MADDR(DMA_CHANNEL_) = uint32_t(buffers_);
	DMA_CNTR (DMA_CHANNEL_) = defs::NUM_ANALOG_INPUTS * 2;
	DMA_CFGR (DMA_CHANNEL_) = 0
		| DMA_CFGR_EN
		| DMA_CFGR_TCIE
		| DMA_CFGR_HTIE
		| DMA_CFGR_DIR_READ
		| DMA_CFGR_CIRC
		| DMA_CFGR_MINC
		| DMA_CFGR_PSIZE_32
		| DMA_CFGR_MSIZE_16
		| DMA_CFGR_PL_HIGH;
}

void AnalogInputs::getInputs(uint8_t *output) const {
	// Determine which buffer is not currently being overwritten by the DMA
	// channel.
	auto pending = DMA_CNTR(DMA_CHANNEL_);
	auto source  = ((pending >= 1) && (pending <= defs::NUM_ANALOG_INPUTS))
		? buffers_[1]
		: buffers_[0];

	for (size_t i = defs::NUM_ANALOG_INPUTS; i > 0; i--) {
		int value = source ? *(source++) : 512;

		if (
			(value >= (512 - CENTER_DEAD_ZONE_)) &&
			(value <= (512 + CENTER_DEAD_ZONE_))
		)
			value = 512;

		value      *= 255;
		*(output++) = (value + 511) / 1023;
	}
}

/* Button matrix */

static constexpr uint8_t MATRIX_ROWS_[NUM_MATRIX_ROWS]{
	1 << defs::PC_MATRIX_ROW0,
	1 << defs::PC_MATRIX_ROW1,
	1 << defs::PC_MATRIX_ROW2,
	1 << defs::PC_MATRIX_ROW3,
	1 << defs::PC_MATRIX_ROW4
};
static constexpr uint8_t MATRIX_COLUMNS_[NUM_MATRIX_COLUMNS]{
	1 << defs::PD_MATRIX_COL0,
	1 << defs::PD_MATRIX_COL1
};

static constexpr uint8_t MATRIX_ROW_MASK_    =
	util::reduceOR(MATRIX_ROWS_,    NUM_MATRIX_ROWS);
static constexpr uint8_t MATRIX_COLUMN_MASK_ =
	util::reduceOR(MATRIX_COLUMNS_, NUM_MATRIX_COLUMNS);

ButtonMatrix buttonMatrix;

ButtonMatrix::ButtonMatrix(void) {
	util::clear(states_);
}

void ButtonMatrix::init(void) {
	// Set up the row (port C) and column (port D) pins.
	util::enablePeripherals(
		RCC_APB2PCENR_IOPCEN,
		RCC_APB2PCENR_IOPDEN
	);

	GPIOC_CFGLR = util::bitwiseTernary(
		util::repeatEachBit<uint32_t>(MATRIX_ROW_MASK_, 4),
		util::repeatBitPattern<uint32_t>(
			0
				| GPIO_CFGLR_MODE_INPUT
				| GPIO_CFGLR_CNF_IN_PULL,
			4
		),
		GPIOC_CFGLR
	);
	GPIOD_CFGLR = util::bitwiseTernary(
		util::repeatEachBit<uint32_t>(MATRIX_COLUMN_MASK_, 4),
		util::repeatBitPattern<uint32_t>(
			0
				| GPIO_CFGLR_MODE_OUTPUT_2MHZ
				| GPIO_CFGLR_CNF_OUT_OPEN_DRAIN,
			4
		),
		GPIOD_CFGLR
	);

	// Enable the internal pullups on matrix columns and clear (pull high) all
	// rows.
	GPIOC_BSHR = MATRIX_ROW_MASK_;
	GPIOD_BSHR = MATRIX_COLUMN_MASK_;

	states_[0] = 0;
	states_[1] = 0;
	states_[2] = 0;
}

void ButtonMatrix::update(void) {
	defs::ButtonMask buttons = 0;
	defs::ButtonMask mask    = 1;

	GPIOD_BSHR = MATRIX_COLUMN_MASK_;

	for (int i = 0; i < NUM_MATRIX_COLUMNS; i++) {
		GPIOC_BCR = MATRIX_COLUMNS_[i];
		auto rows = GPIOD_INDR;

		for (int j = 0; j < NUM_MATRIX_ROWS; j++, mask <<= 1) {
			if (rows & MATRIX_ROWS_[j])
				buttons |= mask;
		}

		GPIOC_BSHR = MATRIX_COLUMNS_[i];
	}

	states_[2] = states_[1];
	states_[1] = states_[0];
	states_[0] = buttons;
}
