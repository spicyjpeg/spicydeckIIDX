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

#define _ADDR8(addr)  ((volatile uint8_t *) (addr))
#define _ADDR16(addr) ((volatile uint16_t *) (addr))
#define _ADDR32(addr) ((volatile uint32_t *) (addr))
#define _MMIO8(addr)  (*_ADDR8(addr))
#define _MMIO16(addr) (*_ADDR16(addr))
#define _MMIO32(addr) (*_ADDR32(addr))

/* Constants */

#define F_CPU_LSI   128000
#define F_CPU_HSI 24000000
#define F_CPU_PLL 48000000

typedef enum {
	FLASH_USER_BASE   = 0x08000000,
	FLASH_BOOT_BASE   = 0x1ffff000,
	FLASH_VENDOR_BASE = 0x1ffff7c0,
	FLASH_OPTION_BASE = 0x1ffff800,
	SRAM_BASE         = 0x20000000,
	TIM2_BASE         = 0x40000000,
	WWDG_BASE         = 0x40002c00,
	IWDG_BASE         = 0x40003000,
	I2C_BASE          = 0x40005400,
	PWR_BASE          = 0x40007000,
	AFIO_BASE         = 0x40010000,
	EXTI_BASE         = 0x40010400,
	GPIOA_BASE        = 0x40010800,
	GPIOC_BASE        = 0x40011000,
	GPIOD_BASE        = 0x40011400,
	ADC_BASE          = 0x40012400,
	TIM1_BASE         = 0x40012c00,
	SPI_BASE          = 0x40013000,
	USART_BASE        = 0x40013800,
	DMA_BASE          = 0x40020000,
	RCC_BASE          = 0x40021000,
	FLASH_IF_BASE     = 0x40022000,
	EXTEND_BASE       = 0x40023800,
	PFIC_BASE         = 0xe000e000,
	SYSTICK_BASE      = 0xe000f000
} BaseAddress;

/* Flash option bytes */

typedef enum {
	OPTION_RDPR_LOCK   = 0x0000,
	OPTION_RDPR_UNLOCK = 0x5aa5
} OptionRDPRKey;

typedef enum {
	OPTION_USER_IWDG_SW              = 1 << 0, // Independent Watchdog (IWDG) hardware enable configuration.
	OPTION_USER_STANDBY_RST          = 1 << 2, // System reset control in Standby mode.
	OPTION_USER_RST_MODE_BITMASK     = 3 << 3, // PD7 multiplexed as external pin reset.
	OPTION_USER_RST_MODE_DELAY_128   = 0 << 3, // Ignoring pin states within 128us after turning on the multiplexing function.
	OPTION_USER_RST_MODE_DELAY_1000  = 1 << 3, // Ignoring pin states within 1ms after turning on the multiplexing function.
	OPTION_USER_RST_MODE_DELAY_12000 = 2 << 3, // Ignoring pin states within 12ms after turning on the multiplexing function.
	OPTION_USER_RST_MODE_OFF         = 3 << 3, // Multiplexing function off, PD7 for I/O function.
	OPTION_USER_START_MODE_BITMASK   = 1 << 5, // Power-on startup mode
	OPTION_USER_START_MODE_USER      = 0 << 5, // Boot from user area
	OPTION_USER_START_MODE_BOOT      = 1 << 5, // Boot from BOOT area
	OPTION_USER_RESERVED             = (1 << 1) | (3 << 6) // Reserved (must be 1)
} OptionUSERFlag;

#define OPTION_RDPR  _MMIO16(FLASH_OPTION_BASE | 0x0)
#define OPTION_USER  _MMIO16(FLASH_OPTION_BASE | 0x2)
#define OPTION_DATA0 _MMIO16(FLASH_OPTION_BASE | 0x4)
#define OPTION_DATA1 _MMIO16(FLASH_OPTION_BASE | 0x6)
#define OPTION_WRPR0 _MMIO16(FLASH_OPTION_BASE | 0x8)
#define OPTION_WRPR1 _MMIO16(FLASH_OPTION_BASE | 0xa)

/* Timer 2 */

// TODO: add register bit definitions

#define TIM2_CTLR1     _MMIO16(TIM2_BASE | 0x00) // TIM2 control register1
#define TIM2_CTLR2     _MMIO16(TIM2_BASE | 0x04) // TIM2 control register2
#define TIM2_SMCFGR    _MMIO16(TIM2_BASE | 0x08) // TIM2 Slave mode control register
#define TIM2_DMAINTENR _MMIO16(TIM2_BASE | 0x0c) // TIM2 DMA/interrupt enable register
#define TIM2_INTFR     _MMIO16(TIM2_BASE | 0x10) // TIM2 interrupt status register
#define TIM2_SWEVGR    _MMIO16(TIM2_BASE | 0x14) // TIM2 event generation register
#define TIM2_CHCTLR1   _MMIO16(TIM2_BASE | 0x18) // TIM2 compare/capture control register1
#define TIM2_CHCTLR2   _MMIO16(TIM2_BASE | 0x1c) // TIM2 compare/capture control register2
#define TIM2_CCER      _MMIO16(TIM2_BASE | 0x20) // TIM2 compare/capture enable register
#define TIM2_CNT       _MMIO16(TIM2_BASE | 0x24) // TIM2 counter
#define TIM2_PSC       _MMIO16(TIM2_BASE | 0x28) // TIM2 count clock prescaler
#define TIM2_ATRLR     _MMIO16(TIM2_BASE | 0x2c) // TIM2 auto-reload register
#define TIM2_CHCVR(N)  _MMIO32((TIM2_BASE | 0x34) + (4 * (N))) // TIM2 compare/capture register
#define TIM2_DMACFGR   _MMIO16(TIM2_BASE | 0x48) // TIM2 DMA control register
#define TIM2_DMAADR    _MMIO16(TIM2_BASE | 0x4c) // TIM2 DMA address register in continuous mode

/* Window watchdog */

typedef enum {
	WWDG_CTLR_T_BITMASK = 127 << 0, // The 7-bit self-decrement counter decrements by 1 every 4096*2WDGTB HCLK cycles. A watchdog reset is generated when the counter decrements from 0x40 to 0x3F, i.e., when T6 jumps to 0.
	WWDG_CTLR_WDGA      =   1 << 7  // Window watchdog reset enable bit.
} WWDGCTLRFlag;

typedef enum {
	WWDG_CFGR_W_BITMASK     = 127 << 0, // Window watchdog 7-bit window value. Used to compare with the counter value.
	WWDG_CFGR_WDGTB_BITMASK =   3 << 7, // Window watchdog clock division selection.
	WWDG_CFGR_WDGTB_DIV1    =   0 << 7, // Divided by 1, counting time base = HCLK/4096.
	WWDG_CFGR_WDGTB_DIV2    =   1 << 7, // Divided by 2, counting time base = HCLK/4096/2.
	WWDG_CFGR_WDGTB_DIV4    =   2 << 7, // Divided by 4, counting time base = HCLK/4096/4.
	WWDG_CFGR_WDGTB_DIV8    =   3 << 7, // Divided by 8, counting time base = HCLK/4096/8.
	WWDG_CFGR_EWI           =   1 << 9  // Early wakeup interrupt enable bit. If this position is 1, an interrupt is generated when the counter value reaches 0x40. This bit can only be invited to 0 by hardware after a reset.
} WWDGCFGRFlag;

typedef enum {
	WWDG_STATR_EWIF = 1 << 0 // Wake up the interrupt flag bit early. When the counter reaches 0x40, this bit is set in hardware and must be cleared to 0 by software; the user setting is invalid.
} WWDGSTATRFlag;

#define WWDG_CTLR  _MMIO16(WWDG_BASE | 0x0) // Control register
#define WWDG_CFGR  _MMIO16(WWDG_BASE | 0x4) // Configuration Register
#define WWDG_STATR _MMIO16(WWDG_BASE | 0x8) // Status Register

/* Independent watchdog */

typedef enum {
	IWDG_KEY_UNLOCK = 0x5555, // Allows modification of the R16_IWDG_PSCR and R16_IWDG_RLDR registers.
	IWDG_KEY_CLEAR  = 0xaaaa, // Feed the dog. Loading of the IWDG_RLDR register value into the independent watchdog counter.
	IWDG_KEY_START  = 0xcccc  // Start the watchdog, but not if the hardware watchdog is enabled (user-option bytes configuration).
} IWDGKey;

typedef enum {
	IWDG_PSCR_PR_BITMASK = 7 << 0, // IWDG clock division factor, write 0x5555 to KEY before modifying this field.
	IWDG_PSCR_PR_DIV4    = 0 << 0, // Divided by 4.
	IWDG_PSCR_PR_DIV8    = 1 << 0, // Divided by 8.
	IWDG_PSCR_PR_DIV16   = 2 << 0, // Divided by 16.
	IWDG_PSCR_PR_DIV32   = 3 << 0, // Divided by 32.
	IWDG_PSCR_PR_DIV64   = 4 << 0, // Divided by 64.
	IWDG_PSCR_PR_DIV128  = 5 << 0, // Divided by 128.
	IWDG_PSCR_PR_DIV256  = 6 << 0  // Divided by 256.
} IWDGPSCRFlag;

typedef enum {
	IWDG_STATR_PVU = 1 << 0, // Clock division factor update flag bit. Hardware set or clear 0.
	IWDG_STATR_RVU = 1 << 1  // Reload value update flag bit. Hardware set or clear 0.
} IWDGSTATRFlag;

#define IWDG_CTLR  _MMIO16(IWDG_BASE | 0x0) // Control register
#define IWDG_PSCR  _MMIO16(IWDG_BASE | 0x4) // Prescaler register
#define IWDG_RLDR  _MMIO16(IWDG_BASE | 0x8) // Reload register
#define IWDG_STATR _MMIO16(IWDG_BASE | 0xc) // Status register

/* I2C */

typedef enum {
	I2C_CTLR1_PE        = 1 <<  0, // I2C peripheral enable bit.
	I2C_CTLR1_ENPEC     = 1 <<  5, // PEC enable bit, set this bit to enable PEC calculation.
	I2C_CTLR1_ENGC      = 1 <<  6, // General call enable bit. Set this bit to enable broadcast call and answer broadcast address 00h.
	I2C_CTLR1_NOSTRETCH = 1 <<  7, // Clock stretching disable bit. This bit is used to disable clock stretching in slave mode when ADDR or BTF flag is set, until it is reset by software.
	I2C_CTLR1_START     = 1 <<  8, // Start generation. This bit is set and cleared by software and cleared by hardware when start is sent or PE=0.
	I2C_CTLR1_STOP      = 1 <<  9, // Stop generation bit. This bit is set and cleared by software, cleared by hardware when a Stop condition is detected, set by hardware when a timeout error is detected.
	I2C_CTLR1_ACK       = 1 << 10, // Acknowledge enable, This bit is set and cleared by software and cleared by hardware when PE=0.
	I2C_CTLR1_POS       = 1 << 11, // ACK and PEC position setting bits, which can be set or cleared by user code and can be cleared by hardware after the PE has been cleared.
	I2C_CTLR1_PEC       = 1 << 12, // Packet error checking bit, set this bit to enable packet error detection. The user code can set or clear this bit.
	I2C_CTLR1_SWRST     = 1 << 15  // Software reset, setting this bit by user code will reset the I2C peripheral. Make sure the pins of the I2C bus are released and the bus is idle before the reset.
} I2CCTLR1Flag;

