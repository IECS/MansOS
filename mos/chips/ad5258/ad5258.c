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

#include "ad5258.h"
#include <platform.h>
#include <i2c.h>
#include <print.h>

// Write AD5258 register
static bool writeAdRegister(uint8_t command)
{
    uint8_t err = 0;
    Handle_t intHandle;
    ATOMIC_START(intHandle);
#if USE_SOFT_I2C
    i2cStart();
    err |= i2cWriteByteRaw((AD5258_ADDRESS << 1) | AD5258_WRITE_FLAG);
    err |= i2cWriteByteRaw(command);
    i2cStop();
#else
    err = i2cWrite(AD5258_I2C_ID, AD5258_ADDRESS, &command, 1) != 0;
#endif
    ATOMIC_END(intHandle);
    return err == 0;
}

// Write AD5258 register data
static bool writeAdRegisterData(uint8_t command, uint8_t data)
{
    uint8_t err = 0;
    Handle_t intHandle;
    ATOMIC_START(intHandle);
#if USE_SOFT_I2C
    i2cStart();
    err |= i2cWriteByteRaw((AD5258_ADDRESS << 1) | AD5258_WRITE_FLAG);
    err <<= 1;
    err |= i2cWriteByteRaw(command);
    err <<= 1;
    err |= i2cWriteByteRaw(data);
    i2cStop();
#else
    uint8_t buf[2] = {command, data};
    err = i2cWrite(AD5258_I2C_ID, AD5258_ADDRESS, buf, 2) != 2;
#endif
    ATOMIC_END(intHandle);
    // PRINTF("err=%u\n", (uint16_t) err);
    return err == 0;
}

// Read AD5258 register
static bool readAdRegisterData(uint8_t command, uint8_t *val)
{
    uint8_t err = 0;
    *val = 0;
    Handle_t intHandle;
    ATOMIC_START(intHandle);
#if USE_SOFT_I2C
    i2cStart();
    err |= i2cWriteByteRaw((AD5258_ADDRESS << 1) | AD5258_WRITE_FLAG);
    err |= i2cWriteByteRaw(command);
    i2cStop();
    i2cStart();
    err |= i2cWriteByteRaw((AD5258_ADDRESS << 1) | AD5258_READ_FLAG);
    *val = i2cReadByteRaw(I2C_NO_ACK);
    i2cStop();
#else
    err = i2cWrite(AD5258_I2C_ID, AD5258_ADDRESS, &command, 1) != 1;
    err |= i2cRead(AD5258_I2C_ID, AD5258_ADDRESS, val, 1) != 1;
#endif
    ATOMIC_END(intHandle);
    // PRINTF("read %#02x, err %#02x\n", *val, err);
    return err == 0;
}

// ---------------------------------------------------

void ad5258Init(void)
{
    i2cInit(AD5258_I2C_ID);
}


bool ad5258Write(uint8_t value)
{
    bool error;
    uint8_t readValue;

    // the potentiometer has only 64 states
    value &= 0x3F;

    // first write to RDAC; then verify; then copy to EEPROM.
    error = !writeAdRegisterData(AD5258_CMD_ACCESS_RDAC, value);
    if (error) return false;

    error = !readAdRegisterData(AD5258_CMD_ACCESS_RDAC, &readValue);
    if (error) return false;

    if (readValue != value) {
        // PRINTF("verify failed!\n");
        return false;
    }

    error = !writeAdRegister(AD5258_CMD_STORE_TO_EEPROM);
    if (error) return false;

    return true;
}
