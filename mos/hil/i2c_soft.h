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

#ifndef MANSOS_I2C_SOFT_H
#define MANSOS_I2C_SOFT_H
//==============================================================================
// Software controlled I2C
//  Prior to use you must define SDA and SCL pins using your specific values 
// #define SDA_PORT 1
// #define SDA_PIN  1
// #define SCL_PORT 1
// #define SCL_PIN  2
//
// Note: enable the pullup resistors, or ensure the hardware has them.
// Note: timing may have to be adjusted for a different microcontroller
//==============================================================================

#include <digital.h>
#include <delay.h>

#ifndef SDA_PORT
#error SDA_PORT not defined for software i2c!
#endif
#ifndef SDA_PIN
#error SDA_PIN not defined for software i2c!
#endif
#ifndef SCL_PORT
#error SCL_PORT not defined for software i2c!
#endif
#ifndef SCL_PIN
#error SCL_PIN not defined for software i2c!
#endif

// Define delays - user should define these depending on clock and MCU
#ifndef wait_1us
#define wait_1us udelay(1)
#endif
#ifndef wait_4us
#define wait_4us udelay(4)
#endif
#ifndef wait_5us
#define wait_5us udelay(5)
#endif
#ifndef wait_10us
#define wait_10us udelay(10)
#endif
#ifndef wait_20us
#define wait_20us udelay(20)
#endif


// I2C acknowledge
typedef enum {
    I2C_NO_ACK = 0,
    I2C_ACK    = 1,
} i2cAck_t;

typedef enum {
    I2C_OK = 0,
    I2C_ACK_ERROR = 1,
    I2C_TIME_OUT_ERROR = 2,
    I2C_CHECKSUM_ERROR = 4,
} i2cError_t;

// The communication on SDA (not SCL) is done by switching pad direction.
// For a low level the direction is set to output. 
// For a high level the direction is set to input (must have a pullup resistor).

#define I2C_SCL_OUT()  pinAsOutput(SCL_PORT, SCL_PIN)
#define I2C_SCL_HI()   pinSet(SCL_PORT, SCL_PIN)
#define I2C_SCL_LO()   pinClear(SCL_PORT, SCL_PIN)

#define I2C_SDA_IN()   pinAsInput(SDA_PORT, SDA_PIN)
#define I2C_SDA_OUT()  pinAsOutput(SDA_PORT, SDA_PIN)
#define I2C_SDA_HI()   do { pinAsOutput(SDA_PORT, SDA_PIN); pinSet(SDA_PORT, SDA_PIN); } while (0)
#define I2C_SDA_LO()   do { pinAsOutput(SDA_PORT, SDA_PIN); pinClear(SDA_PORT, SDA_PIN); } while (0)
// #define I2C_SDA_HI()   pinSet(SDA_PORT, SDA_PIN)
// #define I2C_SDA_LO()   pinClear(SDA_PORT, SDA_PIN)

#define I2C_SDA_GET()  pinRead(SDA_PORT, SDA_PIN)
#define I2C_SDA_SET(b) pinWrite(SDA_PORT, SDA_PIN, b)


//------------------------------------------------------------------------------
//Initializes the ports for I2C
//------------------------------------------------------------------------------
#define i2cInit()     \
    I2C_SDA_OUT();    \
    I2C_SCL_OUT();    \
    I2C_SDA_LO();     \
    I2C_SCL_LO();     \
    I2C_SDA_HI();     \
    I2C_SCL_HI();

//------------------------------------------------------------------------------
// Writes a start condition on I2C-bus
//        ___
// SDA:      |_____
//        _____
// SCL :       |___
//------------------------------------------------------------------------------
#define i2cStart() \
    I2C_SDA_HI();  \
    I2C_SCL_HI();  \
    wait_10us;     \
    I2C_SDA_LO();  \
    wait_10us;     \
    I2C_SCL_LO();  \
    wait_10us;


//------------------------------------------------------------------------------
// Writes a stop condition on I2C-bus
//             ____
// SDA:   ____|
//           ______
// SCL :  __|
//------------------------------------------------------------------------------
#define i2cStop()  \
    I2C_SDA_LO();  \
    I2C_SCL_LO();  \
    I2C_SCL_HI();  \
    wait_10us;     \
    I2C_SDA_HI();  \
    wait_10us;

//------------------------------------------------------------------------------
// Writes a byte to I2C and checks acknowledge
// returns 0 on success
//------------------------------------------------------------------------------
i2cError_t i2cWriteByte(uint8_t txByte);

//------------------------------------------------------------------------------
// Reads a byte from I2C
// Returns the byte received
// note: timing (delay) may have to be changed for different microcontroller
//------------------------------------------------------------------------------
uint8_t i2cReadByte(i2cAck_t ack);

#endif  // MANSOS_I2C_SOFT_H