typedef enum {
	I2C_CTLR2_FREQ_BITMASK = 63 <<  0, // The I2C module clock frequency field, which must be entered at the correct clock frequency to produce the correct timing, allows a range between 8-48MHz.
	I2C_CTLR2_ITERREN      =  1 <<  8, // Error interrupt enable bit. Set to allow error interrupts.
	I2C_CTLR2_ITEVTEN      =  1 <<  9, // Event interrupt enable bit. Set this bit to enable event interrupt.
	I2C_CTLR2_ITBUFEN      =  1 << 10, // Buffer interrupt enable bit.
	I2C_CTLR2_DMAEN        =  1 << 11, // DMA requests enable bit. Set this bit to allow DMA request when TxE or RxEN is set.
	I2C_CTLR2_LAST         =  1 << 12  // DMA last transfer bit.
} I2CCTLR2Flag;

typedef enum {
	I2C_OADDR1_ADD7_BITMASK    =  127 <<  1, // Interface address, bits 7-1.
	I2C_OADDR1_ADD10_BITMASK   = 1023 <<  0, // Interface address, bits 9-0 when using a 10-bit address, ignored when using a 7-bit address.
	I2C_OADDR1_ADDMODE_BITMASK =    1 << 15, // Address mode.
	I2C_OADDR1_ADDMODE_7       =    0 << 15, // 7-bit slave address (does not respond to 10-bit address).
	I2C_OADDR1_ADDMODE_10      =    1 << 15  // 10-bit slave address (does not respond to 7-bit addresses).
} I2COADDR1Flag;

typedef enum {
	I2C_OADDR2_ENDUAL       =   1 << 0, // Dual address mode enable bit, set this bit to allow ADD2 to be recognized as well.
	I2C_OADDR2_ADD2_BITMASK = 127 << 1  // Interface address, bits 7-1 of the address in dual address mode.
} I2COADDR2Flag;

typedef enum {
	I2C_STAR1_SB     = 1 <<  0, // Start bit. Cleared by software by reading the SR1 register followed by writing the DR register, or by hardware when PE=0
	I2C_STAR1_ADDR   = 1 <<  1, // Address sent/matched bit.This bit is cleared by software reading SR1 register followed reading SR2, or by hardware when PE=0.
	I2C_STAR1_BTF    = 1 <<  2, // Byte transfer finished bit. Cleared by software reading SR1 followed by either a read or write in the DR register or by hardware after a start or a stop condition in transmission or when PE=0.
	I2C_STAR1_ADD10  = 1 <<  3, // 10-bit header sent bit. Cleared by software reading the SR1 register followed by a write in the DR register of the second address byte, or by hardware when PE=0.
	I2C_STAR1_STOPF  = 1 <<  4, // Stop detection bit. Cleared by software reading the SR1 register followed by a write in the CR1 RO register, or by hardware when PE=0
	I2C_STAR1_RXNE   = 1 <<  6, // Data register not empty bit. Cleared by software reading or writing the DR register or by hardware RO when PE=0.
	I2C_STAR1_TXE    = 1 <<  7, // Data register empty bit.Cleared by software writing to the DR register or by hardware after a RO start or a stop condition or when PE=0.
	I2C_STAR1_BERR   = 1 <<  8, // The bus error flag bit, which can be reset by a userwrite of 0, or by hardware when PE goes low.
	I2C_STAR1_ARLO   = 1 <<  9, // Arbitration lost bit.Cleared by software writing 0, or by hardware when PE=0.
	I2C_STAR1_AF     = 1 << 10, // Acknowledge failure bit.Cleared by software writing 0, or by hardware when PE=0.
	I2C_STAR1_OVR    = 1 << 11, // Overrun and underrun flag bits.
	I2C_STAR1_PECERR = 1 << 12  // The PEC error flag bit occurs on reception, and this bit can be reset by a user write of 0 or by hardware when PE goes low.
} I2CSTAR1Flag;

typedef enum {
	I2C_STAR2_MSL         =   1 << 0, // Master/slave bit. Set by hardware as soon as the interface is in Master mode (SB=1).
	I2C_STAR2_BUSY        =   1 << 1, // Bus busy bit. Cleared by hardware on detection of a Stop condition. This information is still updated when the interface is disabled (PE=0).
	I2C_STAR2_TRA         =   1 << 2, // Transmitter/receiver bit. It is cleared by hardware after detection of Stop condition (STOPF=1), repeated Start condition, loss of bus arbitration (ARLO=1), or when PE=0.
	I2C_STAR2_GENCALL     =   1 << 4, // General call address bit. Cleared by hardware after a Stop condition or repeated Start condition, or when PE=0.
	I2C_STAR2_DUALF       =   1 << 7, // Dual flag. Cleared by hardware after a Stop condition or repeated Start condition, or when PE=0.
	I2C_STAR2_PEC_BITMASK = 255 << 8  // Packet error checking bit. When PEC is enabled (ENPEC is set), this field holds the value of PEC.
} I2CSTAR2Flag;

typedef enum {
	I2C_CKCFGR_CCR_BITMASK = 4095 <<  0, // Clock control register in Fm/Sm mode
	I2C_CKCFGR_DUTY        =    1 << 14, // Duty cycle in the fast mode
	I2C_CKCFGR_FS          =    1 << 15  // Master mode selection bit.
} I2CCKCFGRFlag;

#define I2C_CTLR1  _MMIO16(I2C_BASE | 0x00) // I2C control register 1
#define I2C_CTLR2  _MMIO16(I2C_BASE | 0x04) // I2C control register 2
#define I2C_OADDR1 _MMIO16(I2C_BASE | 0x08) // I2C address register 1
#define I2C_OADDR2 _MMIO16(I2C_BASE | 0x0c) // I2C address register 2
#define I2C_DATAR  _MMIO16(I2C_BASE | 0x10) // I2C data register
#define I2C_STAR1  _MMIO16(I2C_BASE | 0x14) // I2C status register 1
#define I2C_STAR2  _MMIO16(I2C_BASE | 0x18) // I2C status register 2
#define I2C_CKCFGR _MMIO16(I2C_BASE | 0x1c) // I2C clock register

/* Power control */

typedef enum {
	PWR_CTLR_PDDS          = 1 << 1, // Standby/Sleep mode selection bit in power-down deep sleep scenario.
	PWR_CTLR_PVDE          = 1 << 4, // Power supply voltage monitoring function enable flag bit
	PWR_CTLR_PLS_BITMASK   = 7 << 5, // PVD voltage monitoring threshold setting.
	PWR_CTLR_PLS_2700_2850 = 0 << 5, // 2.85V rising edge/2.7V falling edge.
	PWR_CTLR_PLS_2900_3050 = 1 << 5, // 3.05V rising edge/2.9V falling edge.
	PWR_CTLR_PLS_3150_3300 = 2 << 5, // 3.3V rising edge/3.15V falling edge.
	PWR_CTLR_PLS_3300_3500 = 3 << 5, // 3.5V rising edge/3.3V falling edge.
	PWR_CTLR_PLS_3500_3700 = 4 << 5, // 3.7V rising edge/3.5V falling edge.
	PWR_CTLR_PLS_3700_3900 = 5 << 5, // 3.9V rising edge/3.7V falling edge.
	PWR_CTLR_PLS_3900_4100 = 6 << 5, // 4.1V rising edge/3.9V falling edge.
	PWR_CTLR_PLS_4200_4400 = 7 << 5  // 4.4V rising edge/4.2V falling edge.
} PWRCTLRFlag;

typedef enum {
	PWR_CSR_PVD0 = 1 << 2 // PVD output status flag bit. This bit is valid when PVDE=1 of PWR_CTLR register.
} PWRCSRFlag;

typedef enum {
	PWR_AWUCSR_AWUEN = 1 << 1 // Enable Automatic wake-up
} PWRAWUCSRFlag;

typedef enum {
	PWR_AWUPSC_BITMASK  = 15 << 0, // Counting time base
	PWR_AWUPSC_DIV1     =  1 << 0, // Prescaler off.
	PWR_AWUPSC_DIV2     =  2 << 0, // Divided by 2.
	PWR_AWUPSC_DIV4     =  3 << 0, // Divided by 4.
	PWR_AWUPSC_DIV8     =  4 << 0, // Divided by 8.
	PWR_AWUPSC_DIV16    =  5 << 0, // Divided by 16.
	PWR_AWUPSC_DIV32    =  6 << 0, // Divided by 32.
	PWR_AWUPSC_DIV64    =  7 << 0, // Divided by 64.
	PWR_AWUPSC_DIV128   =  8 << 0, // Divided by 128.
	PWR_AWUPSC_DIV256   =  9 << 0, // Divided by 256.
	PWR_AWUPSC_DIV512   = 10 << 0, // Divided by 512.
	PWR_AWUPSC_DIV1024  = 11 << 0, // Divided by 1024.
	PWR_AWUPSC_DIV2048  = 12 << 0, // Divided by 2048.
	PWR_AWUPSC_DIV4096  = 13 << 0, // Divided by 4096.
	PWR_AWUPSC_DIV10240 = 14 << 0, // Divided by 10240.
	PWR_AWUPSC_DIV61440 = 15 << 0  // Divided by 61440.
} PWRAWUPSCFlag;

#define PWR_CTLR   _MMIO32(PWR_BASE | 0x00) // Power control register
#define PWR_CSR    _MMIO32(PWR_BASE | 0x04) // Power control/status register
#define PWR_AWUCSR _MMIO32(PWR_BASE | 0x08) // Auto-wakeup control/status register
#define PWR_AWUWR  _MMIO32(PWR_BASE | 0x0c) // Auto-wakeup window comparison value register
#define PWR_AWUPSC _MMIO32(PWR_BASE | 0x10) // Auto-wakeup crossover factor register

/* Alternate-function I/O */

