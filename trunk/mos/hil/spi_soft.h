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

#ifndef MANSOS_SPI_SOFT_H
#define MANSOS_SPI_SOFT_H
//==============================================================================
// Software controlled SPI bus implementation, master mode only
// Prior to use you must define:
// *) MISO, MOSI, SCLK pins using your specific values, for example:
// #define SW_MISO_PORT 3
// #define SW_MISO_PIN  2
// #define SW_MOSI_PORT 3
// #define SW_MOSI_PIN  1
// #define SW_SCLK_PORT 3
// #define SW_SCLK_PIN  3
//
// Definition in config file also possible, for example:
// CONST_SW_MISO_PORT=1
// CONST_SW_MISO_PIN=1
// CONST_SW_MOSI_PORT=1
// CONST_SW_MOSI_PIN=2
// CONST_SW_SCLK_PORT=3
// CONST_SW_SCLK_PIN=5
//
// *) Optional: SPI bus speed (default: 1MHz), in KHz
//
// Example (1MHz speed):
// #define SW_SPI_SPEED_KHZ 1000
//
// Definition in config file also possible, for example:
// CFLAGS += -D SW_SPI_SPEED_KHZ=1000
//
// Allowed speeds are in range of 1 to 1000KHz (up to 1MHz). If other speeds
// are needed, ignore this constant and redefine SW_SPI_SLEEP_HALF_CYCLE()
// macro, for example:
// #define SW_SPI_SLEEP_HALF_CYCLE() NOP1()
//
//
// *) Optional: clock polarity and phase (CPOL and CPHA) as defined in
// http://en.wikipedia.org/wiki/Serial_Peripheral_Interface_Bus#Clock_polarity_and_phase
// For example:
// #define SW_SPI_CPOL 0
// #define SW_SPI_CPHA 0
//
// Definition in config file also possible, for example:
// CFLAGS += -D SW_SPI_CPOL=0 -D SW_SPI_CPHA=0
//
// If clock polarity and phase constants not defined, Mode 0 (CPOL = CPHA = 0)
// assumed by default
//
//==============================================================================

// TODO - implement SPI-slave mode

#include <digital.h>
#include <delay.h>
#include <spi.h>

// Pin configuration
#ifndef SW_MISO_PORT
//#error SW_MISO_PORT not defined for software SPI!
#define SW_SPI_PIN_ERROR 1
#endif
#ifndef SW_MISO_PIN
//#error SW_MISO_PIN not defined for software SPI!
#define SW_SPI_PIN_ERROR 1
#endif
#ifndef SW_MOSI_PORT
//#error SW_MOSI_PORT not defined for software SPI!
#define SW_SPI_PIN_ERROR 1
#endif
#ifndef SW_MOSI_PIN
//#error SW_MOSI_PIN not defined for software SPI!
#define SW_SPI_PIN_ERROR 1
#endif
#ifndef SW_SCLK_PORT
//#error SW_SCLK_PORT not defined for software SPI!
#define SW_SPI_PIN_ERROR 1
#endif
#ifndef SW_SCLK_PIN
//#error SW_SCLK_PIN not defined for software SPI!
#define SW_SPI_PIN_ERROR 1
#endif


// Clock phase and polarity constants
#ifndef SW_SPI_CPOL
#define SW_SPI_CPOL 0
#endif
#ifndef SW_SPI_CPHA
#define SW_SPI_CPHA 0
#endif

// Bus speed, 1MHz by default
#ifndef SW_SPI_SPEED_KHZ
#define SW_SPI_SPEED_KHZ 1000
#endif

// delay for a half clock cycle
#ifndef SW_SPI_SLEEP_HALF_CYCLE
#define SW_SPI_SLEEP_HALF_CYCLE() udelay(1000 / SW_SPI_SPEED_KHZ)
#endif


/**
 * Platform independent software-SPI byte exchange
 *
 * Exchange byte with a slave: write a byte to SPI and returns response,
 * received from the slave in full-duplex mode
 * Does not change any Slave-Select pin!
 *
 * @param   b       byte to transmit
 * @return          byte received from the slave
 */
uint8_t sw_spiExchByte(uint8_t b);

#if SW_SPI_PIN_ERROR != 1

#define SW_SPI_MOSI_HI()   pinSet(SW_MOSI_PORT, SW_MOSI_PIN)
#define SW_SPI_MOSI_LO()   pinClear(SW_MOSI_PORT, SW_MOSI_PIN)
#define SW_SPI_MISO_GET()  pinRead(SW_MISO_PORT, SW_MISO_PIN)

// Clock lead/trail functions depend on polarity
#if SW_SPI_CPOL == 0
#define SW_SPI_SCLK_LE()      pinSet(SW_SCLK_PORT, SW_SCLK_PIN)
#define SW_SPI_SCLK_TR()      pinClear(SW_SCLK_PORT, SW_SCLK_PIN)
#else
#define SW_SPI_SCLK_LE()      pinClear(SW_SCLK_PORT, SW_SCLK_PIN)
#define SW_SPI_SCLK_TR()      pinSet(SW_SCLK_PORT, SW_SCLK_PIN)
#endif


//------------------------------------------------------------------------------
// Initializes the ports for SPI
//------------------------------------------------------------------------------
// TODO - handle slave mode
#define sw_spiInit(mode)        \
    if (mode == SPI_MODE_MASTER) { \
        pinAsData(SW_MISO_PORT, SW_MISO_PIN);  \
        pinAsData(SW_MOSI_PORT, SW_MOSI_PIN); \
        pinAsData(SW_SCLK_PORT, SW_SCLK_PIN); \
        pinAsInput(SW_MISO_PORT, SW_MISO_PIN);  \
        pinAsOutput(SW_MOSI_PORT, SW_MOSI_PIN); \
        pinAsOutput(SW_SCLK_PORT, SW_SCLK_PIN); \
        SW_SPI_SCLK_TR(); \
    }

#else // SW_SPI_PIN_ERROR == 1

#warning Soft SPI is not usable because corresponding pins are not defined!

#define sw_spiInit(mode) \
    while (0) {}

#endif // if SW_SPI_PIN_ERROR != 1


#endif  // MANSOS_SPI_SOFT_H
