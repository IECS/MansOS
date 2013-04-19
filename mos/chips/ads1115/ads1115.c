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

#include "ads1115.h"
#include "stdmansos.h"

#define ADS111X_I2C_ID I2C_BUS_SW

uint16_t adsActiveConfig;

// Write ADS111x register
bool writeAdsRegister(uint8_t reg, uint16_t val)
{
    uint8_t err = false;
    Handle_t intHandle;
    ATOMIC_START(intHandle);
    i2cSoftStart();
    err |= i2cSoftWriteByte((ADS_ADDRESS << 1) | I2C_WRITE_FLAG);
    err |= i2cSoftWriteByte(reg);
    err |= i2cSoftWriteByte(val >> 8);
    err |= i2cSoftWriteByte(val & 0xff);
    i2cSoftStop();
    ATOMIC_END(intHandle);
    //PRINTF("wrote: %#x%x\n",val >> 8,val & 0xff);
    return err == 0;
}

// Read ADS111x register
bool readAdsRegister(uint8_t reg, uint16_t *val)
{
    uint8_t err = false;
    *val = 0;
    Handle_t intHandle;
    ATOMIC_START(intHandle);
    i2cSoftStart();
    err |= i2cSoftWriteByte((ADS_ADDRESS << 1) | I2C_WRITE_FLAG);
    err |= i2cSoftWriteByte(reg);
    i2cSoftStop();
    i2cSoftStart();
    err |= i2cSoftWriteByte((ADS_ADDRESS << 1) | I2C_READ_FLAG);
    uint8_t hi = i2cSoftReadByte(I2C_ACK);
    uint8_t lo = i2cSoftReadByte(I2C_NO_ACK);
    *val = (hi << 8) | lo;
    i2cSoftStop();
    ATOMIC_END(intHandle);
    //PRINTF("recieved: %#x%x\n",hi,lo);
    return err == 0;
}

void adsInit(void)
{
    i2cInit(ADS111X_I2C_ID);
    adsActiveConfig = ADS_DEFAULT_CONFIG;
    writeAdsRegister(ADS_CONFIG_REGISTER, 0x8483);
    adsSelectInput(0);
    adsPowerDownSingleShotMode();
}

bool readAds(uint16_t *val)
{
    // check for mode, begin conversion if neccesary
    if (adsActiveConfig & ADS_MODE_MASK) {
        adsBeginSingleConversion();
    }
    return readAdsRegister(ADS_CONVERSION_REGISTER,val);
}
