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

#ifndef SDCARD_PINS_H
#define SDCARD_PINS_H

#include <hil/spi.h>

/*
 * SdCard SPI bus configuration for SADmote v03
 */

// SDcard attached to USART0 SPI BUS
#define SDCARD_SPI_ID   0

// To use soft-SPI, uncomment the line below and define
// MISO, MOSI and SCLK pins (see hil/spi_soft.h) in your config file!
//#define SDCARD_SPI_ID   SPI_BUS_SW

// Flash pins
#define SDCARD_CS_PORT    3   /* P3.0 Output */
#define SDCARD_CS_PIN     0

/* Enable/disable flash access to the SPI bus (active low). */
#define SDCARD_SPI_ENABLE()    \
    pinClear(SDCARD_CS_PORT, SDCARD_CS_PIN)

#define SDCARD_SPI_DISABLE()   \
    pinSet(SDCARD_CS_PORT, SDCARD_CS_PIN); \
    /* ensure MISO goes high impedance */  \
    spiWriteByte(SDCARD_SPI_ID, 0xff);

#endif
