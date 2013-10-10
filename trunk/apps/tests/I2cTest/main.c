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

// I2C communication test

#include "stdmansos.h"
#include <assert.h>

#define I2C_ID            0
#define I2C_ADDRESS    0x31
#define IS_I2C_MASTER  true

static bool writeSingleByte(uint8_t byte)
{
    uint8_t err = 0;
    Handle_t intHandle;
    ATOMIC_START(intHandle);
    err = i2cWrite(I2C_ID, I2C_ADDRESS, &byte, 1) != 0;
    ATOMIC_END(intHandle);
    return err == 0;
}

static bool readSingleByte(uint8_t *byte)
{
    uint8_t err = 0;
    Handle_t intHandle;
    ATOMIC_START(intHandle);
    err = i2cRead(I2C_ID, I2C_ADDRESS, byte, 1) != 0;
    ATOMIC_END(intHandle);
    return err == 0;
}

static bool writeMultipleBytes(const uint8_t *bytes, uint8_t length)
{
    uint8_t err = 0;
    Handle_t intHandle;
    ATOMIC_START(intHandle);
    err = i2cWrite(I2C_ID, I2C_ADDRESS, bytes, length) != 0;
    ATOMIC_END(intHandle);
    return err == 0;
}

static bool readMultipleBytes(uint8_t *bytes, uint8_t length)
{
    uint8_t err = 0;
    Handle_t intHandle;
    ATOMIC_START(intHandle);
    err = i2cRead(I2C_ID, I2C_ADDRESS, bytes, length) != 0;
    ATOMIC_END(intHandle);
    return err == 0;
}

void appMain(void)
{
    const uint8_t dataTx[3] = {1, 2, 3};
    uint8_t dataRx[3];
    bool ok;

    // TODO:
//    i2cSetMasterMode(IS_I2C_MASTER);

    ok = writeSingleByte(dataTx[2]);
    if (!ok) goto end;

    ok = readSingleByte(dataRx);
    if (!ok) goto end;

    ok = writeMultipleBytes(dataTx, sizeof(dataTx));
    if (!ok) goto end;

    ok = readMultipleBytes(dataRx, sizeof(dataRx));
    if (!ok) goto end;

end:
    ASSERT(ok);
}
