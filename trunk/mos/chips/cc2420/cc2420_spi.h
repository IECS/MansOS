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

#ifndef CC2420_SPI_H
#define CC2420_SPI_H

#include <spi.h>
#include <delay.h>
#include "cc2420_pins.h"

/*
 * SPI bus configuration for CC2420 radio chip
 * Platform independent code (uses macros defined by platform)
 */

/*******************************************************************************
*  TEXAS INSTRUMENTS INC.,                                                     *
*  MSP430 APPLICATIONS.                                                        *
*  Copyright Texas Instruments Inc, 2004                                       *
 ******************************************************************************/

/***********************************************************
    FAST SPI: FIFO Access
***********************************************************/
//    p = pointer to the byte array to be read/written
//    c = the number of bytes to read/write
//    b = single data byte

#define CC2420_TX_BYTE(b) \
    spiWriteByte(CC2420_SPI_ID, b)

#define CC2420_TX_WORD(v) \
    spiWriteWord(CC2420_SPI_ID, v)

#define CC2420_TX(buf, len) \
    spiWrite(CC2420_SPI_ID, buf, len)

#define CC2420_RX_BYTE(b) \
    (b) = spiReadByte(CC2420_SPI_ID)

#define CC2420_RX_WORD(v) \
    (v) = spiReadWord(CC2420_SPI_ID)

#define CC2420_RX(buf, len) \
    spiRead(CC2420_SPI_ID, buf, len)

#define CC2420_RX_DISCARD(len) \
    spiReadAndDiscard(CC2420_SPI_ID, len)

#define CC2420_TX_ADDR(a) \
    CC2420_TX_BYTE(a)

// 0x40 = 6th bit means "read mode"
#define CC2420_RX_ADDR(a) \
    CC2420_TX_BYTE(((a) | 0x40));

#define CC2420_STROBE(s)                         \
        CC2420_SPI_ENABLE();                     \
        CC2420_TX_ADDR(s);                       \
        CC2420_SPI_DISABLE();                    \

#define CC2420_GETREG(a,v)                      \
   do {                                         \
        CC2420_SPI_ENABLE();                    \
        CC2420_RX_ADDR(a);                      \
        CC2420_RX_WORD(v);                      \
        clock_delay(1);                         \
        CC2420_SPI_DISABLE();                   \
    } while (0)

// For Z1 platform: may want to look at CC2420_WRITE_REG() in Contiki.
#define CC2420_SETREG(a,v)                      \
   do {                                         \
        CC2420_SPI_ENABLE();                    \
        CC2420_TX_ADDR(a);                      \
        CC2420_TX_WORD(v);                      \
        clock_delay(1);                         \
        CC2420_SPI_DISABLE();                   \
    } while (0)


#define CC2420_WRITE_FIFO(p, c)                 \
    do {                                        \
        CC2420_SPI_ENABLE();                    \
        CC2420_TX_ADDR(CC2420_TXFIFO);          \
        CC2420_TX(p, c);                        \
        CC2420_SPI_DISABLE();                   \
    } while (0)

#define CC2420_WRITE_FIFO2(p1, c1, p2, c2) \
    do {                                           \
        CC2420_SPI_ENABLE();                       \
        CC2420_TX_ADDR(CC2420_TXFIFO);             \
        CC2420_TX(p1, c1);                         \
        CC2420_TX(p2, c2);                         \
        CC2420_SPI_DISABLE();                      \
    } while (0)

#define CC2420_WRITE_FIFO_NOCE(p,c)               \
    do {                                          \
        CC2420_TX_ADDR(CC2420_TXFIFO);            \
        CC2420_TX(p, c);                          \
    } while (0)

// flush of RX buffer is not required, as it is implicit in spiExchByte
#define CC2420_READ_FIFO_BYTE(b)                \
    do {                                        \
        CC2420_SPI_ENABLE();                    \
        CC2420_RX_ADDR(CC2420_RXFIFO);          \
        CC2420_RX_BYTE(b);                      \
        clock_delay(1);                         \
        CC2420_SPI_DISABLE();                   \
    } while (0)


// flush of RX buffer is not required, as it is implicit in spiExchByte
#define CC2420_READ_FIFO_NO_WAIT(p,c)                   \
    do {                                                \
        CC2420_SPI_ENABLE();                            \
        CC2420_RX_ADDR(CC2420_RXFIFO);                  \
        CC2420_RX(p, c);                                \
        clock_delay(1);                                 \
        CC2420_SPI_DISABLE();                           \
    } while (0)

// flush of RX buffer is not required, as it is implicit in spiExchByte
#define CC2420_READ_FIFO_GARBAGE(c)                     \
    do {                                                \
        CC2420_SPI_ENABLE();                            \
        CC2420_RX_ADDR(CC2420_RXFIFO);                  \
        CC2420_RX_DISCARD(c);                           \
        clock_delay(1);                                 \
        CC2420_SPI_DISABLE();                           \
    } while (0)

#define CC2420_UPD_STATUS(b)                            \
    do {                                                \
        CC2420_SPI_ENABLE();                            \
        CC2420_RX_BYTE(b);                              \
        CC2420_SPI_DISABLE();                           \
    } while (0)



/***********************************************************
    FAST SPI: CC2420 RAM access (big or little-endian order)
***********************************************************/
//  FAST SPI: CC2420 RAM access (big or little-endian order)
//    p = pointer to the variable to be written
//    a = the CC2420 RAM address
//    c = the number of bytes to write
//    n = counter variable which is used in for/while loops (u8_t)
//
//  Example of usage:
//    u8_t n;
//    u16_t shortAddress = 0xBEEF;
//    CC2420_WRITE_RAM_LE(&shortAddress, CC2420RAM_SHORTADDR, 2);


#define CC2420_WRITE_RAM_LE(p,a,c)              \
    do {                                        \
        CC2420_SPI_ENABLE();                    \
        CC2420_TX_BYTE(0x80 | (a & 0x7F));      \
        CC2420_TX_BYTE((a >> 1) & 0xC0);        \
        CC2420_TX(p, c);                        \
        CC2420_SPI_DISABLE();                   \
    } while (0)

// flush of RX buffer is not required, as it is implicit in spiExchByte
#define FASTSPI_READ_RAM_LE(p,a,c)                \
    do {                                          \
        CC2420_SPI_ENABLE();                      \
        CC2420_TX_BYTE(0x80 | (a & 0x7F));        \
        CC2420_TX_BYTE(((a >> 1) & 0xC0) | 0x20); \
        CC2420_RX(p, c);                          \
        CC2420_SPI_DISABLE();                     \
    } while (0)


#endif
