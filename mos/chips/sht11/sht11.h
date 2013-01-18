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

#ifndef MANSOS_SHT11_H
#define MANSOS_SHT11_H

//------------------------------------------------------
// Driver for SHT1x/SHT7x humidity & temperature sensors
//------------------------------------------------------

#include <digital.h>
#include <kernel/defines.h>
#include "humidity_hal.h"

// -----------------------
// internal stuff
// -----------------------

extern bool shtIsOn;

// SHT11 pins must be defined in HAL level
//#define SHT11_SDA_PORT 1
//#define SHT11_SDA_PIN 5
//#define SHT11_SCL_PORT 1
//#define SHT11_SCL_PIN 6
//#define SHT11_PWR_PORT 1
//#define SHT11_PWR_PIN 7

// platform-independent constants
// commands (including 3 address bits 000):
// * humidity readout: 000 00011
// * temperature readout: 000 00101
#define SHT11_CMD_TEMP  3
#define SHT11_CMD_HUM   5
#define SHT11_CMD_RESET 30

// response time as per datasheet
#define SHT11_RESPONSE_TIME    8 // seconds!

// platform-independent routines

// send read cmd, return result
uint16_t sht11_cmd(uint_t cmd);
// send connection reset sequence
void sht11_conn_reset(void);

// shortcuts
#ifdef SHT11_PWR_PORT
#define SHT11_PWR_HI() pinSet(SHT11_PWR_PORT, SHT11_PWR_PIN)
#define SHT11_PWR_LO() pinClear(SHT11_PWR_PORT, SHT11_PWR_PIN)
#define SHT11_PWR_OUT() pinAsOutput(SHT11_PWR_PORT, SHT11_PWR_PIN)
#else
// no power control on SM2 and SM3 platforms
#define SHT11_PWR_HI()
#define SHT11_PWR_LO()
#define SHT11_PWR_OUT()
#endif
#define SHT11_CLK_OUT() pinAsOutput(SHT11_SCL_PORT, SHT11_SCL_PIN)
#define SHT11_CLK_HI() pinSet(SHT11_SCL_PORT, SHT11_SCL_PIN)
#define SHT11_CLK_LO() pinClear(SHT11_SCL_PORT, SHT11_SCL_PIN)
#define SHT11_SDA_IN() pinAsInput(SHT11_SDA_PORT, SHT11_SDA_PIN)
#define SHT11_SDA_OUT() pinAsOutput(SHT11_SDA_PORT, SHT11_SDA_PIN)
#define SHT11_SDA_HI() pinSet(SHT11_SDA_PORT, SHT11_SDA_PIN)
#define SHT11_SDA_LO() pinClear(SHT11_SDA_PORT, SHT11_SDA_PIN)
#define SHT11_SDA_GET() pinRead(SHT11_SDA_PORT, SHT11_SDA_PIN)
#define SHT11_SDA_SET(b) pinWrite(SHT11_SDA_PORT, SHT11_SDA_PIN, b)

// 11ms required for sensor to start up, wait 15ms just to be sure
#define SHT11_ON() \
    SHT11_PWR_HI(); \
    sht11_conn_reset(); \
    sht11_cmd(SHT11_CMD_RESET); \
    shtIsOn = true

#define SHT11_OFF() \
    SHT11_PWR_LO(); \
    shtIsOn = false

// init sensor, turn power off
#define SHT11_INIT() \
    SHT11_PWR_OUT(); \
    SHT11_CLK_OUT(); \
    SHT11_OFF();     \
    SHT11_SDA_IN();  \
    SHT11_CLK_LO();  \
    SHT11_SDA_HI()

// -----------------------
// public functions
// -----------------------

// read temperature
static inline uint16_t sht11_read_temperature(void) {
    if (!shtIsOn) SHT11_ON();
    return sht11_cmd(SHT11_CMD_TEMP);
}

// read humidity
static inline uint16_t sht11_read_humidity(void) {
    if (!shtIsOn) SHT11_ON();
    return sht11_cmd(SHT11_CMD_HUM);
}

// TODO: improve this (use global variable?)
#define sht11_is_error() \
    (sht11_read_humidity() == 0xffff)

#endif