typedef enum {
	AFIO_PCFR1_SPI1_RM_BITMASK   = 1 <<  0, // Remapping of SPI1
	AFIO_PCFR1_SPI1_RM_DEFAULT   = 0 <<  0, // Default mapping (NSS/PC1, CK/PC5, MISO/PC7, MOSI/PC6).
	AFIO_PCFR1_SPI1_RM_ALT       = 1 <<  0, // Remapping (NSS/PC0, CK/PC5, MISO/PC7, MOSI/PC6).
	AFIO_PCFR1_I2C1_RM_BITMASK   = (1 << 1) | (1 << 22), // I2C1 remapping
	AFIO_PCFR1_I2C1_RM_DEFAULT   = (0 << 1) | (0 << 22), // Default mapping (SCL/PC2, SDA/PC1).
	AFIO_PCFR1_I2C1_RM_ALT1      = (1 << 1) | (0 << 22), // Remapping (SCL/ PD1, SDA/ PD0).
	AFIO_PCFR1_I2C1_RM_ALT2      = (0 << 1) | (1 << 22), // Remapping (SCL/PC5, SDA/PC6).
	AFIO_PCFR1_USART1_RM_BITMASK = (1 << 2) | (1 << 21), // USART1 mapping configuration
	AFIO_PCFR1_USART1_RM_DEFAULT = (0 << 2) | (0 << 21), // Default mapping (CK/PD4, TX/PD5, RX/PD6, CTS/PD3, RTS/PC2).
	AFIO_PCFR1_USART1_RM_ALT1    = (1 << 2) | (0 << 21), // Remapping (CK/PD7, TX/PD0, RX/PD1, CTS/PC3, RTS/PC2, SW_RX/PD0).
	AFIO_PCFR1_USART1_RM_ALT2    = (0 << 2) | (1 << 21), // Remapping (CK/PD7, TX/PD6, RX/PD5, CTS/PC6, RTS/PC7, SW_RX/PD6).
	AFIO_PCFR1_USART1_RM_ALT3    = (1 << 2) | (1 << 21), // Remapping (CK/PC5, TX/PC0, RX/PC1, CTS/PC6, RTS/PC7, SW_RX/PC0).
	AFIO_PCFR1_TIM1_RM_BITMASK   = 3 <<  6, // Remap bits for timer 1. It controls the mapping of channels 1 to 4, 1N to 3N, external trigger (ETR) and brake input (BKIN) of timer 1 to the GPIO ports.
	AFIO_PCFR1_TIM1_RM_DEFAULT   = 0 <<  6, // Default mapping (ETR/PC5, CH1/PD2, CH2/PA1, CH3/PC3, CH4/PC4, BKIN/PC2, CH1N/PD0, CH2N/PA2, CH3N/PD1).
	AFIO_PCFR1_TIM1_RM_ALT1      = 1 <<  6, // Partial mapping (ETR/PC5, CH1/PC6, CH2/PC7, CH3/PC0, CH4/PD3, BKIN/PC1, CH1N/PC3, CH2N/PC4, CH3N/PD1).
	AFIO_PCFR1_TIM1_RM_ALT2      = 2 <<  6, // Partial mapping (ETR/PD4, CH1/PD2, CH2/PA1, CH3/PC3, CH4/PC4, BKIN/PC2, CH1N/PD0, CH2N/PA2, CH3N/PD1).
	AFIO_PCFR1_TIM1_RM_ALT3      = 3 <<  6, // Complete mapping (ETR/PC2, CH1/PC4, CH2/PC7, CH3/PC5, CH4/PD4, BKIN/PC1, CH1N/PC3, CH2N/PD2, CH3N/PC6).
	AFIO_PCFR1_TIM2_RM_BITMASK   = 3 <<  8, // Remap bits for timer 2. It controls the mapping of Timer 2's channels 1 through 4 and external trigger (ETR) on the GPIO ports.
	AFIO_PCFR1_TIM2_RM_DEFAULT   = 0 <<  8, // Default mapping (CH1/ETR/PD4, CH2/PD3, CH3/PC0, CH4/PD7).
	AFIO_PCFR1_TIM2_RM_ALT1      = 1 <<  8, // Partial mapping (CH1/ETR/PC5, CH2/PC2, CH3/PD2, CH4/PC1).
	AFIO_PCFR1_TIM2_RM_ALT2      = 2 <<  8, // Partial mapping (CH1/ETR/PC1, CH2/PD3, CH3/PC0, CH4/PD7).
	AFIO_PCFR1_TIM2_RM_ALT3      = 3 <<  8, // Complete mapping (CH1/ETR/PC1, CH2/PC7, CH3/PD6, CH4/PD5).
	AFIO_PCFR1_PA12_RM           = 1 << 15, // Pin PA1 & PA2 remapping bit. It controls the proper function of PA1 and PA2 (set to 1 when connected to an external crystal pin)
	AFIO_PCFR1_ADC_ETRGINJ_RM    = 1 << 17, // Remap bit for ADC external trigger rule conversion.
	AFIO_PCFR1_ADC_ETRGREG_RM    = 1 << 18, // Remap bit for ADC external trigger rule conversion.
	AFIO_PCFR1_TIM1_IREMAP       = 1 << 23, // Control timer 1 channel 1 selection
	AFIO_PCFR1_SWCFG_BITMASK     = 7 << 24, // These bits are used to configure the I/O ports for SW function and trace function.
	AFIO_PCFR1_SWCFG_SDI         = 0 << 24, // SWD (SDI) enabled.
	AFIO_PCFR1_SWCFG_GPIO        = 4 << 24  // Turn off SWD (SDI), which functions as a GPIO.
} AFIOPCFR1Flag;

typedef enum {
	AFIO_EXTICR_EXTI_BITMASK = 3 << 0, // External interrupt input pin configuration bit. Used to determine to which port pins the external interrupt pins are mapped.
	AFIO_EXTICR_EXTI_GPIOA   = 0 << 0, // xth pin of the PA pin.
	AFIO_EXTICR_EXTI_GPIOC   = 2 << 0, // xth pin of the PC pin.
	AFIO_EXTICR_EXTI_GPIOD   = 3 << 0  // xth pin of the PD pin.
} AFIOEXTICRFlag;

#define AFIO_PCFR1  _MMIO32(AFIO_BASE | 0x4) // Remap Register 1
#define AFIO_EXTICR _MMIO32(AFIO_BASE | 0x8) // External interrupt configuration register 1

/* External interrupt controller */

#define EXTI_INTENR _MMIO32(EXTI_BASE | 0x00) // Interrupt enable register
#define EXTI_EVENR  _MMIO32(EXTI_BASE | 0x04) // Event enable register
#define EXTI_RTENR  _MMIO32(EXTI_BASE | 0x08) // Rising edge trigger enable register
#define EXTI_FTENR  _MMIO32(EXTI_BASE | 0x0c) // Falling edge trigger enable register
#define EXTI_SWIEVR _MMIO32(EXTI_BASE | 0x10) // Soft interrupt event register
#define EXTI_INTFR  _MMIO32(EXTI_BASE | 0x14) // Interrupt flag register

/* GPIO */

typedef enum {
	GPIO_CFGLR_MODE_BITMASK          = 3 << 0, // Port x mode selection, configure the corresponding port by these bits.
	GPIO_CFGLR_MODE_INPUT            = 0 << 0, // Input mode.
	GPIO_CFGLR_MODE_OUTPUT_10MHZ     = 1 << 0, // Output mode, maximum speed 10MHz.
	GPIO_CFGLR_MODE_OUTPUT_2MHZ      = 2 << 0, // Output mode, maximum speed 2MHz.
	GPIO_CFGLR_MODE_OUTPUT_30MHZ     = 3 << 0, // Output mode, maximum speed 30MHz.
	GPIO_CFGLR_CNF_BITMASK           = 3 << 2, // The configuration bits for port x, by which the corresponding port is configured.
	GPIO_CFGLR_CNF_IN_ANALOG         = 0 << 2, // Analog input mode.
	GPIO_CFGLR_CNF_IN_FLOATING       = 1 << 2, // Floating input mode.
	GPIO_CFGLR_CNF_IN_PULL           = 2 << 2, // With pull-up and pull-down mode.
	GPIO_CFGLR_CNF_OUT_PUSH_PULL     = 0 << 2, // Universal push-pull output mode.
	GPIO_CFGLR_CNF_OUT_OPEN_DRAIN    = 1 << 2, // Universal open-drain output mode.
	GPIO_CFGLR_CNF_OUT_AF_PUSH_PULL  = 2 << 2, // Multiplexed function push-pull output mode.
	GPIO_CFGLR_CNF_OUT_AF_OPEN_DRAIN = 3 << 2  // Multiplexed function open-drain output mode.
} GPIOCFGLRFlag;

#define GPIOA_CFGLR _MMIO32(GPIOA_BASE | 0x00) // PA port configuration register low
#define GPIOA_INDR  _MMIO32(GPIOA_BASE | 0x08) // PA port input data register
#define GPIOA_OUTDR _MMIO32(GPIOA_BASE | 0x0c) // PA port output data register
#define GPIOA_BSHR  _MMIO32(GPIOA_BASE | 0x10) // PA port set/reset register
#define GPIOA_BCR   _MMIO32(GPIOA_BASE | 0x14) // PA port reset register
#define GPIOA_LCKR  _MMIO32(GPIOA_BASE | 0x18) // PA port configuration lock register

#define GPIOC_CFGLR _MMIO32(GPIOC_BASE | 0x00) // PC port configuration register low
#define GPIOC_INDR  _MMIO32(GPIOC_BASE | 0x08) // PC port input data register
#define GPIOC_OUTDR _MMIO32(GPIOC_BASE | 0x0c) // PC port output data register
#define GPIOC_BSHR  _MMIO32(GPIOC_BASE | 0x10) // PC port set/reset register
#define GPIOC_BCR   _MMIO32(GPIOC_BASE | 0x14) // PC port reset register
#define GPIOC_LCKR  _MMIO32(GPIOC_BASE | 0x18) // PC port configuration lock register

#define GPIOD_CFGLR _MMIO32(GPIOD_BASE | 0x00) // PD port configuration register low
#define GPIOD_INDR  _MMIO32(GPIOD_BASE | 0x08) // PD port input data register
#define GPIOD_OUTDR _MMIO32(GPIOD_BASE | 0x0c) // PD port output data register
#define GPIOD_BSHR  _MMIO32(GPIOD_BASE | 0x10) // PD port set/reset register
#define GPIOD_BCR   _MMIO32(GPIOD_BASE | 0x14) // PD port reset register
#define GPIOD_LCKR  _MMIO32(GPIOD_BASE | 0x18) // PD port configuration lock register

/* ADC */

typedef enum {
	ADC_STATR_AWD   = 1 << 0, // Analog watchdog flag bit.
	ADC_STATR_EOC   = 1 << 1, // Conversion end state.
	ADC_STATR_JEOC  = 1 << 2, // Injection into the end state of the channel group conversion.
	ADC_STATR_JSTRT = 1 << 3, // Injection channel conversion start state.
	ADC_STATR_STRT  = 1 << 4  // Rule channel transition start state
} ADCSTATRFlag;

