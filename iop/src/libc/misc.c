/*
 * ps1-bare-metal - (C) 2023 spicyjpeg
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

#include <stdbool.h>
#include <stdio.h>
#include "ch32v003/registers.h"

/* Serial port stdin/stdout */

enum {
	PIN_UART_TX = 5,
	PIN_UART_RX = 6
};

void initSerialIO(int hclk, int baud) {
	// Enable the clock to the UART and reset it.
	RCC_APB2PCENR |= RCC_APB2PCENR_USART1EN;

	uint32_t prstr = RCC_APB2PRSTR;
	RCC_APB2PRSTR  = prstr |  RCC_APB2PRSTR_USART1RST;
	RCC_APB2PRSTR  = prstr & ~RCC_APB2PRSTR_USART1RST;

	// Configure and enable the UART.
	// NOTE: according to the datasheet USART_BRR is supposed to be in 12.4
	// fixed-point format, however that does not seem to be the case.
	const uint32_t ctlr1 = 0
		| USART_CTLR1_RE
		| USART_CTLR1_TE
		| USART_CTLR1_M_8;

	USART_CTLR1 = ctlr1;
	USART_CTLR2 = USART_CTLR2_STOP_1;
	USART_CTLR3 = 0;
	USART_BRR   = (hclk + baud / 2) / baud;
	USART_CTLR1 = ctlr1 | USART_CTLR1_UE;
}

void routeSerialIO(bool tx, bool rx) {
	// Ensure the clock to GPIO port D is enabled and clear any previously
	// configured alternate mapping.
	RCC_APB2PCENR |= 0
		| RCC_APB2PCENR_AFIOEN
		| RCC_APB2PCENR_IOPDEN;

	AFIO_PCFR1 = (AFIO_PCFR1 & ~AFIO_PCFR1_USART1_RM_BITMASK)
		| AFIO_PCFR1_USART1_RM_DEFAULT;

	const uint32_t mask   = 0
		| GPIO_CFGLR_MODE_BITMASK
		| GPIO_CFGLR_CNF_BITMASK;
	const uint32_t txMask = 0
		| GPIO_CFGLR_MODE_OUTPUT_10MHZ
		| GPIO_CFGLR_CNF_OUT_AF_PUSH_PULL;
	const uint32_t rxMask = 0
		| GPIO_CFGLR_MODE_INPUT
		| GPIO_CFGLR_CNF_IN_PULL;

	const int txShift = PIN_UART_TX * 4;
	const int rxShift = PIN_UART_RX * 4;

	if (tx) {
		GPIOD_CFGLR = (GPIOD_CFGLR & ~(mask << txShift)) | (txMask << txShift);
	}
	if (rx) {
		GPIOD_CFGLR = (GPIOD_CFGLR & ~(mask << rxShift)) | (rxMask << rxShift);
		GPIOD_BSHR  = 1 << PIN_UART_RX;
	}
}

void _putchar(char ch) {
	while (!(USART_STATR & USART_STATR_TXE))
		__asm__ volatile("");

	USART_DATAR = ch;
}

int _getchar(void) {
	while (!(USART_STATR & USART_STATR_RXNE))
		__asm__ volatile("");

	return USART_DATAR & 0xff;
}

int _puts(const char *str) {
	int length = 1;

	for (; *str; str++, length++)
		_putchar(*str);

	_putchar('\n');
	return length;
}

/* Abort functions */

void _assertAbort(const char *file, int line, const char *expr) {
#ifndef NDEBUG
	printf("%s:%d: assert(%s)\n", file, line, expr);
#endif

	for (;;)
		__asm__ volatile("");
}

void abort(void) {
#ifndef NDEBUG
	puts("abort()");
#endif

	for (;;)
		__asm__ volatile("");
}

void __cxa_pure_virtual(void) {
#ifndef NDEBUG
	puts("__cxa_pure_virtual()");
#endif

	for (;;)
		__asm__ volatile("");
}
