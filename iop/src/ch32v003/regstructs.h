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

#define REG16 volatile uint16_t
#define REG32 volatile uint32_t

/* Timer 2 */

typedef struct {
	REG16 CTLR1;     // TIM2 control register1
	REG16 _pad;
	REG16 CTLR2;     // TIM2 control register2
	REG16 _pad2;
	REG16 SMCFGR;    // TIM2 Slave mode control register
	REG16 _pad3;
	REG16 DMAINTENR; // TIM2 DMA/interrupt enable register
	REG16 _pad4;
	REG16 INTFR;     // TIM2 interrupt status register
	REG16 _pad5;
	REG16 SWEVGR;    // TIM2 event generation register
	REG16 _pad6;
	REG16 CHCTLR1;   // TIM2 compare/capture control register1
	REG16 _pad7;
	REG16 CHCTLR2;   // TIM2 compare/capture control register2
	REG16 _pad8;
	REG16 CCER;      // TIM2 compare/capture enable register
	REG16 _pad9;
	REG16 CNT;       // TIM2 counter
	REG16 _pad10;
	REG16 PSC;       // TIM2 count clock prescaler
	REG16 _pad11;
	REG16 ATRLR;     // TIM2 auto-reload register
	REG32 _pad12;
	REG32 CHCVR[4];  // TIM2 compare/capture register
	REG32 _pad13;
	REG16 DMACFGR;   // TIM2 DMA control register
	REG16 _pad14;
	REG16 DMAADR;    // TIM2 DMA address register in continuous mode
} TIM2Registers;

/* Window watchdog */

typedef struct {
	REG16 CTLR;  // Control register
	REG16 _pad;
	REG16 CFGR;  // Configuration Register
	REG16 _pad2;
	REG16 STATR; // Status Register
} WWDGRegisters;

/* Independent watchdog */

typedef struct {
	REG16 CTLR;  // Control register
	REG16 _pad;
	REG16 PSCR;  // Prescaler register
	REG16 _pad2;
	REG16 RLDR;  // Reload register
	REG16 _pad3;
	REG16 STATR; // Status register
} IWDGRegisters;

/* I2C */

typedef struct {
	REG16 CTLR1;  // I2C control register 1
	REG16 _pad;
	REG16 CTLR2;  // I2C control register 2
	REG16 _pad2;
	REG16 OADDR1; // I2C address register 1
	REG16 _pad3;
	REG16 OADDR2; // I2C address register 2
	REG16 _pad4;
	REG16 DATAR;  // I2C data register
	REG16 _pad5;
	REG16 STAR1;  // I2C status register 1
	REG16 _pad6;
	REG16 STAR2;  // I2C status register 2
	REG16 _pad7;
	REG16 CKCFGR; // I2C clock register
} I2CRegisters;

/* Power control */

typedef struct {
	REG32 CTLR;   // Power control register
	REG32 CSR;    // Power control/status register
	REG32 AWUCSR; // Auto-wakeup control/status register
	REG32 AWUWR;  // Auto-wakeup window comparison value register
	REG32 AWUPSC; // Auto-wakeup crossover factor register
} PWRRegisters;

/* Alternate-function I/O */

typedef struct {
	REG32 _pad;
	REG32 PCFR1;  // Remap Register 1
	REG32 EXTICR; // External interrupt configuration register 1
} AFIORegisters;

/* External interrupt controller */

typedef struct {
	REG32 INTENR; // Interrupt enable register
	REG32 EVENR;  // Event enable register
	REG32 RTENR;  // Rising edge trigger enable register
	REG32 FTENR;  // Falling edge trigger enable register
	REG32 SWIEVR; // Soft interrupt event register
	REG32 INTFR;  // Interrupt flag register
} EXTIRegisters;

/* GPIO */

typedef struct {
	REG32 CFGLR; // Port configuration register low
	REG32 _pad;
	REG32 INDR;  // Port input data register
	REG32 OUTDR; // Port output data register
	REG32 BSHR;  // Port set/reset register
	REG32 BCR;   // Port reset register
	REG32 LCKR;  // Port configuration lock register
} GPIORegisters;

/* ADC */

typedef struct {
	REG32 STATR;     // ADC status register
	REG32 CTLR1;     // ADC control register 1
	REG32 CTLR2;     // ADC control register 2
	REG32 SAMPTR1;   // ADC sample time register 1
	REG32 SAMPTR2;   // ADC sample time register 2
	REG32 IOFR[4];   // ADC injected channel data offset register
	REG32 WDHTR;     // ADC watchdog high threshold register
	REG32 WDLTR;     // ADC watchdog low threshold register
	REG32 RSQR1;     // ADC regular sequence register 1
	REG32 RSQR2;     // ADC regular sequence register 2
	REG32 RSQR3;     // ADC regular sequence register 3
	REG32 ISQR;      // ADC injected sequence register
	REG32 IDATAR[4]; // ADC injected data register
	REG32 RDATAR;    // ADC regular data register
	REG32 DLYR;      // ADC delayed data register
} ADCRegisters;

/* Timer 1 */