typedef enum {
	ADC_CTLR1_AWDCH_BITMASK   = 31 <<  0, // Analog watchdog channel selection bits.
	ADC_CTLR1_EOCIE           =  1 <<  5, // End of conversion (rule or injection channel group) interrupt enable bit.
	ADC_CTLR1_AWDIE           =  1 <<  6, // Analog watchdog interrupt enable bit.
	ADC_CTLR1_JEOCIE          =  1 <<  7, // Inject the channel group end-of-conversion interrupt enable bit
	ADC_CTLR1_SCAN            =  1 <<  8, // Scan mode enable bit.
	ADC_CTLR1_AWDSGL          =  1 <<  9, // In scan mode, use the analog watchdog enable bit on a single channel.
	ADC_CTLR1_JAUTO           =  1 << 10, // After the opening of the rule channel is completed, the injection channel group enable bit is automatically switched.
	ADC_CTLR1_DISCEN          =  1 << 11, // Intermittent mode enable bit on rule channel.
	ADC_CTLR1_JDISCEN         =  1 << 12, // Inject the intermittent mode enable bit on the channel.
	ADC_CTLR1_DISCNUM_BITMASK =  7 << 13, // Number of rule channels to be converted after external triggering in intermittent mode.
	ADC_CTLR1_JAWDEN          =  1 << 22, // Analog watchdog function enable bit on the injection channel.
	ADC_CTLR1_AWDEN           =  1 << 23, // Analog watchdog function enable bit on the rule channel.
	ADC_CTLR1_CALVOL_BITMASK  =  3 << 25, // Calibration voltage selection
	ADC_CTLR1_CALVOL_50       =  1 << 25, // Calibration voltage 2/4 AVDD
	ADC_CTLR1_CALVOL_75       =  2 << 25  // Calibration voltage 3/4 AVDD
} ADCCTLR1Flag;

typedef enum {
	ADC_CTLR2_ADON             = 1 <<  0, // On/off A/D converter. When this bit is 0, writing 1 will wake up the ADC from power-down mode; when this bit is 1, writing 1 will start the conversion.
	ADC_CTLR2_CONT             = 1 <<  1, // Continuous conversion enable.
	ADC_CTLR2_CAL              = 1 <<  2, // A/D calibration, this bit is set by software and cleared to 0 by hardware at the end of calibration.
	ADC_CTLR2_RSTCAL           = 1 <<  3, // Reset calibration, this bit is set by software and cleared by hardware after the reset is completed.
	ADC_CTLR2_DMA              = 1 <<  8, // Direct Memory Access (DMA) mode enable.
	ADC_CTLR2_ALIGN            = 1 << 11, // Data alignment.
	ADC_CTLR2_JEXTSEL_BITMASK  = 7 << 12, // External trigger event selection for initiating injection channel conversion.
	ADC_CTLR2_JEXTSEL_TIM1_CC3 = 0 << 12, // CC3 event of timer 1.
	ADC_CTLR2_JEXTSEL_TIM1_CC4 = 1 << 12, // CC4 event of timer 1.
	ADC_CTLR2_JEXTSEL_TIM2_CC3 = 2 << 12, // CC3 event of timer 2.
	ADC_CTLR2_JEXTSEL_TIM2_CC4 = 3 << 12, // CC4 event of timer 2.
	ADC_CTLR2_JEXTSEL_GPIO     = 6 << 12, // PD1/PA2.
	ADC_CTLR2_JEXTSEL_MANUAL   = 7 << 12, // JSWSTART software trigger.
	ADC_CTLR2_JEXTTRIG         = 1 << 15, // External trigger transition mode enable for the injected channel.
	ADC_CTLR2_EXTSEL_BITMASK   = 7 << 17, // External trigger event selection for initiating rule channel conversion.
	ADC_CTLR2_EXTSEL_TIM1_TRGO = 0 << 17, // TRGO event of timer 1.
	ADC_CTLR2_EXTSEL_TIM1_CC1  = 1 << 17, // CC1 event of timer 1.
	ADC_CTLR2_EXTSEL_TIM1_CC2  = 2 << 17, // CC2 event of timer 1.
	ADC_CTLR2_EXTSEL_TIM2_TRGO = 3 << 17, // TRGO event of timer 2.
	ADC_CTLR2_EXTSEL_TIM2_CC1  = 4 << 17, // CC1 event of timer 2.
	ADC_CTLR2_EXTSEL_TIM2_CC2  = 5 << 17, // CC2 event of timer 2.
	ADC_CTLR2_EXTSEL_GPIO      = 6 << 17, // PD3/PC2 events.
	ADC_CTLR2_EXTSEL_MANUAL    = 7 << 17, // SWSTART software trigger.
	ADC_CTLR2_EXTTRIG          = 1 << 20, // External trigger transition mode enable for the rule channel.
	ADC_CTLR2_JSWSTART         = 1 << 21, // This bit is set by software and is cleared to 0 by hardware or 0 by software when the conversion starts.
	ADC_CTLR2_SWSTART          = 1 << 22, // This bit is set by software and cleared to 0 by hardware when conversion starts.
} ADCCTLR2Flag;

typedef enum {
	ADC_SAMPTR_SMP_BITMASK = 7 << 0, // Sample time configuration for channel x.
	ADC_SAMPTR_SMP_3       = 0 << 0, // 3 cycles.
	ADC_SAMPTR_SMP_9       = 1 << 0, // 9 cycles.
	ADC_SAMPTR_SMP_15      = 2 << 0, // 15 cycles.
	ADC_SAMPTR_SMP_30      = 3 << 0, // 30 cycles.
	ADC_SAMPTR_SMP_43      = 4 << 0, // 43 cycles.
	ADC_SAMPTR_SMP_57      = 5 << 0, // 57 cycles.
	ADC_SAMPTR_SMP_73      = 6 << 0, // 73 cycles.
	ADC_SAMPTR_SMP_241     = 7 << 0  // 241 cycles.
} ADCSAMPTRFlag;

#define ADC_STATR     _MMIO32(ADC_BASE | 0x00) // ADC status register
#define ADC_CTLR1     _MMIO32(ADC_BASE | 0x04) // ADC control register 1
#define ADC_CTLR2     _MMIO32(ADC_BASE | 0x08) // ADC control register 2
#define ADC_SAMPTR1   _MMIO32(ADC_BASE | 0x0c) // ADC sample time register 1
#define ADC_SAMPTR2   _MMIO32(ADC_BASE | 0x10) // ADC sample time register 2
#define ADC_IOFR(N)   _MMIO32((ADC_BASE | 0x14) + (4 * (N))) // ADC injected channel data offset register 1
#define ADC_WDHTR     _MMIO32(ADC_BASE | 0x24) // ADC watchdog high threshold register
#define ADC_WDLTR     _MMIO32(ADC_BASE | 0x28) // ADC watchdog low threshold register
#define ADC_RSQR1     _MMIO32(ADC_BASE | 0x2c) // ADC regular sequence register 1
#define ADC_RSQR2     _MMIO32(ADC_BASE | 0x30) // ADC regular sequence register 2
#define ADC_RSQR3     _MMIO32(ADC_BASE | 0x34) // ADC regular sequence register 3
#define ADC_ISQR      _MMIO32(ADC_BASE | 0x38) // ADC injected sequence register
#define ADC_IDATAR(N) _MMIO32((ADC_BASE | 0x3c) + (4 * (N))) // ADC injected data register
#define ADC_RDATAR    _MMIO32(ADC_BASE | 0x4c) // ADC regular data register
#define ADC_DLYR      _MMIO32(ADC_BASE | 0x50) // ADC delayed data register

/* Timer 1 */

// TODO: add register bit definitions

#define TIM1_CTLR1     _MMIO16(TIM1_BASE | 0x00) // Control register 1
#define TIM1_CTLR2     _MMIO16(TIM1_BASE | 0x04) // Control register 2
#define TIM1_SMCFGR    _MMIO16(TIM1_BASE | 0x08) // Slave mode control register
#define TIM1_DMAINTENR _MMIO16(TIM1_BASE | 0x0c) // DMA/interrupt enable register
#define TIM1_INTFR     _MMIO16(TIM1_BASE | 0x10) // Interrupt status register
#define TIM1_SWEVGR    _MMIO16(TIM1_BASE | 0x14) // Event generation register
#define TIM1_CHCTLR1   _MMIO16(TIM1_BASE | 0x18) // Compare/capture control register 1
#define TIM1_CHCTLR2   _MMIO16(TIM1_BASE | 0x1c) // Compare/capture control register 2
#define TIM1_CCER      _MMIO16(TIM1_BASE | 0x20) // Compare/capture enable register
#define TIM1_CNT       _MMIO16(TIM1_BASE | 0x24) // Counters
#define TIM1_PSC       _MMIO16(TIM1_BASE | 0x28) // Counting clock prescaler
#define TIM1_ATRLR     _MMIO16(TIM1_BASE | 0x2c) // Auto-reload value register
#define TIM1_RPTCR     _MMIO16(TIM1_BASE | 0x30) // Recurring count value register
#define TIM1_CHCVR(N)  _MMIO32((TIM1_BASE | 0x34) + (4 * (N))) // Compare/capture register
#define TIM1_BDTR      _MMIO16(TIM1_BASE | 0x44) // Brake and deadband registers
#define TIM1_DMACFGR   _MMIO16(TIM1_BASE | 0x48) // DMA control register
#define TIM1_DMAADR    _MMIO16(TIM1_BASE | 0x4c) // DMA address register for continuous mode

/* SPI */

