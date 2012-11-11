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

#ifndef MANSOS_I2C_H
#define MANSOS_I2C_H

//==============================================================================
// Hardware controlled I2C, master mode only
// The actual implementation is in HPL. HAL layer provides macros which
// call the real code
//==============================================================================

#include <stdtypes.h>

// I2C acknowledge
typedef enum {
  I2C_NO_ACK = 0,
  I2C_ACK    = 1,
} i2cAck_t;

typedef enum {
  I2C_OK = 0,
  I2C_ACK_ERROR = 1,
  I2C_OTHER = 2
} i2cError_t;

// I2C command type: read or write (used in the last bit of the address byte)
typedef enum {
    I2C_CMD_WRITE = 0,
    I2C_CMD_READ = 1
} I2CCmd_t;

#ifdef USE_SOFT_I2C
// Use software I2C module
#include <i2c_soft.h>
#else
// Use hardware I2C module
//#include <i2c_hw.h>
#endif


/**
 * Initializes the I2C bus
 */
void i2cInit(void);

/**
 * Turn on the I2C bus
 */
void i2cOn(void);

/**
 * Turn off the I2C bus
 */
void i2cOff(void);

/**
 * Writes a byte to I2C and checks acknowledge
 * @param   addr    address of the slave receiver
 * @param   txByte  byte to transmit
 * @param   sendStop    whether to send stop condition after data
 * @return          0 on success, error code otherwise
 */
#define i2cWriteByte(addr, txByte, sendStop) \
    i2cWrite(addr, &(txByte), 1, sendStop)

/*
 * Writes a string to I2C and checks acknowledge
 * @param   addr        address of the slave receiver
 * @param   buf         the buffer containing the string
 * @param   len         buffer length in bytes
 * @param   sendStop    whether to send stop condition after data
 * @return  0           on success, error code otherwise
 */
i2cError_t i2cWrite(uint8_t addr, const void *buf, uint8_t len,
        bool sendStop);

/*
 * Reads a message into buffer from I2C - requests it from a slave
 * @param   addr        address of the slave transmitter
 * @param   buf         the buffer to store the message
 * @param   len         buffer length in bytes
 * @param   sendStop    whether to send stop condition after data
 * @return  received byte count
 */
uint8_t i2cRead(uint8_t addr, void *buf, uint8_t len, bool sendStop);

/*
 * Reads a byte from I2C - requests it from a slave
 * @param   addr        address of the slave transmitter
 * @param   rxByte      buffer, where the received data will be stored
 * @param   sendStop    whether to send stop condition after data
 * @return  received byte count (1 on success, 0 on error)
 */
static inline uint8_t i2cReadByte(uint8_t slaveAddr, bool sendStop) {
    uint8_t byte = 0;
    i2cRead(slaveAddr, &byte, 1, sendStop);
    return byte;
}


#endif
