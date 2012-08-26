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

#ifndef MRF24J40_SPI_HAL_H
#define MRF24J40_SPI_HAL_H

#include <digital.h>

/*
 * MRF24J40 SPI bus configuration
 */

// Radio attached to USART0 SPI BUS
#define MRF24_SPI_ID   0

// To use soft-SPI for communication with CC2420, uncomment the line below and
// define MISO, MOSI and SCLK pins (see spi_soft.h) in your config file!
// #define MRF24_SPI_ID   SPI_BUS_SW


#define MRF24_CS_PORT     4   /* P4.2 Output */
#define MRF24_CS_PIN      2

#define MRF24_RESET_PORT  4   /* P4.4 Output */
#define MRF24_RESET_PIN   4

#define MRF24_WAKE_PORT   1   /* P1.3 Output */
#define MRF24_WAKE_PIN    3

#define MRF24_INT_PORT    1   /* P1.0 Input */
#define MRF24_INT_PIN     0

/* Enable/disable radio access to the SPI bus (active low). */
#define MRF24_SPI_ENABLE()    spiSlaveEnable(MRF24_CS_PORT, MRF24_CS_PIN)
#define MRF24_SPI_DISABLE()   spiSlaveDisable(MRF24_CS_PORT, MRF24_CS_PIN)

#define MRF24_WR_BYTE(b) \
    spiWriteByte(MRF24_SPI_ID, b)
#define MRF24_RD_BYTE() \
    spiReadByte(MRF24_SPI_ID)
#define MRF24_WR_MANY(buf, len) \
    spiWrite(MRF24_SPI_ID, buf, len)
#define MRF24_RD_MANY(buf, len) \
    spiRead(MRF24_SPI_ID, buf, len)

#endif
