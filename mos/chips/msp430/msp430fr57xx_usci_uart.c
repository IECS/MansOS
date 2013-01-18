/*
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

#include <serial.h>
#include "msp430fr57xx_usci.h"

SerialCallback_t serialRecvCb[SERIAL_COUNT];

//
// Initialization
//

uint_t serialInit(uint8_t id, uint32_t speed, uint8_t conf)
{
    //
    // Default setting is: 8 data bits, 1 stop bit, no parity bit, LSB first
    //
#define UART_MODE 0

    (void)conf;
    (void)id;

    // XXX: ugly!
    P2SEL1 |= BV(USCI_A0_TX_PIN) + BV(USCI_A0_RX_PIN);
    P2SEL0 &= ~(BV(USCI_A0_TX_PIN) + BV(USCI_A0_RX_PIN));

    UCA0CTL1  = UCSWRST;   // Hold the module in reset state
    UCA0CTL1 |= UCSSEL_2;  // SMCLK clock source
    if (speed <= CPU_HZ / 16) {
        // Use oversampling mode
        UCA0BR0  = (CPU_HZ / (speed * 16)) & 0xFF;
        UCA0BR1  = (CPU_HZ / (speed * 16)) >> 8;
        UCA0MCTLW = UCOS16 | ((CPU_HZ / speed) & 0xF) << 4;
    }
    else {        // Use normal mode with fractional clock divider
        UCA0BR0  = (CPU_HZ / speed) & 0xFF;
        UCA0BR1  = (CPU_HZ / speed) >> 8;
        UCA0MCTLW = ((CPU_HZ * 8 / speed) & 0x7) << 1;
    }
    UCA0CTL0 = UART_MODE;
    UC0IE |= UCA0RXIE;     // Enable receive interrupt
    UCA0CTL1 &= ~UCSWRST;  // Release hold

    return 0;
}

// UART mode receive handler
#if UART_ON_USCI_A0

ISR(USCI_A0, USCIAInterruptHandler)
{
    bool error = UCA0STATW & UCRXERR;
    uint8_t data = UCA0RXBUF;
    if (error || UCA0STATW & UCOE) {
        // There was an error or a register overflow; clear UCOE and exit
        data = UCA0RXBUF;
        return;
    }

    if (serialRecvCb[0]) {
        serialRecvCb[0](data);
    }
}

#else // !UART_ON_USCI_A0

ISR(USCI_A1, USCIAInterruptHandler)
{
    bool error = UCA1STATW & UCRXERR;
    uint8_t data = UCA1RXBUF;
    if (error || UCA1STATW & UCOE) {
        // There was an error or a register overflow; clear UCOE and exit
        data = UCA1RXBUF;
        return;
    }

    if (serialRecvCb[0]) {
        serialRecvCb[0](data);
    }
}

#endif // UART_ON_USCI_A0
