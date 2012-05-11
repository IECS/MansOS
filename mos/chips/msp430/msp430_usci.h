/**
 * Copyright (c) 2011, Institute of Electronics and Computer Science
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of  conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * msp430_usci.h -- USCI module (UART/SPI/I2C modes) on MSP430 x2xx
 *
 * There are two modules with a similar set of registers. Module A supports
 * UART and SPI modes, module B supports SPI and I2C modes. To reduce code
 * duplication we use name-composing macros for register manipulation. This
 * may increase code size, but at the same time it also improves execution
 * speed when compared to run-time register selection.
 */

#ifndef _MSP430_USCI_H_
#define _MSP430_USCI_H_

#include <kernel/defines.h>

// The port for all pins is port 3
#define USCI_PORT 3

#define UCA0TX_PIN 4
#define UCA0RX_PIN 5

#define UCA0SIMO_PIN 4
#define UCA0SOMI_PIN 5
#define UCA0CLK_PIN 0
#define UCA0STE_PIN 3

#define UCB0SIMO_PIN 1
#define UCB0SOMI_PIN 2
#define UCB0CLK_PIN 3
#define UCB0STE_PIN 0

#define UCB0SDA_PIN 1
#define UCB0SCL_PIN 2

#define PRINTF_USART_ID 0
#define USART_COUNT 1

#if defined(__msp430x54xA) || defined __IAR_SYSTEMS_ICC__
# define UC0IE     UCA0IE
# define UC0IFG    UCA0IFG
# define UCA0RXIE  UCRXIE
# define UCA0TXIE  UCTXIE
# define UCA0RXIFG UCRXIFG
# define UCA0TXIFG UCTXIFG
# define UCB0RXIE  UCRXIE
# define UCB0TXIE  UCTXIE
# define UCB0RXIFG UCRXIFG
# define UCB0TXIFG UCTXIFG
#endif

// This module enables and disables components automatically
static inline uint_t USARTEnableTX(uint8_t id) { return 0; }
static inline uint_t USARTDisableTX(uint8_t id) { return 0; }
static inline uint_t USARTEnableRX(uint8_t id) {
    UC0IE |= UCA0RXIE;     // Enable receive interrupt
    return 0;
}
static inline uint_t USARTDisableRX(uint8_t id) {
    UC0IE &= ~UCA0RXIE;    // Disable receive interrupt
    return 0;
}
static inline void hw_spiBusOn(uint8_t busId) { }
static inline void hw_spiBusOff(uint8_t busId) { }

#endif // _MSP430_USCI_H_
