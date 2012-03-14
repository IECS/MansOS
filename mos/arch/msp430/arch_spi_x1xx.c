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

/*
 * SPI bus configuration for the MSP430 x1xx
 */

#include "arch_spi_x1xx.h"

/**
 * Exchange byte with a slave: write a byte to SPI and returns response,
 * received from the slave in full-duplex mode
 * Does not change any Slave-Select pin!
 *
 * @param   busId   SPI bus ID
 * @param   b       byte to transmit
 * @return          byte received from the slave
 */
uint8_t hw_spiExchByte(uint8_t busId, uint8_t b) {
    if (busId == 0) {
        U0TXBUF = b;
        while ((U0TCTL & TXEPT) == 0);
        while ((IFG1 & URXIFG0) == 0);
        return U0RXBUF;
    } else {
        U1TXBUF = b;
        while ((U1TCTL & TXEPT) == 0);
        while ((IFG2 & URXIFG1) == 0);
        return U1RXBUF;
    }
}
