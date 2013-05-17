/*
 * Copyright (c) 2008-2013 the MansOS team. All rights reserved.
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

#ifndef M25PXX_SPI_HAL_H
#define M25PXX_SPI_HAL_H

#include <spi.h>

/*
 * M25P16 Flash SPI bus configuration for Z1 platform
 */

// Flash attached to USCI 1 (B0) SPI BUS
#define M25PXX_SPI_ID      1
#define EXT_FLASH_SPI_ID   M25PXX_SPI_ID

// To use soft-SPI, uncomment the line below and define
// MISO, MOSI and SCLK pins (see hil/spi_soft.h) in your config file!
// #define M25PXX_SPI_ID   SPI_BUS_SW

// Flash pins
#define M25PXX_CS_PORT    4   /* P4.4 Output */
#define M25PXX_CS_PIN     4

#define M25PXX_HOLD_PORT  5   /* P4.7 Output */
#define M25PXX_HOLD_PIN   7

/* M25PXX flash functions */
#define SPI_M25PXX_HOLD()      pinClear(M25PXX_HOLD_PORT, M25PXX_HOLD_PIN);
#define SPI_M25PXX_UNHOLD()    pinSet(M25PXX_HOLD_PORT, M25PXX_HOLD_PIN);

/* Enable/disable flash access to the SPI bus (active low). */
#define M25PXX_SPI_ENABLE()    spiSlaveEnable(M25PXX_CS_PORT, M25PXX_CS_PIN)
#define M25PXX_SPI_DISABLE()   spiSlaveDisable(M25PXX_CS_PORT, M25PXX_CS_PIN)

#endif
