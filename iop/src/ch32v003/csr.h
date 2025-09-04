/*
 * ps1-bare-metal - (C) 2023-2025 spicyjpeg
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#pragma once

#include <stdint.h>

#define DEF(type) static inline type __attribute__((always_inline))

/* CSR definitions */

typedef enum {
	CSR_MSTATUS   = 0x300, // Status register
	CSR_MISA      = 0x301, // Hardware instruction set register
	CSR_MTVEC     = 0x305, // Exception base address register
	CSR_MSCRATCH  = 0x340, // Machine mode staging register
	CSR_MEPC      = 0x341, // Exception program pointer register
	CSR_MCAUSE    = 0x342, // Exception cause register
	CSR_DCSR      = 0x7b0, // Debug control and status registers
	CSR_DPC       = 0x7b1, // Debug mode program pointer register
	CSR_DSCRATCH0 = 0x7b2, // Debug mode staging register 0
	CSR_DSCRATCH1 = 0x7b3, // Debug mode staging register 1
	CSR_DBGMCUCR  = 0x7c0, // Debug MCU configuration register
	CSR_INTSYSCR  = 0x804, // Interrupt system control register
	CSR_MARCHID   = 0xf12, // Architecture number register
	CSR_MIMPID    = 0xf13  // Hardware implementation numbering register
} CSRRegister;

typedef enum {
	CSR_MSTATUS_MIE            = 1 <<  3, // Machine mode interrupt enable
	CSR_MSTATUS_MPIE           = 1 <<  7, // Interrupt enable state before entering interrupt
	CSR_MSTATUS_MPP_BITMASK    = 3 << 11, // Privileged mode before entering break
	CSR_MSTATUS_MPP_USER       = 0 << 11,
	CSR_MSTATUS_MPP_SUPERVISOR = 1 << 11,
	CSR_MSTATUS_MPP_MACHINE    = 3 << 11,
	CSR_MSTATUS_MPOP           = 1 << 23, // Whether the current active interrupt needs to come out of the stack
	CSR_MSTATUS_MPPOP          = 1 << 24  // Whether the current subactive interrupt needs to come out of the stack
} CSRMStatusFlag;

typedef enum {
	CSR_MTVEC_MODE0_BITMASK    =        1 <<  0, // Interrupt or exception entry address mode selection.
	CSR_MTVEC_MODE0_SINGLE     =        0 <<  0, // Use of the uniform entry address.
	CSR_MTVEC_MODE0_VECTORED   =        1 <<  0, // Address offset based on interrupt number *4.
	CSR_MTVEC_MODE1_BITMASK    =        1 <<  1, // Interrupt vector table identifies patterns.
	CSR_MTVEC_MODE1_INLINE     =        0 <<  1, // Identification by jump instruction, limited range, support for non-jump instructions.
	CSR_MTVEC_MODE1_ADDRESS    =        1 <<  1, // Identify by absolute address, support full range, but must jump.
	CSR_MTVEC_BASEADDR_BITMASK = 0x3fffff << 10  // The interrupt vector table base address, which needs to be 1KB aligned.
} CSRMTVECFlag;

typedef enum {
	CSR_CAUSE_EXC_BITMASK = 255 <<  0, // Exception codes
	CSR_CAUSE_INT         =   1 << 31  // Interrupt indication field
} CSRMCauseFlag;

typedef enum {
	CSR_DCSR_PRV_BITMASK       =  3 <<  0, // Privilege mode
	CSR_DCSR_PRV_USER          =  0 <<  0,
	CSR_DCSR_PRV_SUPERVISOR    =  1 <<  0,
	CSR_DCSR_PRV_MACHINE       =  3 <<  0,
	CSR_DCSR_STEP              =  1 <<  2, // Enable single-step debugging
	CSR_DCSR_CAUSE_BITMASK     =  7 <<  6, // Reasons for entering debugging
	CSR_DCSR_CAUSE_EBREAK      =  1 <<  6, // Entering debugging in the form of ebreak command (priority 3)
	CSR_DCSR_CAUSE_TRIGGER     =  2 <<  6, // Entering debugging in the form of trigger module (priority 4, the highest)
	CSR_DCSR_CAUSE_PAUSE       =  3 <<  6, // Entering debugging in the form of pause request (priority 1)
	CSR_DCSR_CAUSE_STEP        =  4 <<  6, // Entering debugging in the form of single-step debugging (priority 0, the lowest)
	CSR_DCSR_CAUSE_RESET       =  5 <<  6, // Enter debug mode directly after microprocessor reset (priority 2)
	CSR_DCSR_STOPTIME          =  1 <<  9, // System timer stop in Debug mode
	CSR_DCSR_STEPIE            =  1 << 11, // Enable interrupts under single-step debugging
	CSR_DCSR_EBREAKU           =  1 << 12, // The ebreak command in user mode can enter debug mode
	CSR_DCSR_EBREAKM           =  1 << 15, // The ebreak command in machine mode can enter debug mode
	CSR_DCSR_XDEBUGVER_BITMASK = 15 << 28
} CSRDCSRFlag;

