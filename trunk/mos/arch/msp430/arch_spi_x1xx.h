/**
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

#ifndef MSP430_SPI_X1XX_H
#define MSP430_SPI_X1XX_H

#include <hil/gpio.h>
#include <msp430/msp430_usart.h>

/*
 * SPI bus configuration for the MSP430 x1xx
 */

/**
 * Initializes SPI bus in either master or slave mode
 * Does not change any Slave-Select pins!

 * @param   busId   SPI bus ID
 * @param   mode    SPI bus mode: either master or slave
 * @return  0       on success, -1 on error
 */
static inline int8_t hw_spiBusInit(uint8_t busId, SpiBusMode_t spiBusMode) {
    return msp430USARTInitSPI(busId, spiBusMode);
}

/**
 * Turn on the SPI bus, provide bus ID (starting from 0)
 * On MSP430, USART TX/RX is used for SPI TX/RX
 * @param   busId   SPI bus ID
 */
static inline void hw_spiBusOn(uint8_t busId) {
    // enable tx and rx
    if (busId == 0) {
        U0ME |= (UTXE0 | URXE0);
    } else {
        U1ME |= (UTXE1 | URXE1);
    }
}

/**
 * Turn off the SPI bus, provide bus ID (starting from 0)
 * On MSP430, USART TX/RX is used for SPI TX/RX
 * @param   busId   SPI bus ID
 */
static inline void hw_spiBusOff(uint8_t busId) {
    // disable tx and rx
    if (busId == 0) {
        U0ME &= ~(UTXE0 | URXE0);
    } else {
        U1ME &= ~(UTXE1 | URXE1);
    }
}

#endif