typedef enum {
	SPI_CTLR1_CPHA       = 1 <<  0, // Clock phase setting bit, this bit cannot be modified during communication.
	SPI_CTLR1_CPOL       = 1 <<  1, // Clock polarity selection bit, this bit cannot be modified during communication.
	SPI_CTLR1_MSTR       = 1 <<  2, // Master-slave setting bit, this bit cannot be modified during communication.
	SPI_CTLR1_BR_BITMASK = 7 <<  3, // Baud rate setting field, this field cannot be modified during communication.
	SPI_CTLR1_BR_DIV2    = 0 <<  3,
	SPI_CTLR1_BR_DIV4    = 1 <<  3,
	SPI_CTLR1_BR_DIV8    = 2 <<  3,
	SPI_CTLR1_BR_DIV16   = 3 <<  3,
	SPI_CTLR1_BR_DIV32   = 4 <<  3,
	SPI_CTLR1_BR_DIV64   = 5 <<  3,
	SPI_CTLR1_BR_DIV128  = 6 <<  3,
	SPI_CTLR1_BR_DIV256  = 7 <<  3,
	SPI_CTLR1_SPE        = 1 <<  6, // SPI enable bit.
	SPI_CTLR1_LSBFIRST   = 1 <<  7, // Frame format control bit. It is not possible to modify this bit during communication.
	SPI_CTLR1_SSI        = 1 <<  8, // Internal slave select bit, with SSM set, this bit determines the level of the NSS pin.
	SPI_CTLR1_SSM        = 1 <<  9, // Software slave management bit, this bit determines whether the level of the NSS pin is controlled by hardware or software.
	SPI_CTLR1_RXONLY     = 1 << 10, // The receive-only bit in 2-wire mode is used in conjunction with BIDIMODE. Setting this bit allows the device to receive only and not transmit.
	SPI_CTLR1_DFF        = 1 << 11, // Data frame format bit, this bit can only be written when SPE is 0.
	SPI_CTLR1_CRCNEXT    = 1 << 12, // After the next data transfer, send the value of the CRC register. This should be set immediately after the last data is written to the data register.
	SPI_CTLR1_CRCEN      = 1 << 13, // Hardware CRC checksum enable bit, this bit can only be written when SPE is 0. This bit can only be used in full-duplex mode.
	SPI_CTLR1_BIDIOE     = 1 << 14, // Output enable in bidirectional mode bit, used in conjunction with BIDImode.
	SPI_CTLR1_BIDIMODE   = 1 << 15  // Bidirectional data mode enable bit.
} SPICTLR1Flag;

typedef enum {
	SPI_CTLR2_RXDMAEN = 1 << 0, // Rx buffer DMA enable bit.
	SPI_CTLR2_TXDMAEN = 1 << 1, // Tx buffer DMA enable bit.
	SPI_CTLR2_SSOE    = 1 << 2, // SS output enable bit. Disabling SS output can work in multi-master mode.
	SPI_CTLR2_ERRIE   = 1 << 5, // Error interrupt enable bit. Setting this bit allows interrupts to be generated when errors (CRCERR, OVR, MODF) are generated.
	SPI_CTLR2_RXNEIE  = 1 << 6, // RX buffer not empty interrupt enable bit. Used to generate an interrupt request when the RXNE flag is set.
	SPI_CTLR2_TXEIE   = 1 << 7  // Tx buffer empty interrupt enable bit. Setting this bit allows an interrupt to be generated when TXE is set.
} SPICTLR2Flag;

typedef enum {
	SPI_STATR_RXNE   = 1 << 0, // Receive buffer not empty. Note: Read DATAR and auto-zero.
	SPI_STATR_TXE    = 1 << 1, // Transmit buffer empty.
	SPI_STATR_CHSID  = 1 << 2, // Channel side. This flag is set by hardware and reset by a software sequence.
	SPI_STATR_UDR    = 1 << 3, // Underrun flag. This flag is set by hardware and reset by a software sequence.
	SPI_STATR_CRCERR = 1 << 4, // CRC error flag. This flag is set by hardware and reset by a software sequence.
	SPI_STATR_MODF   = 1 << 5, // Mode fault. This flag is set by hardware and reset by a software sequence.
	SPI_STATR_OVR    = 1 << 6, // Overrun flag. This flag is set by hardware and reset by a software sequence.
	SPI_STATR_BSY    = 1 << 7  // Busy flag. This flag is set and cleared by hardware.
} SPISTATRFlag;

typedef enum {
	SPI_HSCR_HSRXEN = 1 << 0 // Read enable in SPI high-speed mode. The bit is write-only.
} SPIHSCRFlag;

#define SPI_CTLR1 _MMIO16(SPI_BASE | 0x00) // SPI Control register 1
#define SPI_CTLR2 _MMIO16(SPI_BASE | 0x04) // SPI Control register 2
#define SPI_STATR _MMIO16(SPI_BASE | 0x08) // SPI Status register
#define SPI_DATAR _MMIO16(SPI_BASE | 0x0c) // SPI Data register
#define SPI_CRCR  _MMIO16(SPI_BASE | 0x10) // SPI Polynomial register
#define SPI_RCRCR _MMIO16(SPI_BASE | 0x14) // SPI Receive CRC register
#define SPI_TCRCR _MMIO16(SPI_BASE | 0x18) // SPI Transmit CRC register
#define SPI_HSCR  _MMIO16(SPI_BASE | 0x24) // SPI High-speed control register

/* USART */

typedef enum {
	USART_STATR_PE   = 1 << 0, // Checksum error flag. A read of this bit and then a read of the data register operation resets this bit.
	USART_STATR_FE   = 1 << 1, // Frame error flag. Reading this bit and then reading the data register operation will reset this bit.
	USART_STATR_NE   = 1 << 2, // Noise error flag. The operation of reading the status register and then reading the data register resets this bit.
	USART_STATR_ORE  = 1 << 3, // Overload error flag.
	USART_STATR_IDLE = 1 << 4, // Bus idle flag. The operation of reading the status register and then reading the data register will clear this bit.
	USART_STATR_RXNE = 1 << 5, // Read data register non-empty flag. A read operation of the data register clears this bit. It is also possible to clear the bit by writing a 0 directly.
	USART_STATR_TC   = 1 << 6, // Send completion flag. The software will clear this bit by reading it and then writing to the data register. It is also possible to write 0 directly to clear this bit.
	USART_STATR_TXE  = 1 << 7, // Send data register empty flag. If TXEIE is already set, an interrupt will be generated to perform a write operation to the data register and this bit will be reset.
	USART_STATR_LBD  = 1 << 8, // LIN Break detection flag. This bit is set by hardware when a LIN Break is detected. It is cleared by software.
	USART_STATR_CTS  = 1 << 9  // CTS state change flag. If the CTSE bit is set, this bit will be set high by hardware when the nCTS output state changes. It is cleared to zero by software.
} USARTSTATRFlag;

typedef enum {
	USART_CTLR1_SBK          = 1 <<  0, // Send break bit. Set this bit to send break character. It is reset by hardware on the stop bit of the break frame.
	USART_CTLR1_RWU          = 1 <<  1, // Receiver wakeup. This bit determines whether to place the USART in silent mode.
	USART_CTLR1_RE           = 1 <<  2, // Receiver enable. Setting this bit enables the receiver, which starts detecting the start bit on the RX pin.
	USART_CTLR1_TE           = 1 <<  3, // Transmitter enable. Setting this bit will enable the transmitter.
	USART_CTLR1_IDLEIE       = 1 <<  4, // IDLE interrupt enable. This bit allows IDLE interrupt to be generated.
	USART_CTLR1_RXNEIE       = 1 <<  5, // RXNE interrupt enable. This bit indicates that a RXNE interrupt is allowed to be generated.
	USART_CTLR1_TCIE         = 1 <<  6, // Transmit completion interrupt enable. This bit indicates that the transmit completion interrupt is allowed to be generated.
	USART_CTLR1_TXEIE        = 1 <<  7, // TXE interrupt enable. This bit indicates that a TXE interrupt is allowed to be generated.
	USART_CTLR1_PEIE         = 1 <<  8, // Parity check interrupt enable bit. This bit indicates that parity check error interrupts are allowed.
	USART_CTLR1_PS_BITMASK   = 1 <<  9, // Parity selection. When this bit is set, the parity bit enable will take effect only after the current byte transmission is completed.
	USART_CTLR1_PS_EVEN      = 0 <<  9,
	USART_CTLR1_PS_ODD       = 1 <<  9,
	USART_CTLR1_PCE          = 1 << 10, // The parity bit is enabled. For the receiver, it is the parity check of the data; for the sender, it is the insertion of the parity bit.
	USART_CTLR1_WAKE_BITMASK = 1 << 11, // Wake-up bit. This bit determines the method of waking up the USART.
	USART_CTLR1_WAKE_IDLE    = 0 << 11, // Bus idle.
	USART_CTLR1_WAKE_ADDRESS = 1 << 11, // Address marker.
	USART_CTLR1_M_BITMASK    = 1 << 12, // Word long bit.
	USART_CTLR1_M_8          = 0 << 12, // 8 data bits.
	USART_CTLR1_M_9          = 1 << 12, // 9 data bits.
	USART_CTLR1_UE           = 1 << 13  // USART enable bit. When this bit is set, both the USART divider and the output stop working after the current byte transfer is completed.
} USARTCTLR1Flag;

typedef enum {
	USART_CTLR2_ADD_BITMASK  = 15 <<  0, // Address of the USART node, this bit-field gives the address of the USART node.
	USART_CTLR2_LBDL_BITMASK =  1 <<  5, // LIN Break detection length, this bit is used to select whether the Break character detection is 11 bits or 10 bits.
	USART_CTLR2_LBDL_10      =  0 <<  5, // 10-bit Break character detection.
	USART_CTLR2_LBDL_11      =  1 <<  5, // 11-bit Break character detection.
	USART_CTLR2_LBDIE        =  1 <<  6, // LIN Break detection interrupt enable, this position bit enables interrupts caused by LBD.
	USART_CTLR2_LBCL         =  1 <<  8, // This bit allows the user to select whether the clock pulse associated with the last data bit transmitted (MSB) has to be output on the CK pin in synchronous mode.
	USART_CTLR2_CPHA         =  1 <<  9, // This bit allows the user to select the phase of the clock output on the CK pin in synchronous mode.
	USART_CTLR2_CPOL         =  1 << 10, // This bit allows the user to select the polarity of the clock output on the CK pin in synchronous mode.
	USART_CTLR2_CLKEN        =  1 << 11, // This bit allows the user to enable the CK pin.
	USART_CTLR2_STOP_BITMASK =  3 << 12, // These bits are used for programming the stop bits.
	USART_CTLR2_STOP_1       =  0 << 12, // 1 Stop bit
	USART_CTLR2_STOP_0_5     =  1 << 12, // 0.5 Stop bit
	USART_CTLR2_STOP_2       =  2 << 12, // 2 Stop bits
	USART_CTLR2_STOP_1_5     =  3 << 12, // 1.5 Stop bit
	USART_CTLR2_LINEN        =  1 << 14  // LIN mode enable, set to enable LIN mode. The LIN mode enables the capability to send LIN Synch Breaks using the SBK bit in the USART_CTLR1 register, and to detect LIN Sync breaks.
} USARTCTLR2Flag;

