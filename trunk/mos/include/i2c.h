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

#ifndef MANSOS_I2C_H
#define MANSOS_I2C_H

/// \file
/// Generic I2C interface.
///
/// Depending on bus ID, either hardware or software implementation can be selected.
/// Only I2C master mode is supported.
///

#include <i2c_types.h>
#include <i2c_soft.h>
#include <i2c_hal.h> // hardware function declaration

/**
 * Initializes the I2C bus
 */
static inline void i2cInit(uint8_t busId) {
    if (busId == I2C_BUS_SW) {
        i2cSoftInit();
    } else {
        i2cHwInit(busId);
    }
}

/**
 * Turn on the I2C bus
 */
static inline void i2cOn(uint8_t busId) {
    if (busId != I2C_BUS_SW) i2cHwOn(busId);
}

/**
 * Turn off the I2C bus
 */
static inline void i2cOff(uint8_t busId) {
    if (busId != I2C_BUS_SW) i2cHwOff(busId);
}

/**
 * Writes a string to I2C and checks acknowledge
 * @param   addr        address of the slave receiver (7 bits!)
 * @param   buf         the buffer containing the string
 * @param   len         buffer length in bytes
 * @return  0           on success, error code otherwise
 */
static inline i2cError_t i2cWrite(uint8_t busId, uint8_t addr,
                                  const void *buf, uint8_t len)
{
    if (busId == I2C_BUS_SW) return i2cSoftWrite(addr, buf, len);
    return i2cWrite(busId, addr, buf, len);
}

/**
 * Reads a message into buffer from I2C - requests it from a slave
 * @param   addr        address of the slave transmitter (7 bits!)
 * @param   buf         the buffer to store the message
 * @param   len         buffer length in bytes
 * @return  received byte count
 */
static inline uint8_t i2cRead(uint8_t busId, uint8_t addr,
                              void *buf, uint8_t len)
{
    if (busId == I2C_BUS_SW) return i2cSoftRead(addr, buf, len);
    return i2cRead(busId, addr, buf, len);
}

/**
 * Writes a byte to I2C and checks acknowledge
 * @param   addr    address of the slave receiver (7 bits!)
 * @param   txByte  byte to transmit
 * @return          0 on success, error code otherwise
 */
static inline i2cError_t i2cWriteByte(uint8_t busId, uint8_t addr, uint8_t txByte)
{
    if (busId == I2C_BUS_SW) return i2cSoftWriteByte(txByte);
    return i2cWrite(busId, addr, &txByte, 1);
}

/**
 * Reads a byte from I2C - requests it from a slave
 * @param   addr        address of the slave transmitter (7 bits!)
 * @param   rxByte      buffer, where the received data will be stored
 * @return  received byte count (1 on success, 0 on error)
 */
static inline uint8_t i2cReadByte(uint8_t busId, uint8_t slaveAddr)
{
    if (busId == I2C_BUS_SW) return i2cSoftReadByte(I2C_ACK);
    uint8_t byte = 0;
    i2cRead(busId, slaveAddr, &byte, 1);
    return byte;
}

#endif
