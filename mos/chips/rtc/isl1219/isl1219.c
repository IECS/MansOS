/*
 * Copyright (c) 2013 the MansOS team. All rights reserved.
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

#include "isl1219.h"
#include <platform.h>
#include <i2c.h>
#include <print.h>
//
// Initially, define functions for ISL29003 soft I2C support.
// The chip expects slightly variation from
// the default MSP430 HW and MansOS SW implementations.
//

/* ISL29003 soft I2C support */
#define ISL_I2C_SDA_HI()   pinSet(SDA_PORT, SDA_PIN)
#define ISL_I2C_SDA_LO()   pinClear(SDA_PORT, SDA_PIN)
/* End of ISL29003 soft I2C support */

// Write ISL1219 register
static uint8_t writeRegister(uint8_t address, uint8_t data)
{
    uint8_t err = 0;
    Handle_t handle;
    ATOMIC_START(handle);
    i2cSoftStart();
    // err |= islI2cWriteByte((ISL1219_ADDRESS << 1) | I2C_WRITE_FLAG);
    // err |= islI2cWriteByte(address);
    // err |= islI2cWriteByte(data);
    err |= i2cSoftWriteByte((ISL1219_ADDRESS << 1) | I2C_WRITE_FLAG);
    err |= i2cSoftWriteByte(address);
    err |= i2cSoftWriteByte(data);
    i2cSoftStop();
    ATOMIC_END(handle);
    return err;
}

// Read ISL1219 register
static uint8_t readRegister(uint8_t address, uint8_t *buffer, uint8_t dataLength)
{
    uint8_t err = 0;
    Handle_t handle;
    ATOMIC_START(handle);
    i2cSoftStart();
    // err |= islI2cWriteByte((ISL1219_ADDRESS << 1) | I2C_WRITE_FLAG);
    // err |= islI2cWriteByte(address);
    err |= i2cSoftWriteByte((ISL1219_ADDRESS << 1) | I2C_WRITE_FLAG);
    err |= i2cSoftWriteByte(address);
    i2cSoftStart();
//    err |= islI2cWriteByte((ISL1219_ADDRESS << 1) | I2C_READ_FLAG); 
    err |= i2cSoftWriteByte((ISL1219_ADDRESS << 1) | I2C_READ_FLAG); 
    while (dataLength > 1) {
        dataLength--;
        *buffer++ = i2cSoftReadByte(I2C_ACK);
    }
    *buffer++ = i2cSoftReadByte(I2C_NO_ACK);
    // ISL_I2C_SDA_LO();
    // I2C_SCL_LO();
    // udelay(4);
    i2cSoftStop();
    ATOMIC_END(handle);
    // PRINTF("read %#02x, err %#02x\n", *val, err);
    return err;
}

// ---------------------------------------------------

void isl1219Init(void)
{
    i2cInit(ISL1219_I2C_ID);
}

bool isl1219Write(const ISL1219_Clock_t *clock)
{
    uint8_t err;
    mdelay(100);
    err = writeRegister(ISL1219_REGISTER_STATUS_SECTION, ISL1219_WRITE_FLAG);

    uint8_t flags;
    err |= readRegister(ISL1219_REGISTER_STATUS_SECTION, &flags, 1);

    // write first 6 bytes, do not touch DW
    err |= writeRegister(ISL1219_REGISTER_RTC_SECTION + 0, clock->second);
    err |= writeRegister(ISL1219_REGISTER_RTC_SECTION + 1, clock->minute);
    err |= writeRegister(ISL1219_REGISTER_RTC_SECTION + 2, clock->hour);
    err |= writeRegister(ISL1219_REGISTER_RTC_SECTION + 3, clock->date);
    err |= writeRegister(ISL1219_REGISTER_RTC_SECTION + 4, clock->month);
    err |= writeRegister(ISL1219_REGISTER_RTC_SECTION + 5, clock->year);

    return err == 0;
}

bool isl1219Read(ISL1219_Clock_t *result)
{
    uint8_t err = 0;

    // read first 6 bytes, do not touch DW
    err |= readRegister(ISL1219_REGISTER_RTC_SECTION, (uint8_t *) result, 6);

    return err == 0;
}