typedef enum {
	USART_CTLR3_EIE   = 1 <<  0, // Error interrupt enable bit, when set, generates an interrupt if FE, ORE or NE is set provided that DMAR is set.
	USART_CTLR3_IREN  = 1 <<  1, // IrDA enable bit, set this bit to enable infrared mode.
	USART_CTLR3_IRLP  = 1 <<  2, // IrDA low-power bit, set this bit to enable low-power mode when IrDA is selected.
	USART_CTLR3_HDSEL = 1 <<  3, // Half-duplex selection bit, set this bit to select half-duplex mode.
	USART_CTLR3_NACK  = 1 <<  4, // Smartcard NACK enable bit, set this bit to send NACK in case of check error.
	USART_CTLR3_SCEN  = 1 <<  5, // Smartcard mode enable bit, set to 1 to enable smart card mode.
	USART_CTLR3_DMAR  = 1 <<  6, // DMA receive enable bit. This position 1 uses DMA on receive.
	USART_CTLR3_DMAT  = 1 <<  7, // DMA transmit enable bit. This bit 1 uses DMA when transmitting.
	USART_CTLR3_RTSE  = 1 <<  8, // RTS enable bit, setting this bit will enable RTS flow control.
	USART_CTLR3_CTSE  = 1 <<  9, // CTS enable bit, setting this bit will enable CTS flow control.
	USART_CTLR3_CTSIE = 1 << 10  // CTS interrupt enable bit, when this bit is set, an interrupt will be generated when CTS is set.
} USARTCTLR3Flag;

typedef enum {
	USART_GPR_PSC_BITMASK = 255 << 0, // Prescaler value field.
	USART_GPR_GT_BITMASK  = 255 << 8  // Guard time value. This bit-field gives the Guard time value in terms of number of baud clocks.
} USARTGPRFlag;

#define USART_STATR _MMIO32(USART_BASE | 0x00) // UASRT status register
#define USART_DATAR _MMIO32(USART_BASE | 0x04) // UASRT data register
#define USART_BRR   _MMIO32(USART_BASE | 0x08) // UASRT baud rate register
#define USART_CTLR1 _MMIO32(USART_BASE | 0x0c) // UASRT control register 1
#define USART_CTLR2 _MMIO32(USART_BASE | 0x10) // UASRT control register 2
#define USART_CTLR3 _MMIO32(USART_BASE | 0x14) // UASRT control register 3
#define USART_GPR   _MMIO32(USART_BASE | 0x18) // UASRT protection time and prescaler register

/* DMA */

typedef enum {
	DMA_INTFR_GIF  = 1 << 0, // Global interrupt flag for channel x.
	DMA_INTFR_TCIF = 1 << 1, // Transmission completion flag for channel x.
	DMA_INTFR_HTIF = 1 << 2, // Transmission halfway flag for channel x.
	DMA_INTFR_TEIF = 1 << 3  // Transmission error flag for channel x.
} DMAINTFRFlag;

typedef enum {
	DMA_INTFCR_CGIF  = 1 << 0, // Clear the global interrupt flag for channel x.
	DMA_INTFCR_CTCIF = 1 << 1, // Clear the transmission completion flag for channel x.
	DMA_INTFCR_CHTIF = 1 << 2, // Clear the transmission halfway flag for channel x.
	DMA_INTFCR_CTEIF = 1 << 3  // Clear the transmission error flag for channel x.
} DMAINTFCRFlag;

typedef enum {
	DMA_CFGR_EN            = 1 <<  0, // Channel enable control.
	DMA_CFGR_TCIE          = 1 <<  1, // Transmission completion interrupt enable control.
	DMA_CFGR_HTIE          = 1 <<  2, // Transmission over half interrupt enable control.
	DMA_CFGR_TEIE          = 1 <<  3, // Transmission error interrupt enable control.
	DMA_CFGR_DIR_BITMASK   = 1 <<  4, // Data transfer direction.
	DMA_CFGR_DIR_READ      = 0 <<  4, // Read from peripheral.
	DMA_CFGR_DIR_WRITE     = 1 <<  4, // Read from memory.
	DMA_CFGR_CIRC          = 1 <<  5, // DMA channel cyclic mode enable.
	DMA_CFGR_PINC          = 1 <<  6, // Peripheral address incremental incremental mode enable.
	DMA_CFGR_MINC          = 1 <<  7, // Memory address incremental incremental mode enable.
	DMA_CFGR_PSIZE_BITMASK = 3 <<  8, // Peripheral address data width setting.
	DMA_CFGR_PSIZE_8       = 0 <<  8, // 8 bits.
	DMA_CFGR_PSIZE_16      = 1 <<  8, // 16 bits.
	DMA_CFGR_PSIZE_32      = 2 <<  8, // 32 bits.
	DMA_CFGR_MSIZE_BITMASK = 3 << 10, // Memory address data width setting.
	DMA_CFGR_MSIZE_8       = 0 << 10, // 8 bits.
	DMA_CFGR_MSIZE_16      = 1 << 10, // 16 bits.
	DMA_CFGR_MSIZE_32      = 2 << 10, // 32 bits.
	DMA_CFGR_PL_BITMASK    = 3 << 12, // Channel priority setting.
	DMA_CFGR_PL_LOW        = 0 << 12, // Low.
	DMA_CFGR_PL_MEDIUM     = 1 << 12, // Medium.
	DMA_CFGR_PL_HIGH       = 2 << 12, // High.
	DMA_CFGR_PL_HIGHEST    = 3 << 12, // Very high.
	DMA_CFGR_MEM2MEM       = 1 << 14  // Memory-to-memory mode enable.
} DMACFGRFlag;

#define DMA_INTFR    _MMIO32(DMA_BASE | 0x00) // DMA interrupt status register
#define DMA_INTFCR   _MMIO32(DMA_BASE | 0x04) // DMA interrupt flag clear register
#define DMA_CFGR(N)  _MMIO32((DMA_BASE | 0x08) + (20 * (N))) // DMA channel configuration register
#define DMA_CNTR(N)  _MMIO32((DMA_BASE | 0x0c) + (20 * (N))) // DMA channel number of data register
#define DMA_PADDR(N) _MMIO32((DMA_BASE | 0x10) + (20 * (N))) // DMA channel peripheral address register
#define DMA_MADDR(N) _MMIO32((DMA_BASE | 0x14) + (20 * (N))) // DMA channel memory address register

/* Reset and clock control */

typedef enum {
	RCC_CTLR_HSION           =   1 <<  0, // Internal high-speed clock (24MHz) enable control bit.
	RCC_CTLR_HSIRDY          =   1 <<  1, // Internal high-speed clock (24MHz) Stable Ready flag bit (set by hardware).
	RCC_CTLR_HSITRIM_BITMASK =  31 <<  3, // Internal high-speed clock adjustment value.
	RCC_CTLR_HSICAL_BITMASK  = 255 <<  8, // Internal high-speed clock calibration values, which are automatically initialized at system startup.
	RCC_CTLR_HSEON           =   1 << 16, // External high-speed crystal oscillation enable control bit.
	RCC_CTLR_HSERDY          =   1 << 17, // External high-speed crystal oscillation stabilization ready flag bit (set by hardware).
	RCC_CTLR_HSEBYP          =   1 << 18, // External high-speed crystal bypass control bit.
	RCC_CTLR_CSSON           =   1 << 19, // Clock security system enable control bit.
	RCC_CTLR_PLLON           =   1 << 24, // PLL clock enable control bit.
	RCC_CTLR_PLLRDY          =   1 << 25  // PLL clock-ready lock flag bit.
} RCCCTLRFlag;

typedef enum {
	RCC_CFGR0_SW_BITMASK     =  3 <<  0, // Select the system clock source.
	RCC_CFGR0_SW_HSI         =  0 <<  0, // HSI as system clock.
	RCC_CFGR0_SW_HSE         =  1 <<  0, // HSE as system clock.
	RCC_CFGR0_SW_PLL         =  2 <<  0, // PLL output as system clock.
	RCC_CFGR0_SWS_BITMASK    =  3 <<  2, // System clock (SYSCLK) status (hardware set).
	RCC_CFGR0_SWS_HSI        =  0 <<  2, // The system clock source is HSI.
	RCC_CFGR0_SWS_HSE        =  1 <<  2, // The system clock source is HSE.
	RCC_CFGR0_SWS_PLL        =  2 <<  2, // The system clock source is a PLL.
	RCC_CFGR0_HPRE_BITMASK   = 15 <<  4, // HB clock source prescaler control.
	RCC_CFGR0_HPRE_DIV1      =  0 <<  4, // Prescaler off.
	RCC_CFGR0_HPRE_DIV2      =  1 <<  4, // SYSCLK divided by 2.
	RCC_CFGR0_HPRE_DIV3      =  2 <<  4, // SYSCLK divided by 3.
	RCC_CFGR0_HPRE_DIV4      =  3 <<  4, // SYSCLK divided by 4.
	RCC_CFGR0_HPRE_DIV5      =  4 <<  4, // SYSCLK divided by 5.
	RCC_CFGR0_HPRE_DIV6      =  5 <<  4, // SYSCLK divided by 6.
	RCC_CFGR0_HPRE_DIV7      =  6 <<  4, // SYSCLK divided by 7.
	RCC_CFGR0_HPRE_DIV8      =  7 <<  4, // SYSCLK divided by 8.
	RCC_CFGR0_HPRE_DIV16     = 11 <<  4, // SYSCLK divided by 16.
	RCC_CFGR0_HPRE_DIV32     = 12 <<  4, // SYSCLK divided by 32.
	RCC_CFGR0_HPRE_DIV64     = 13 <<  4, // SYSCLK divided by 64.
	RCC_CFGR0_HPRE_DIV128    = 14 <<  4, // SYSCLK divided by 128.
	RCC_CFGR0_HPRE_DIV256    = 15 <<  4, // SYSCLK divided by 256.
	RCC_CFGR0_ADCPRE_BITMASK = 31 << 11, // ADC clock source prescaler control.
	RCC_CFGR0_ADCPRE_DIV2    =  0 << 11, // HBCLK divided by 2 as ADC clock.
	RCC_CFGR0_ADCPRE_DIV4    =  4 << 11, // HBCLK divided by 4 as ADC clock.
	RCC_CFGR0_ADCPRE_DIV8    =  5 << 11, // HBCLK divided by 8 as ADC clock.
	RCC_CFGR0_ADCPRE_DIV16   =  6 << 11, // HBCLK divided by 16 as ADC clock.
	RCC_CFGR0_ADCPRE_DIV32   =  7 << 11, // HBCLK divided by 32 as ADC clock.
	RCC_CFGR0_ADCPRE_DIV64   = 15 << 11, // HBCLK divided by 64 as ADC clock.
	RCC_CFGR0_ADCPRE_DIV6    = 16 << 11, // HBCLK divided by 6 as ADC clock.
	RCC_CFGR0_ADCPRE_DIV12   = 20 << 11, // HBCLK divided by 12 as ADC clock.
	RCC_CFGR0_ADCPRE_DIV24   = 21 << 11, // HBCLK divided by 24 as ADC clock.
	RCC_CFGR0_ADCPRE_DIV48   = 22 << 11, // HBCLK divided by 48 as ADC clock.
	RCC_CFGR0_ADCPRE_DIV96   = 23 << 11, // HBCLK divided by 96 as ADC clock.
	RCC_CFGR0_ADCPRE_DIV128  = 31 << 11, // HBCLK divided by 128 as ADC clock.
	RCC_CFGR0_PLLSRC_BITMASK =  1 << 16, // Input clock source for PLL (write only when PLL is off).
	RCC_CFGR0_PLLSRC_HSI     =  0 << 16, // HSI is not divided and sent to PLL.
	RCC_CFGR0_PLLSRC_HSE     =  1 << 16, // HSE is fed into PLL without dividing the frequency.
	RCC_CFGR0_MCO_BITMASK    =  7 << 24, // Microcontroller MCO pin clock output control.
	RCC_CFGR0_MCO_NONE       =  0 << 24, // No clock output.
	RCC_CFGR0_MCO_SYSCLK     =  4 << 24, // System clock (SYSCLK) output.
	RCC_CFGR0_MCO_HSI        =  5 << 24, // Internal 24 MHz RC oscillator clock (HSI) output.
	RCC_CFGR0_MCO_HSE        =  6 << 24, // External oscillator clock (HSE) output.
	RCC_CFGR0_MCO_PLL        =  7 << 24, // PLL clock output.
} RCCCFGR0Flag;

