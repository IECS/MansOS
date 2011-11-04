/**
 * Copyright (c) 2008-2010 Leo Selavo and the contributors. All rights reserved.
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

// Write ADS1115 register
bool writeAdsRegister(uint8_t reg, uint16_t val){
    bool err = false;
    Handle_t intHandle;
    ATOMIC_START(intHandle);
    i2cStart();
    err |= i2cWriteByte(ADS_WRITE_FLAG);
    err |= i2cWriteByte(reg);
    err |= i2cWriteByte(val >> 8);
    err |= i2cWriteByte(val & 0xff);
    i2cStop();
    ATOMIC_END(intHandle);
    return err;
}

// Read ADS1115 register
bool readAdsRegister(uint8_t reg, uint16_t *val){
    bool err = false;
    *val = 0;
    Handle_t intHandle;
    ATOMIC_START(intHandle);
    i2cStart();
    err |= i2cWriteByte(ADS_WRITE_FLAG);
    err |= i2cWriteByte(reg);
    i2cStop();
    i2cStart();
    err |= i2cWriteByte(ADS_READ_FLAG); 
    *val = (i2cReadByte(I2C_ACK) << 8) | i2cReadByte(I2C_ACK) ;
    i2cStop();
    ATOMIC_END(intHandle);
    return err;
}

ISR(PORT2, ads_interrupt)
{
    if (pinReadIntFlag(ADS_INT_PORT, ADS_INT_PIN)) {
        PRINTF("got ads interrupt!\n");
        pinClearIntFlag(ADS_INT_PORT, ADS_INT_PIN);
    } else {
        PRINTF("got some other port2 interrupt!\n");
    }
}

void adsInit(){
	i2cInit();
    adsActiveConfig = ADS_DEFAULT_CONFIG;
    adsPowerDownSingleShotMode();
}

bool readAds(uint16_t *val){
	// check for mode, begin conversion if neccesary
	if (adsActiveConfig & ADS_MODE_MASK){
		adsBeginSingleConversion();
	}
	return readAdsRegister(ADS_CONVERSION_REGISTER,val);
}
