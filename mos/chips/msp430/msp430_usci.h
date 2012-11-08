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
 */

/**
 * msp430_usci.h -- USCI module (UART/SPI/I2C modes) on MSP430
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

// TODO: clean up this file

#define USCI_A0_RXTX_PORT 3
#define USCI_A0_TX_PIN    4
#define USCI_A0_RX_PIN    5
#define USCI_A0_SIMO_PIN  USCI_A0_TX_PIN
#define USCI_A0_SOMI_PIN  USCI_A0_RX_PIN
#define USCI_A0_CLK_PORT  3
#define USCI_A0_CLK_PIN   0
// #define USCI_A0_STE_PORT  3
// #define USCI_A0_STE_PIN   3

#define USCI_A1_RXTX_PORT 3
#define USCI_A1_TX_PIN    6
#define USCI_A1_RX_PIN    7
#define USCI_A1_SIMO_PIN  USCI_A1_TX_PIN
#define USCI_A1_SOMI_PIN  USCI_A1_RX_PIN
#define USCI_A1_CLK_PORT  5
#define USCI_A1_CLK_PIN   0
// #define USCI_A1_STE_PORT  5
// #define USCI_A1_STE_PIN   3

#define USCI_B0_RXTX_PORT 3
#define USCI_B0_SIMO_PIN  1
#define USCI_B0_SOMI_PIN  2
#define USCI_B0_CLK_PORT  3
#define USCI_B0_CLK_PIN   3
// #define USCI_B0_STE_PORT  3
// #define USCI_B0_STE_PIN   0

#define USCI_B0_I2C_PORT  3
#define USCI_B0_SDA_PIN   1
#define USCI_B0_SCL_PIN   2

#define USCI_B1_RXTX_PORT 5
#define USCI_B1_SIMO_PIN  1
#define USCI_B1_SOMI_PIN  2
#define USCI_B1_CLK_PORT  5
#define USCI_B1_CLK_PIN   3
// #define USCI_B1_STE_PORT  5
// #define USCI_B1_STE_PIN   0

#define USCI_B1_I2C_PORT  5
#define USCI_B1_SDA_PIN   1
#define USCI_B1_SCL_PIN   2


#if PLATFORM_XM1000
#define SERIAL_COUNT       2
#define PRINTF_SERIAL_ID   1
#define UART_ON_USCI_A1    1
#elif PLATFORM_Z1 || PLATFORM_TESTBED || PLATFORM_TESTBED2
#define SERIAL_COUNT       2
#define PRINTF_SERIAL_ID   0
#define UART_ON_USCI_A0    1
#else
#define SERIAL_COUNT       1
#define PRINTF_SERIAL_ID   0
#define UART_ON_USCI_A0    1
#endif


// This module enables and disables components automatically
static inline void serialEnableTX(uint8_t id) {  }
static inline void serialDisableTX(uint8_t id) {  }


// Enable receive interrupt
#if PLATFORM_Z1 || PLATFORM_TESTBED || PLATFORM_TESTBED2
//
// Zolertia & TestBed specific code
//
static inline void serialEnableRX(uint8_t id) {
    if (id == 0) UC0IE |= UCA0RXIE;
    else UC1IE |= UCA1RXIE;
}
// Disable receive interrupt
static inline void serialDisableRX(uint8_t id) {
    if (id == 0) UC0IE &= ~UCA0RXIE;
    else UC1IE &= ~UCA1RXIE;
}

// #elif PLATFORM_TESTBED2
// //
// // TestBed specific code
// //
// static inline void serialEnableRX(uint8_t id) {
//     if (id == 0) UC0IE |= UCA0RXIE;
//     if (id == 1) UC0IE |= UCB0RXIE;
//     if (id == 2) UC1IE |= UCA1RXIE;
//     else UC1IE |= UCB1RXIE;
// }
// // Disable receive interrupt
// static inline void serialDisableRX(uint8_t id) {
//     if (id == 0) UC0IE &= ~UCA0RXIE;
//     else if (id == 1) UC0IE &= ~UCB0RXIE;
//     else if (id == 2) UC1IE &= ~UCA1RXIE;
//     else UC1IE &= ~UCB1RXIE;
// }

#else
//
// For other motes
//
static inline void serialEnableRX(uint8_t id) {
#if UART_ON_USCI_A0
    UC0IE |= UCA0RXIE;
#elif UART_ON_USCI_A1
    UC1IE |= UCA1RXIE;
#endif
}
// Disable receive interrupt
static inline void serialDisableRX(uint8_t id) {
#if UART_ON_USCI_A0
    UC0IE &= ~UCA0RXIE;
#elif UART_ON_USCI_A1
    UC1IE &= ~UCA1RXIE;
#endif
}
#endif

static inline void hw_spiBusOn(uint8_t busId) { }
static inline void hw_spiBusOff(uint8_t busId) { }

#endif