typedef enum {
	RCC_INTR_LSIRDYF  = 1 <<  0, // LSI clock-ready interrupt flag.
	RCC_INTR_HSIRDYF  = 1 <<  2, // HSI clock-ready interrupt flag.
	RCC_INTR_HSERDYF  = 1 <<  3, // HSE clock-ready interrupt flag.
	RCC_INTR_PLLRDYF  = 1 <<  4, // PLL clock-ready lockout interrupt flag.
	RCC_INTR_CSSF     = 1 <<  7, // Clock security system interrupt flag bit.
	RCC_INTR_LSIRDYIE = 1 <<  8, // LSI-ready interrupt enable bit.
	RCC_INTR_HSIRDYIE = 1 << 10, // HSI-ready interrupt enable bit.
	RCC_INTR_HSERDYIE = 1 << 11, // HSE-ready interrupt enable bit.
	RCC_INTR_PLLRDYIE = 1 << 12, // PLL-ready interrupt enable bit.
	RCC_INTR_LSIRDYC  = 1 << 16, // Clear the LSI oscillator ready interrupt flag bit.
	RCC_INTR_HSIRDYC  = 1 << 18, // Clear the HSI oscillator ready interrupt flag bit.
	RCC_INTR_HSERDYC  = 1 << 19, // Clear the HSE oscillator ready interrupt flag bit.
	RCC_INTR_PLLRDYC  = 1 << 20, // Clear the PLL-ready interrupt flag bit.
	RCC_INTR_CSSC     = 1 << 23  // Clear the clock security system interrupt flag bit (CSSF).
} RCCINTRFlag;

typedef enum {
	RCC_APB2PRSTR_AFIORST   = 1 <<  0, // I/O auxiliary function module reset control.
	RCC_APB2PRSTR_IOPARST   = 1 <<  2, // PA port module reset control for I/O.
	RCC_APB2PRSTR_IOPCRST   = 1 <<  4, // PC port module reset control for I/O.
	RCC_APB2PRSTR_IOPDRST   = 1 <<  5, // PD port module reset control for I/O.
	RCC_APB2PRSTR_ADC1RST   = 1 <<  9, // ADC1 module reset control.
	RCC_APB2PRSTR_TIM1RST   = 1 << 11, // TIM1 module reset control.
	RCC_APB2PRSTR_SPI1RST   = 1 << 12, // SPI1 interface reset control.
	RCC_APB2PRSTR_USART1RST = 1 << 14  // USART1 interface reset control.
} RCCAPB2PRSTRFlag;

typedef enum {
	RCC_APB1PRSTR_TIM2RST = 1 <<  0, // Timer 2 module reset control.
	RCC_APB1PRSTR_WWDGRST = 1 << 11, // Window watchdog reset control.
	RCC_APB1PRSTR_I2C1RST = 1 << 21, // I2C 1 interface reset control.
	RCC_APB1PRSTR_PWRRST  = 1 << 28  // Power interface module reset control.
} RCCAPB1PRSTRFlag;

typedef enum {
	RCC_AHBPCENR_DMA1EN = 1 << 0, // DMA1 module clock enable bit.
	RCC_AHBPCENR_SRAMEN = 1 << 2  // SRAM interface module clock enable bit.
} RCCAHBPCENRFlag;

typedef enum {
	RCC_APB2PCENR_AFIOEN   = 1 <<  0, // I/O auxiliary function module clock enable bit.
	RCC_APB2PCENR_IOPAEN   = 1 <<  2, // PA port module clock enable bit for I/O.
	RCC_APB2PCENR_IOPCEN   = 1 <<  4, // PC port module clock enable bit for I/O.
	RCC_APB2PCENR_IOPDEN   = 1 <<  5, // PD port module clock enable bit for I/O.
	RCC_APB2PCENR_ADC1EN   = 1 <<  9, // ADC1 module clock enable bit.
	RCC_APB2PCENR_TIM1EN   = 1 << 11, // TIM1 module clock enable bit.
	RCC_APB2PCENR_SPI1EN   = 1 << 12, // SPI1 interface clock enable bit.
	RCC_APB2PCENR_USART1EN = 1 << 14  // USART1 interface clock enable bit.
} RCCAPB2PCENRFlag;

typedef enum {
	RCC_APB1PCENR_TIM2EN = 1 <<  0, // Timer 2 module clock enable bit.
	RCC_APB1PCENR_WWDGEN = 1 << 11, // Window watchdog clock enable bit.
	RCC_APB1PCENR_I2C1EN = 1 << 21, // I2C 1 interface clock enable bit.
	RCC_APB1PCENR_PWREN  = 1 << 28  // Power interface module clock enable bit.
} RCCAPB1PCENRFlag;

typedef enum {
	RCC_RSTSCKR_LSION    = 1 <<  0, // Internal low-speed clock (LSI) enable control bit.
	RCC_RSTSCKR_LSIRDY   = 1 <<  1, // Internal Low Speed Clock (LSI) Stable Ready flag bit (set by hardware).
	RCC_RSTSCKR_RMVF     = 1 << 24, // Clear reset flag control.
	RCC_RSTSCKR_PINRSTF  = 1 << 26, // External manual reset (NRST pin) flag.
	RCC_RSTSCKR_PORRSTF  = 1 << 27, // Power-up/power-down reset flag.
	RCC_RSTSCKR_SFTRSTF  = 1 << 28, // Software reset flag.
	RCC_RSTSCKR_IWDGRSTF = 1 << 29, // Independent watchdog reset flag.
	RCC_RSTSCKR_WWDGRSTF = 1 << 30, // Window watchdog reset flag
	RCC_RSTSCKR_LPWRRSTF = 1 << 31  // Low-power reset flag.
} RCCRSTSCKRFlag;

#define RCC_CTLR      _MMIO32(RCC_BASE | 0x00) // Clock control register
#define RCC_CFGR0     _MMIO32(RCC_BASE | 0x04) // Clock configuration register 0
#define RCC_INTR      _MMIO32(RCC_BASE | 0x08) // Clock interrupt register
#define RCC_APB2PRSTR _MMIO32(RCC_BASE | 0x0c) // PB2 peripheral reset register
#define RCC_APB1PRSTR _MMIO32(RCC_BASE | 0x10) // PB1 peripheral reset register
#define RCC_AHBPCENR  _MMIO32(RCC_BASE | 0x14) // HB peripheral clock enable register
#define RCC_APB2PCENR _MMIO32(RCC_BASE | 0x18) // PB2 peripheral clock enable register
#define RCC_APB1PCENR _MMIO32(RCC_BASE | 0x1c) // PB1 peripheral clock enable register
#define RCC_RSTSCKR   _MMIO32(RCC_BASE | 0x24) // Control/status register

/* Flash interface */

typedef enum {
	FLASH_KEY_RDPRT = 0x000000a5,
	FLASH_KEY1      = 0x45670123,
	FLASH_KEY2      = 0xcdef89ab
} FlashKey;

typedef enum {
	FLASH_ACTLR_LATENCY_BITMASK = 3 << 0, // Number of FLASH wait states
	FLASH_ACTLR_LATENCY_0       = 0 << 0, // 0 wait (recommended 0<=SYSCLK<=24MHz)
	FLASH_ACTLR_LATENCY_1       = 1 << 0, // 1 wait (recommended 24<=SYSCLK<=48MHz)
	FLASH_ACTLR_LATENCY_2       = 2 << 0
} FlashACTLRFlag;

typedef enum {
	FLASH_STATR_BUSY     = 1 <<  0, // Indicates busy status.
	FLASH_STATR_WRPRTERR = 1 <<  4, // Indicates a write protection error, write 1 clear. The hardware will set the address if it is programmed for write protection.
	FLASH_STATR_EOP      = 1 <<  5, // Indicates the end of the operation, and write 1 clears 0. The hardware is set each time it is successfully erased or programmed.
	FLASH_STATR_MODE     = 1 << 14, // Control the switch between user area and BOOT area
	FLASH_STATR_LOCK     = 1 << 15  // BOOT Lock
} FlashSTATRFlag;

typedef enum {
	FLASH_CTLR_PG      = 1 <<  0, // Performs standard programming operations.
	FLASH_CTLR_PER     = 1 <<  1, // Perform sector erase (1K)
	FLASH_CTLR_MER     = 1 <<  2, // Performs a full-erase operation (erases the entire user area).
	FLASH_CTLR_OBPG    = 1 <<  4, // Perform user-option bytes programming
	FLASH_CTLR_OBER    = 1 <<  5, // Perform user-option bytes erasure
	FLASH_CTLR_STRT    = 1 <<  6, // Start. Set 1 to start an erase action and the hardware automatically clears 0 (BSY becomes '0').
	FLASH_CTLR_LOCK    = 1 <<  7, // Lock. Only '1' can be written. When this bit is '1' it means that FPEC and FLASH_CTLR are locked and unwritable.
	FLASH_CTLR_OBWRE   = 1 <<  9, // User selects word lock, software clears 0.
	FLASH_CTLR_ERRIE   = 1 << 10, // Error status interrupt control (PGERR/WRPRTERR set in FLASH_STATR register).
	FLASH_CTLR_EOPIE   = 1 << 12, // Operation completion interrupt control (EOP set in FLASH_STATR register).
	FLASH_CTLR_FLOCK   = 1 << 15, // Fast programming lock. Write '1' only. When this bit is '1' it indicates that fast programming/erase mode is not available.
	FLASH_CTLR_FTPG    = 1 << 16, // Performs quick page programming operations.
	FLASH_CTLR_FTER    = 1 << 17, // Performs a fast page (64Byte) erase operation.
	FLASH_CTLR_BUFLOAD = 1 << 18, // Cache data into BUF
	FLASH_CTLR_BUFRST  = 1 << 19  // BUF reset operation
} FlashCTLRFlag;

