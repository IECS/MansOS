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

//
// SPI bus configuration for the MSP430 x1xx
//

#include "msp430_usart_spi.h"
#include <digital.h>

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



//-----------------------------------------------------------
//-----------------------------------------------------------

static void msp430SerialInitSpiPins(void)
{
    /* Select Peripheral functionality */
    pinAsFunction(HW_SCK_PORT, HW_SCK_PIN);
    pinAsFunction(HW_MOSI_PORT, HW_MOSI_PIN);
    pinAsFunction(HW_MISO_PORT, HW_MISO_PIN);

    /* Configure as outputs(SIMO,CLK). */
    pinAsOutput(HW_SCK_PORT, HW_SCK_PIN);
    pinAsOutput(HW_MOSI_PORT, HW_MOSI_PIN);
    pinAsInput(HW_MISO_PORT, HW_MISO_PIN);
}

void msp430SerialInitSPI0(uint_t spiBusMode)
{
    U0CTL = SWRST; // reset must be hold while configuring
    U0CTL |= CHAR | SYNC | MST; /* 8-bit transfer, SPI mode, master */
    // CKPH = 1 actually corresponds to CPHA = 0
    // (MSP430 User's guide, Figure 14-9)
    U0TCTL = CKPH | SSEL1 | STC; /* Data on Rising Edge, SMCLK, 3-wire. */

    // SPI CLK = SMCLK / 2
    U0BR0  = 0x02;  /* SPICLK set baud. */
    U0BR1  = 0;     /* Dont need baud rate control register 2 - clear it */
    U0MCTL = 0;     /* Dont need modulation control. */

    msp430SerialInitSpiPins();

    /* Enable SPI module & UART module (DO NOT REMOVE!) */
    IE1 &= ~(UTXIE0 | URXIE0);      // interrupt disabled
    U0ME |= USPIE0 | UTXE0 | URXE0;
    U0CTL &= ~SWRST;                  /* Remove RESET flag */
}

void msp430SerialInitSPI1(uint_t spiBusMode)
{
    U1CTL = SWRST; // reset must be hold while configuring
    U1CTL |= CHAR | SYNC | MST; /* 8-bit transfer, SPI mode, master */
    // CKPH = 1 actually corresponds to CPHA = 0
    // (MSP430 User's guide, Figure 14-9)
    U1TCTL = CKPH | SSEL1 | STC; /* Data on Rising Edge, SMCLK, 3-wire. */

    // SPI CLK = SMCLK / 2
    U1BR0  = 0x02;  /* SPICLK set baud. */
    U1BR1  = 0;     /* Dont need baud rate control register 2 - clear it */
    U1MCTL = 0;     /* Dont need modulation control. */

    msp430SerialInitSpiPins();

    IE2 &= ~(UTXIE1 | URXIE1);      // interrupt disabled
    U1ME |= USPIE1 | UTXE1 | URXE1;
    U1CTL &= ~SWRST;                  /* Remove RESET flag */
}
