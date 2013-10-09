/*
 * Copyright (c) 2008-2012 the MansOS team. All rights reserved.
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

#ifndef MSP430_USART_H
#define MSP430_USART_H

#include "msp430_usart_spi.h"
#include <kernel/stack.h>

//===========================================================
// Data types and constants
//===========================================================
// SPI Serial Clock (SCK) on P3.3, MOSI on P3.1, MISO on P3.2
// I2C Serial Clock (SCL) on P3.3, SDA on P3.1
// USART0 TX on P3.4, RX on P3.5
// USART1 TX on P3.6, RX on P3.7
#define UTXD0_PORT 3
#define URXD0_PORT 3
#define UTXD1_PORT 3
#define URXD1_PORT 3

#define UTXD0_PIN  4
#define URXD0_PIN  5
#define UTXD1_PIN  6
#define URXD1_PIN  7

// UART1 aliases
#ifndef U1ME
#define U1ME              ME2
#define U1IE              IE2
#define U1IFG             IFG2
#endif

#if !USE_SOFT_SERIAL

static inline bool serialIsUART(uint8_t id) {
    if (id == 0) return !(U0CTL & SYNC);
    else return !(U1CTL & SYNC);
}

static inline bool serialIsSPI(uint8_t id) {
    if (id == 0) return (U0CTL & SYNC) && !(U0CTL & I2C);
    else return (U1CTL & SYNC) && !(U1CTL & I2C);
}

static inline bool serialIsI2C(uint8_t id) {
    if (id == 0) return (U0CTL & SYNC) && (U0CTL & I2C);
    else return (U1CTL & SYNC) && (U1CTL & I2C);
}

static inline void serialSendByte(uint8_t id, uint8_t data)
{
//    STACK_GUARD();

    if (id == 0) {
        U0TXBUF = data;
        while ((U0TCTL & TXEPT) == 0);  // Is byte sent ?
    } else {
        U1TXBUF = data;
        while ((U1TCTL & TXEPT) == 0);  // Is byte sent ?
    }
}

static inline void serialEnableTX(uint8_t id) {
    if (id == 0) U0ME |= UTXE0;
    else U1ME |= UTXE1;
}

static inline void serialDisableTX(uint8_t id) {
    if (id == 0) U0ME &= ~UTXE0;
    else U1ME &= ~UTXE1;
}

static inline void serialEnableRX(uint8_t id) {
    if (id == 0) {
        U0ME |= URXE0;
        // Enable RX interrupt only in UART mode
        if (serialIsUART(0)) IE1 |= URXIE0;
    } else {
        U1ME |= URXE1;
        // Enable RX interrupt only in UART mode
        if (serialIsUART(1)) IE2 |= URXIE1;
    }
}

static inline void serialDisableRX(uint8_t id) {
    if (id == 0) {
        U0ME &= ~URXE0;
        IE1 &= ~URXIE0;
    } else {
        U1ME &= ~URXE1;
        IE2 &= ~URXIE1;
    }
}

static inline bool serialUsesSMCLK(void ) {
    // return true ONLY if SMCLK is used for reception - it is impossible
    // to *transmit* anything while the MCU is sleeping
    if ((U0ME & URXE0) && (U0TCTL & SSEL_SMCLK)) return true;
    if ((U1ME & URXE1) && (U1TCTL & SSEL_SMCLK)) return true;
    return false;
}

// Not on all platforms SPI and I2C is part of USART
uint_t msp430SerialInitI2C(uint8_t id);

//
// Initialize the serial port
//
static inline uint_t serialInit(uint8_t id, uint32_t speed, uint8_t conf) {
    extern void msp430UsartSerialInit0(uint32_t speed);
    extern void msp430UsartSerialInit1(uint32_t speed);

    if (id == 0) msp430UsartSerialInit0(speed);
    else msp430UsartSerialInit1(speed);
    return 0;
}

#endif // !USE_SOFT_SERIAL


#endif
