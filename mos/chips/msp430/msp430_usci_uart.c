/*
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
 * msp430_usci_uart.c -- USCI module on MSP430, UART mode
 */

#include <digital.h>
#include <serial.h>
#include <kernel/stack.h>

#include "msp430_usci.h"

// --------------------------------------------------

SerialCallback_t serialRecvCb[SERIAL_COUNT];

volatile Serial_t serial[SERIAL_COUNT];

// --------------------------------------------------

//
// Default setting is: 8 data bits, 1 stop bit, no parity bit, LSB first
//
#define UART_MODE 0

void msp430UsciSerialInit0(uint32_t speed)
{
    pinAsFunction(USCI_A0_RXTX_PORT, USCI_A0_TX_PIN);
    pinAsFunction(USCI_A0_RXTX_PORT, USCI_A0_RX_PIN);

    UCA0CTL1  = UCSWRST;   // Hold the module in reset state
    UCA0CTL1 |= UCSSEL_2;  // SMCLK clock source
    if (0 /* speed <= CPU_HZ / 16 */) { // FIXME
        // Use oversampling mode
        UCA0BR0  = (CPU_HZ / (speed * 16)) & 0xFF;
        UCA0BR1  = (CPU_HZ / (speed * 16)) >> 8;
        UCA0MCTL = UCOS16 | ((CPU_HZ / speed) & 0xF) << 4;
    }
    else {        // Use normal mode with fractional clock divider
        UCA0BR0  = (CPU_HZ / speed) & 0xFF;
        UCA0BR1  = (CPU_HZ / speed) >> 8;
        UCA0MCTL = ((CPU_HZ * 8 / speed) & 0x7) << 1;
    }
    UCA0CTL0 = UART_MODE;
    UC0IE |= UCA0RXIE;     // Enable receive interrupt
    UCA0CTL1 &= ~UCSWRST;  // Release hold
}

// For serial 1 on plaftforms where one is present
#if SERIAL_COUNT > 1
void msp430UsciSerialInit1(uint32_t speed)
{
    pinAsFunction(USCI_A1_RXTX_PORT, USCI_A1_TX_PIN);
    pinAsFunction(USCI_A1_RXTX_PORT, USCI_A1_RX_PIN);

    UCA1CTL1  = UCSWRST;   // Hold the module in reset state
    UCA1CTL1 |= UCSSEL_2;  // SMCLK clock source
    if (0 /* speed <= CPU_HZ / 16 */) { // FIXME
        // Use oversampling mode
        UCA1BR0  = (CPU_HZ / (speed * 16)) & 0xFF;
        UCA1BR1  = (CPU_HZ / (speed * 16)) >> 8;
        UCA1MCTL = UCOS16 | ((CPU_HZ / speed) & 0xF) << 4;
    }
    else {
        // Use normal mode with fractional clock divider
        UCA1BR0  = (CPU_HZ / speed) & 0xFF;
        UCA1BR1  = (CPU_HZ / speed) >> 8;
        UCA1MCTL = ((CPU_HZ * 8 / speed) & 0x7) << 1;
    }
    UCA1CTL0 = UART_MODE;
    UC1IE |= UCA1RXIE;     // Enable receive interrupt
    UCA1CTL1 &= ~UCSWRST;  // Release hold
}
#endif // SERIAL_COUNT > 1

//
// Send a byte to serial port
//
void serialSendByte(uint8_t id, uint8_t data)
{
    STACK_GUARD();

    if (id == 0) {
        while (!(UC0IFG & UCA0TXIFG));
        // Send data
        UCA0TXBUF = data;
    } else {
#if SERIAL_COUNT > 1
        while (!(UC1IFG & UCA1TXIFG));
        // Send data
        UCA1TXBUF = data;
#endif
    }
}

// UART mode receive handler
ISR(USCIAB0RX, USCI0InterruptHandler)
{
    bool error = UCA0STAT & UCRXERR;
    uint8_t data = UCA0RXBUF;
    if (error || UCA0STAT & UCOE) {
        // There was an error or a register overflow; clear UCOE and exit
        data = UCA0RXBUF;
        return;
    }

    if (serialRecvCb[0]) {
        serialRecvCb[0](data);
    }
}

#if SERIAL_COUNT > 1
ISR(USCIAB1RX, USCI1InterruptHandler)
{
    //
    // For Z1 this ISR is defined used as part of hardware I2C protocol
    //
#if PLATFORM_Z1 && USE_I2C
    if (UCB1STAT & UCNACKIFG) {
        // PRINTF("!!! NACK received in RX\n");
        UCB1CTL1 |= UCTXSTP;
        UCB1STAT &= ~UCNACKIFG;
        return;
    }
#endif

    bool error = UCA1STAT & UCRXERR;
    uint8_t data = UCA1RXBUF;
    if (error || UCA1STAT & UCOE) {
        // There was an error or a register overflow; clear UCOE and exit
        data = UCA1RXBUF;
        return;
    }

    if (serialRecvCb[1]) {
        serialRecvCb[1](data);
    }
}
#endif // SERIAL_COUNT > 1