typedef enum {
	CSR_DBGMCUCR_SLEEP     = 1 <<  0, // In Sleep mode, both FCLK and HCLK clocks are provided by the originally configured system clock.
	CSR_DBGMCUCR_STANDBY   = 1 <<  2, // The digital circuitry section is not powered down, and the FCLK and HCLK clocks are clocked by the internal RL oscillator.
	CSR_DBGMCUCR_IWDG_STOP = 1 <<  8, // IWDG debug stop bit. The debug IWDG stops working when the core enters the debug state.
	CSR_DBGMCUCR_WWDG_STOP = 1 <<  9, // WWDG debug stop bit. The debug WWDG stops working when the core enters the debug state.
	CSR_DBGMCUCR_TIM1_STOP = 1 << 12, // Timer 1 debug stop bit. The counter stops when the core enters the debug state.
	CSR_DBGMCUCR_TIM2_STOP = 1 << 13  // Timer 2 debug stop bit. The counter stops when the core enters the debug state.
} CSRDBGMCUCRFlag;

typedef enum {
	CSR_INTSYSCR_HWSTKEN = 1 << 0, // HPE enable.
	CSR_INTSYSCR_INESTEN = 1 << 1, // Interrupt nesting enable.
	CSR_INTSYSCR_EABIEN  = 1 << 2  // EABI enable.
} CSRINTSYSCRFlag;

// Note that reg must be a constant value known at compile time, as the
// csrw/csrr instructions only support addressing coprocessor registers directly
// through immediates.
DEF(void) csr_setReg(const CSRRegister reg, uint32_t value) {
	__asm__ volatile(
		".option push\n"
		".option arch, +zicsr\n"
		"csrw %1, %0\n"
		".option pop\n"
		:: "r"(value), "i"(reg)
	);
}
DEF(uint32_t) csr_getReg(const CSRRegister reg) {
	uint32_t value;

	__asm__ volatile(
		".option push\n"
		".option arch, +zicsr\n"
		"csrr %0, %1\n"
		".option pop\n"
		: "=r"(value) : "i"(reg)
	);
	return value;
}

DEF(void) csr_enableInterrupts(void) {
	uint32_t status = csr_getReg(CSR_MSTATUS);

	csr_setReg(CSR_MSTATUS, status | CSR_MSTATUS_MIE | CSR_MSTATUS_MPIE);
}
DEF(uint32_t) csr_disableInterrupts(void) {
	uint32_t status = csr_getReg(CSR_MSTATUS);

	csr_setReg(CSR_MSTATUS, status & ~(CSR_MSTATUS_MIE | CSR_MSTATUS_MPIE));
	return status & (CSR_MSTATUS_MIE | CSR_MSTATUS_MPIE);
}

DEF(void) csr_setVectorJumpArea(const void *jumpArea) {
	csr_setReg(
		CSR_MTVEC,
		0
			| CSR_MTVEC_MODE0_VECTORED
			| CSR_MTVEC_MODE1_INLINE
			| (((uint32_t) jumpArea) & CSR_MTVEC_BASEADDR_BITMASK)
	);
}
DEF(void) csr_setVectorTable(const void *table) {
	csr_setReg(
		CSR_MTVEC,
		0
			| CSR_MTVEC_MODE0_VECTORED
			| CSR_MTVEC_MODE1_ADDRESS
			| (((uint32_t) table) & CSR_MTVEC_BASEADDR_BITMASK)
	);
}

#undef DEF
