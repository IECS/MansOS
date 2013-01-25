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

#ifndef ATMEGA_I2C_HAL_H
#define ATMEGA_I2C_HAL_H

/*
 * ATMega 2-wire is I2C-compatible
 */

#include <i2c_types.h>
#include "atmega/atmega_2wire.h"

/**
 * Initializes the I2C bus
 */
static inline void i2cHwInit(uint8_t busId) {
    twiInit();
}

/**
 * Turn on the I2C bus
 */
static inline void i2cHwOn(uint8_t busId) {
    twiOn();
}

/**
 * Turn off the I2C bus
 */
static inline void i2cHwOff(uint8_t busId) {
    twiOff();
}

/*
 * Writes a string to I2C and checks acknowledge.
 * Warning: STOP signal is ALWAYS sent, regardless of sendStop param!
 * TWI driver implementation does not support omit of STOP.
 * @param   addr        address of the slave receiver
 * @param   buf         the buffer containing the string
 * @param   len         buffer length in bytes
 * @param   sendStop    whether to send stop condition after data
 * @return  0           on success, error code otherwise
 */
static inline i2cError_t i2cHwWrite(uint8_t busId, uint8_t addr,
                                    const void *buf, uint8_t len,
                                    bool sendStop) {
    return twiWrite(addr, buf, len, 1);
}

/*
 * Reads a message into buffer from I2C - requests it from a slave
 * Warning: STOP signal is ALWAYS sent, regardless of sendStop param!
 * TWI driver implementation does not support omit of STOP.
 * @param   addr        address of the slave transmitter
 * @param   buf         the buffer to store the message
 * @param   len         buffer length in bytes
 * @param   sendStop    whether to send stop condition after data
 * @return  received byte count
 */
static inline uint8_t i2cHwRead(uint8_t busId, uint8_t addr,
                               void *buf, uint8_t len,
                               bool sendStop) {
    return twiRead(addr, buf, len);
}

#endif
