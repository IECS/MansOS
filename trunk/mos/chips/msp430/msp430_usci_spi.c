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
 * msp430_usci_spi.c -- USCI module on MSP430 x2xx, SPI mode
 */

#include <hil/gpio.h>
#include <kernel/defines.h>
#include <kernel/stdtypes.h>

#include "msp430_usci.h"
    
/* Initialization */
int8_t hw_spiBusInit(uint8_t busId, SpiBusMode_t spiBusMode)
{
    /* SPI mode: master, MSB first, 8-bit, 3-pin */
#define SPI_MODE  (UCCKPH | UCMSB | UCMST | UCMODE_0 | UCSYNC)
#define SPI_SPEED (CPU_HZ / 2)

#define SETUP_SPI_PINS(id) ( \
    pinAsFunction(USCI_PORT, UC##id##SIMO_PIN), \
    pinAsFunction(USCI_PORT, UC##id##SOMI_PIN), \
    pinAsFunction(USCI_PORT, UC##id##CLK_PIN) )
#define SETUP_USCI(id) ( \
    UC##id##CTL1 = UCSWRST,             /* Hold the module in reset state */   \
    UC##id##CTL1 |= UCSSEL_2,           /* SMCLK clock source */               \
    UC##id##BR0 = (CPU_HZ / SPI_SPEED) & 0xFF, /* Clock divider, lower part */ \
    UC##id##BR1 = (CPU_HZ / SPI_SPEED) >> 8, /* Clock divider, higher part */  \
    UC##id##CTL0 = SPI_MODE,            /* Set specified mode */               \
    UC0IE &= ~UC##id##RXIE,             /* Disable receive interrupt */        \
    UC##id##CTL1 &= ~UCSWRST)           /* Release hold */

    if (busId == 0)
    {
        SETUP_SPI_PINS(A0);
        SETUP_USCI(A0);
    }
    else /* busId == 1 */
    {
        SETUP_SPI_PINS(B0);
        SETUP_USCI(B0);
    }
    
    return 0;
}

/* Data transmission */
uint8_t hw_spiExchByte(uint8_t busId, uint8_t b)
{
#define SPI_EXCH_BYTE(id) { \
    while (!(UC0IFG & UC##id##TXIFG)); /* Wait for ready */ \
    UC##id##TXBUF = b;               /* Send data */        \
    while (!(UC0IFG & UC##id##RXIFG)); /* Wait for reply */ \
    return UC##id##RXBUF; }          /* Return reply */

    if (busId == 0)
    {
        SPI_EXCH_BYTE(A0)
    }
    else /* busId == 1 */
    {
        SPI_EXCH_BYTE(B0)
    }
}
