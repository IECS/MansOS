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

#ifndef MANSOS_ISL29003_H
#define MANSOS_ISL29003_H

#include "i2c_soft.h"

/* ISL29003 soft I2C support */
#define ISL_I2C_SDA_HI()   pinSet(SDA_PORT, SDA_PIN)
#define ISL_I2C_SDA_LO()   pinClear(SDA_PORT, SDA_PIN)
/* End of ISL29003 soft I2C support */

// ISL29003 I2C address
#define ISL_ADDRESS 0x44
// Command register number
#define ISL_COMMAND_REGISTER 0x00
// Control register number
#define ISL_CONTROL_REGISTER 0x01
// 8th bit in command register
#define ISL_ENABLE_BIT 0x80
// 7th bit in command register
#define ISL_SLEEP_BIT 0x40
// 5th bit in control register
#define ISL_INTERUPT_BIT 0x20
// ISL29003 register for clearing interupt
#define ISL_CLEAR_INTERUPT_REGISTER 0x40

// Configure ISL29003 - http://www.intersil.com/data/fn/fn7464.pdf pages 6-7
// mode:
//              0 - use DIODE1
//              1 - use DIODE2
//              2 - use ~(DIODE1 - DIODE2) refer to datasheet (default)
//              3 - No operation
//             >3 - ignore
// clock_cycles:
//              0 - 2^16 = 65,536 (default)
//              1 - 2^12 = 4,096
//              2 - 2^8 = 256
//              3 - 2^4 = 16
//             >3 - ignore
// range_gain:
//              0 - 973 (default)
//              1 - 3892
//              2 - 15,568
//              3 - 62,272
//             >3 - ignore
// integration_cycles:
//              0 - 1
//              1 - 4
//              2 - 8
//              3 - 16 (default)
//             >3 - ignore
typedef struct IslConfigure_s{
    enum{
        USE_DIODE1 = 0,
        USE_DIODE2 = 1,
        USE_BOTH_DIODES = 2,
        NO_OPERATION = 3,
        IGNORE_MODE = 4
    } mode;
    enum{
        CLOCK_CYCLES_16 = 0,
        CLOCK_CYCLES_12 = 1,
        CLOCK_CYCLES_8 = 2,
        CLOCK_CYCLES_4 = 3,
        IGNORE_CLOCK_CYCLES = 4
    } clock_cycles;
    enum{
        RANGE_GAIN_09 = 0,
        RANGE_GAIN_3 = 1,
        RANGE_GAIN_15 = 2,
        RANGE_GAIN_62 = 3,
        IGNORE_RANGE_GAIN = 4
    } range_gain;
    enum{
        INTEGRATION_CYCLES_1 = 0,
        INTEGRATION_CYCLES_4 = 1,
        INTEGRATION_CYCLES_8 = 2,
        INTEGRATION_CYCLES_16 = 3,
        IGNORE_INTEGRATION_CYCLES = 4
    } integration_cycles;
} IslConfigure_t;

// Enable ISL29003
bool islOn();

// Disable ISL29003
bool islOff();

// Put ISL29003 to normal mode
bool islWake();

// Put ISL29003 to sleep mode
bool islSleep();

// Check if ISL29003 is ON
bool isIslOn();

// Check if ISL29003 is awake
bool isIslWake();

// Initialize ISL29003, configure and turn it off
bool islInit();

// Write ISL29003 register
bool writeIslRegister(uint8_t reg, uint8_t val);

// Read ISL29003 register
bool readIslRegister(uint8_t reg, uint8_t *val);

// Configure ISL29003 registers
bool configureIsl(IslConfigure_t newConf);

// Check if ISL29003 have data to give
bool islInterupt(bool clearIfHave);

// Clear ISL29003 interupt bit
bool clearIslInterupt();

// Read ISL29003 sensor data
bool islRead(uint16_t *data, bool checkInterupt);

uint16_t islReadSimple(void);

#endif