typedef enum {
	FLASH_OBR_OBERR         =   1 <<  0, // Wrong choice of words.
	FLASH_OBR_RDPRT         =   1 <<  1, // Read protection status.
	FLASH_OBR_USER_BITMASK  = 255 <<  2,
	FLASH_OBR_DATA0_BITMASK = 255 << 10, // Data byte 0
	FLASH_OBR_DATA1_BITMASK = 255 << 18  // Data byte 1
} FlashOBRFlag;

#define FLASH_ACTLR         _MMIO32(FLASH_IF_BASE | 0x00) // Control register
#define FLASH_KEYR          _MMIO32(FLASH_IF_BASE | 0x04) // FPEC key register
#define FLASH_OBKEYR        _MMIO32(FLASH_IF_BASE | 0x08) // OBKEY register
#define FLASH_STATR         _MMIO32(FLASH_IF_BASE | 0x0c) // Status register
#define FLASH_CTLR          _MMIO32(FLASH_IF_BASE | 0x10) // Configuration register
#define FLASH_ADDR          _MMIO32(FLASH_IF_BASE | 0x14) // Address register
#define FLASH_OBR           _MMIO32(FLASH_IF_BASE | 0x1c) // Option byte register
#define FLASH_WPR           _MMIO32(FLASH_IF_BASE | 0x20) // Write protection register
#define FLASH_MODEKEYR      _MMIO32(FLASH_IF_BASE | 0x24) // Extended key register
#define FLASH_BOOT_MODEKEYR _MMIO32(FLASH_IF_BASE | 0x28) // Unlock BOOT key register

/* Extended configuration */

typedef enum {
	EXTEND_CTR_LKUPEN   = 1 <<  6, // LOCKUP monitoring function.
	EXTEND_CTR_LKUPRST  = 1 <<  7, // LOCKUP reset flag.
	EXTEND_CTR_LDOTRIM  = 1 << 10, // Core voltage modes.
	EXTEND_CTR_OPA_EN   = 1 << 16, // OPA Enable
	EXTEND_CTR_OPA_NSEL = 1 << 17, // OPA negative end channel selection
	EXTEND_CTR_OPA_PSEL = 1 << 18  // OPA positive end channel selection
} ExtendCTRFlag;

#define EXTEND_CTR _MMIO32(EXTEND_BASE | 0x0) // Configure extended control registers

/* PFIC */

typedef enum {
	EXC_FETCH_ADDR_ERROR =  0, // Instruction address misalignment
	EXC_FETCH_BUS_ERROR  =  1, // Fetch command access error
	EXC_RESERVED_INST    =  2, // Illegal instructions
	EXC_BREAK            =  3, // Breakpoints
	EXC_LOAD_ADDR_ERROR  =  4, // Load instruction access address misalignment
	EXC_LOAD_BUS_ERROR   =  5, // Load command access error
	EXC_STORE_ADDR_ERROR =  6, // Store/AMO instruction access address misalignment
	EXC_STORE_BUS_ERROR  =  7, // Store/AMO command access error
	EXC_ECALL_USER       =  8, // Environment call in User mode
	EXC_ECALL_MACHINE    = 11  // Environment call in Machine mode
} ExceptionChannel;

typedef enum {
	IRQ_NMI       =  2, // Non-maskable interrupts
	IRQ_HARDFAULT =  3, // Abnormal interruptions
	IRQ_SYSTICK   = 12, // System timer interrupt
	IRQ_SW        = 14, // Software interrupt
	IRQ_WWDG      = 16, // Window timer interrupt
	IRQ_PVD       = 17, // Supply voltage detection interrupt (EXTI)
	IRQ_FLASH     = 18, // Flash global interrupt
	IRQ_RCC       = 19, // Reset and clock control interrupts
	IRQ_EXTI      = 20, // EXTI line 0-7 interrupt
	IRQ_AWU       = 21, // Wake-up interrupt
	IRQ_DMA1_CH1  = 22, // DMA1 channel 1 global interrupt
	IRQ_DMA1_CH2  = 23, // DMA1 channel 2 global interrupt
	IRQ_DMA1_CH3  = 24, // DMA1 channel 3 global interrupt
	IRQ_DMA1_CH4  = 25, // DMA1 channel 4 global interrupt
	IRQ_DMA1_CH5  = 26, // DMA1 channel 5 global interrupt
	IRQ_DMA1_CH6  = 27, // DMA1 channel 6 global interrupt
	IRQ_DMA1_CH7  = 28, // DMA1 channel 7 global interrupt
	IRQ_ADC       = 29, // ADC global interrupt
	IRQ_I2C1_EV   = 30, // I2C1 event interrupt
	IRQ_I2C1_ER   = 31, // I2C1 error interrupt
	IRQ_USART1    = 32, // USART1 global interrupt
	IRQ_SPI1      = 33, // SPI1 global interrupt
	IRQ_TIM1BRK   = 34, // TIM1 brake interrupt
	IRQ_TIM1UP    = 35, // TIM1 update interrupt
	IRQ_TIM1TRG   = 36, // TIM1 trigger interrupt
	IRQ_TIM1CC    = 37, // TIM1 capture/compare interrupt
	IRQ_TIM2      = 38  // TIM2 global interrupt
} IRQChannel;

typedef enum {
	PFIC_CFGR_RESETSYS        =      1 <<  7, // System reset (simultaneous writing to KEY3). Auto clear 0.
	PFIC_CFGR_KEYCODE_BITMASK = 0xffff << 16,
	PFIC_CFGR_KEYCODE_KEY1    = 0xfa05 << 16,
	PFIC_CFGR_KEYCODE_KEY2    = 0xbcaf << 16,
	PFIC_CFGR_KEYCODE_KEY3    = 0xbeef << 16
} PFICCFGRFlag;

typedef enum {
	PFIC_GISR_NESTSTA_BITMASK = 255 << 0, // Current interrupt nesting status, currently supports a maximum of 2 levels of nesting and a maximum hardware stack depth of 2 levels.
	PFIC_GISR_NESTSTA_1       =   1 << 0, // Level 1 interrupt in progress.
	PFIC_GISR_NESTSTA_2       =   3 << 0, // Level 2 interrupt in progress.
	PFIC_GISR_GACTSTA         =   1 << 8, // Are there any interrupts currently being executed
	PFIC_GISR_GPENDSTA        =   1 << 9  // Are there any interrupts currently on hold.
} PFICGISRFlag;

typedef enum {
	PFIC_IPRIOR_PRI_BITMASK = 1 << 6,
	PFIC_IPRIOR_PRI_HIGH    = 0 << 6,
	PFIC_IPRIOR_PRI_LOW     = 1 << 6,
	PFIC_IPRIOR_PREEMPT     = 1 << 7
} PFICIPRIORFlag;

typedef enum {
	PFIC_SCTLR_SLEEPONEXIT = 1 <<  1, // System status after control leaves the interrupt service program.
	PFIC_SCTLR_SLEEPDEEP   = 1 <<  2, // Low-power mode of the control system.
	PFIC_SCTLR_WFITOWFE    = 1 <<  3, // Execute the WFI command as if it were a WFE.
	PFIC_SCTLR_SEVONPEND   = 1 <<  4, // Enabled events and all interrupts (including unenabled interrupts) can wake up the system.
	PFIC_SCTLR_SETEVENT    = 1 <<  5, // Set the event to wake up the WFE case.
	PFIC_SCTLR_SYSRESET    = 1 << 31  // System reset, clear 0 automatically. write 1 valid, write 0 invalid, same effect as PFIC_CFGR register.
} PFICSCTLRFlag;

#define PFIC_ISR(N)      _MMIO32((PFIC_BASE | 0x000) + (4 * (N))) // PFIC interrupt enable status register
#define PFIC_IPR(N)      _MMIO32((PFIC_BASE | 0x020) + (4 * (N))) // PFIC interrupt pending status register
#define PFIC_ITHRESDR    _MMIO32(PFIC_BASE | 0x040) // PFIC interrupt priority threshold configuration register
#define PFIC_CFGR        _MMIO32(PFIC_BASE | 0x048) // PFIC interrupt configuration register
#define PFIC_GISR        _MMIO32(PFIC_BASE | 0x04c) // PFIC interrupt global status register
#define PFIC_VTFIDR      _MMIO32(PFIC_BASE | 0x050) // PFIC VTF interrupt ID configuration register
#define PFIC_VTFADDRR(N) _MMIO32((PFIC_BASE | 0x060) + (4 * (N))) // PFIC VTF interrupt offset address register
#define PFIC_IENR(N)     _MMIO32((PFIC_BASE | 0x100) + (4 * (N))) // PFIC interrupt enable setting register
#define PFIC_IRER(N)     _MMIO32((PFIC_BASE | 0x180) + (4 * (N))) // PFIC interrupt enable clear register
#define PFIC_IPSR(N)     _MMIO32((PFIC_BASE | 0x200) + (4 * (N))) // PFIC interrupt pending setting register
#define PFIC_IPRR(N)     _MMIO32((PFIC_BASE | 0x280) + (4 * (N))) // PFIC interrupt pending clear register
#define PFIC_IACTR(N)    _MMIO32((PFIC_BASE | 0x300) + (4 * (N))) // PFIC interrupt activation status register
#define PFIC_IPRIOR(N)   _MMIO8((PFIC_BASE | 0x400) + (N)) // PFIC interrupt priority configuration register
#define PFIC_SCTLR       _MMIO32(PFIC_BASE | 0xd10) // PFIC system control register

/* SysTick timer */

typedef enum {
	STK_CTLR_STE           = 1 <<  0, // System counter enable control bit.
	STK_CTLR_STIE          = 1 <<  1, // Counter interrupt enable control bit.
	STK_CTLR_STCLK_BITMASK = 1 <<  2, // Counter clock source selection bit.
	STK_CTLR_STCLK_DIV8    = 0 <<  2, // HCLK/8 for time base.
	STK_CTLR_STCLK_DIV1    = 1 <<  2, // HCLK for time base.
	STK_CTLR_STRE          = 1 <<  3, // Auto-reload count enable bit.
	STK_CTLR_SWIE          = 1 << 31  // Software interrupt trigger enable (SWI).
} STKCTLRFlag;

typedef enum {
	STK_SR_CNTIF = 1 << 0 // Count value comparison flag, write 0 to clear, write 1 to invalidate.
} STKSRFlag;

#define STK_CTLR  _MMIO32(SYSTICK_BASE | 0x00) // System count control register
#define STK_SR    _MMIO32(SYSTICK_BASE | 0x04) // System count status register
#define STK_CNTL  _MMIO32(SYSTICK_BASE | 0x08) // System counter register
#define STK_CMPLR _MMIO32(SYSTICK_BASE | 0x10) // Counting comparison register
