/**
 * Copyright (c) 2012 the MansOS team. All rights reserved.
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
 * msp430_usci.h -- USCI module (UART/SPI/I2C modes) on MSP430FR57xx
 */

#ifndef _MSP430FR57XX_USCI_H_
#define _MSP430FR57XX_USCI_H_

#include <kernel/defines.h>

#define USCI_A0_RXTX_PORT 2
#define USCI_A0_TX_PIN    0
#define USCI_A0_RX_PIN    1
#define USCI_A0_SIMO_PIN  USCI_A0_TX_PIN
#define USCI_A0_SOMI_PIN  USCI_A0_RX_PIN
#define USCI_A0_CLK_PORT  1
#define USCI_A0_CLK_PIN   5
#define USCI_A0_STE_PORT  1
#define USCI_A0_STE_PIN   4

#define USCI_A1_RXTX_PORT 2
#define USCI_A1_TX_PIN    5
#define USCI_A1_RX_PIN    6
#define USCI_A1_SIMO_PIN  USCI_A1_TX_PIN
#define USCI_A1_SOMI_PIN  USCI_A1_RX_PIN
#define USCI_A1_CLK_PORT  2
#define USCI_A1_CLK_PIN   4
#define USCI_A1_STE_PORT  2
#define USCI_A1_STE_PIN   3


#define USCI_B0_RXTX_PORT 1
#define USCI_B0_SIMO_PIN  6
#define USCI_B0_SOMI_PIN  7
#define USCI_B0_CLK_PORT  2
#define USCI_B0_CLK_PIN   2
#define USCI_B0_STE_PORT  1
#define USCI_B0_STE_PIN   3

#define USCI_B0_I2C_PORT  1
#define USCI_B0_SDA_PIN   6
#define USCI_B0_SCL_PIN   7


#define SERIAL_COUNT      2
#define PRINTF_SERIAL_ID  1


#define UC0IE     UCA0IE
#define UC0IFG    UCA0IFG
#define UCA0RXIE  UCRXIE
#define UCA0TXIE  UCTXIE
#define UCA0RXIFG UCRXIFG
#define UCA0TXIFG UCTXIFG
#define UCB0RXIE  UCRXIE
#define UCB0TXIE  UCTXIE
#define UCB0RXIFG UCRXIFG
#define UCB0TXIFG UCTXIFG

// This module enables and disables components automatically
static inline void serialEnableTX(uint8_t id) { }
static inline void serialDisableTX(uint8_t id) { }
static inline void serialEnableRX(uint8_t id) {
    // Enable receive interrupt
#ifdef UART_ON_USCI_A0
    UC0IE |= UCA0RXIE;
#else
    UC1IE |= UCA1RXIE;
#endif
}
static inline void serialDisableRX(uint8_t id) {
    // Disable receive interrupt
#ifdef UART_ON_USCI_A0
    UC0IE &= ~UCA0RXIE;
#else
    UC1IE &= ~UCA1RXIE;
#endif
}
static inline void hw_spiBusOn(uint8_t busId) { }
static inline void hw_spiBusOff(uint8_t busId) { }



// Send data
static inline void serialSendByte(uint8_t id, uint8_t data)
{
    while (!(UC0IFG & UCA0TXIFG));
    UCA0TXBUF = data;
}


#endif
