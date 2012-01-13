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

#ifndef AT25DF_PINS_H
#define AT25DF_PINS_H

#include <hil/spi.h>

/*
 * AT25DF161 Flash SPI bus configuration for SADmote
 */

// Flash attached to USART0 SPI BUS
#define AT25DF_SPI_ID   0

// To use soft-SPI, uncomment the line below and define
// MISO, MOSI and SCLK pins (see hil/spi_soft.h) in your config file!
//#define AT25DF_SPI_ID   SPI_BUS_SW

// Flash pins
#define AT25DF_CS_PORT    4   /* P4.1 Output */
#define AT25DF_CS_PIN     1

#define AT25DF_WP_PORT    4   /* P4.0 Output */
#define AT25DF_WP_PIN     0

#define AT25DF_HOLD_PORT  4   /* P4.3 Output */
#define AT25DF_HOLD_PIN   3

/* AT25DF flash functions */
#define SPI_AT25DF_HOLD()      pinClear(AT25DF_HOLD_PORT, AT25DF_HOLD_PIN)
#define SPI_AT25DF_UNHOLD()    pinSet(AT25DF_HOLD_PORT, AT25DF_HOLD_PIN)

/* Enable/disable flash access to the SPI bus (active low). */
#define AT25DF_SPI_ENABLE()    spiSlaveEnable(AT25DF_CS_PORT, AT25DF_CS_PIN)
#define AT25DF_SPI_DISABLE()   spiSlaveDisable(AT25DF_CS_PORT, AT25DF_CS_PIN)

#endif