typedef struct {
	REG16 CTLR1;     // Control register 1
	REG16 _pad;
	REG16 CTLR2;     // Control register 2
	REG16 _pad2;
	REG16 SMCFGR;    // Slave mode control register
	REG16 _pad3;
	REG16 DMAINTENR; // DMA/interrupt enable register
	REG16 _pad4;
	REG16 INTFR;     // Interrupt status register
	REG16 _pad5;
	REG16 SWEVGR;    // Event generation register
	REG16 _pad6;
	REG16 CHCTLR1;   // Compare/capture control register 1
	REG16 _pad7;
	REG16 CHCTLR2;   // Compare/capture control register 2
	REG16 _pad8;
	REG16 CCER;      // Compare/capture enable register
	REG16 _pad9;
	REG16 CNT;       // Counters
	REG16 _pad10;
	REG16 PSC;       // Counting clock prescaler
	REG16 _pad11;
	REG16 ATRLR;     // Auto-reload value register
	REG16 _pad12;
	REG16 RPTCR;     // Recurring count value register
	REG16 _pad13;
	REG32 CHCVR[4];
	REG16 BDTR;      // Brake and deadband registers
	REG16 _pad14;
	REG16 DMACFGR;   // DMA control register
	REG16 _pad15;
	REG16 DMAADR;    // DMA address register for continuous mode
} TIM1Registers;

/* SPI */

typedef struct {
	REG16 CTLR1;    // SPI Control register 1
	REG16 _pad;
	REG16 CTLR2;    // SPI Control register 2
	REG16 _pad2;
	REG16 STATR;    // SPI Status register
	REG16 _pad3;
	REG16 DATAR;    // SPI Data register
	REG16 _pad4;
	REG16 CRCR;     // SPI Polynomial register
	REG16 _pad5;
	REG16 RCRCR;    // SPI Receive CRC register
	REG16 _pad6;
	REG16 TCRCR;    // SPI Transmit CRC register
	REG16 _pad7[5];
	REG16 HSCR;     // SPI High-speed control register
} SPIRegisters;

/* USART */

typedef struct {
	REG32 STATR; // UASRT status register
	REG32 DATAR; // UASRT data register
	REG32 BRR;   // UASRT baud rate register
	REG32 CTLR1; // UASRT control register 1
	REG32 CTLR2; // UASRT control register 2
	REG32 CTLR3; // UASRT control register 3
	REG32 GPR;   // UASRT protection time and prescaler register
} USARTRegisters;

/* DMA */

typedef struct {
	REG32 CFGR;  // DMA channel configuration register
	REG32 CNTR;  // DMA channel number of data register
	REG32 PADDR; // DMA channel peripheral address register
	REG32 MADDR; // DMA channel memory address register
	REG32 _pad;
} DMAChannelRegisters;

typedef struct {
	REG32 INTFR;  // DMA interrupt status register
	REG32 INTFCR; // DMA interrupt flag clear register

	DMAChannelRegisters CH[7];
} DMARegisters;

/* Reset and clock control */

typedef struct {
	REG32 CTLR;      // Clock control register
	REG32 CFGR0;     // Clock configuration register 0
	REG32 INTR;      // Clock interrupt register
	REG32 APB2PRSTR; // PB2 peripheral reset register
	REG32 APB1PRSTR; // PB1 peripheral reset register
	REG32 AHBPCENR;  // HB peripheral clock enable register
	REG32 APB2PCENR; // PB2 peripheral clock enable register
	REG32 APB1PCENR; // PB1 peripheral clock enable register
	REG32 _pad;
	REG32 RSTSCKR;   // Control/status register
} RCCRegisters;

/* Flash interface */

typedef struct {
	REG32 ACTLR;         // Control register
	REG32 KEYR;          // FPEC key register
	REG32 OBKEYR;        // OBKEY register
	REG32 STATR;         // Status register
	REG32 CTLR;          // Configuration register
	REG32 ADDR;          // Address register
	REG32 _pad;
	REG32 OBR;           // Option byte register
	REG32 WPR;           // Write protection register
	REG32 MODEKEYR;      // Extended key register
	REG32 BOOT_MODEKEYR; // Unlock BOOT key register
} FlashIFRegisters;

/* PFIC */

typedef struct {
	REG32 ISR[8];      // PFIC interrupt enable status register
	REG32 IPR[8];      // PFIC interrupt pending status register
	REG32 ITHRESDR;    // PFIC interrupt priority threshold configuration register
	REG32 _pad;
	REG32 CFGR;        // PFIC interrupt configuration register
	REG32 GISR;        // PFIC interrupt global status register
	REG32 VTFIDR;      // PFIC VTF interrupt ID configuration register
	REG32 _pad2[3];
	REG32 VTFADDRR[2]; // PFIC VTF interrupt offset address register
	REG32 _pad3[38];
	REG32 IENR[8];     // PFIC interrupt enable setting register
	REG32 _pad4[24];
	REG32 IRER[8];     // PFIC interrupt enable clear register
	REG32 _pad5[24];
	REG32 IPSR[8];     // PFIC interrupt pending setting register
	REG32 _pad6[24];
	REG32 IPRR[8];     // PFIC interrupt pending clear register
	REG32 _pad7[24];
	REG32 IACTR[8];    // PFIC interrupt activation status register
	REG32 _pad8[56];
	REG32 IPRIOR[256]; // PFIC interrupt priority configuration register
	REG32 _pad9[324];
	REG32 SCTLR;       // PFIC system control register
} PFICRegisters;

/* SysTick timer */

typedef struct {
	REG32 CTLR;  // System count control register
	REG32 SR;    // System count status register
	REG32 CNTL;  // System counter register
	REG32 _pad;
	REG32 CMPLR; // Counting comparison register
} SysTickRegisters;

#undef REG16
#undef REG32
