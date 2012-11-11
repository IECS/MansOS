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

#include "apds9300.h"

void apdsInit(void) {
    // init SDA and SCK pins (defined in config file)
    i2cInit();

    // init INT pin
    pinAsInput(APDS_INT_PORT, APDS_INT_PIN);
    // pinEnableInt(APDS_INT_PORT, APDS_INT_PIN);
    pinDisableInt(APDS_INT_PORT, APDS_INT_PIN);
    pinClearIntFlag(APDS_INT_PORT, APDS_INT_PIN);
}

uint8_t apdsWriteByte(uint8_t cmd, uint8_t val) {
    uint8_t err = 0;
    Handle_t intHandle;
    ATOMIC_START(intHandle);
    i2cStart();
    err |= i2cWriteByteRaw(SLAVE_ADDRESS << 1);
    err <<= 1;
    err |= i2cWriteByteRaw(cmd);
    err <<= 1;
    err |= i2cWriteByteRaw(val);
    i2cStop();
    ATOMIC_END(intHandle);
    return err;
}

uint8_t apdsWriteWord(uint8_t cmd, uint16_t val) {
    uint8_t err = 0;
    Handle_t intHandle;
    ATOMIC_START(intHandle);
    i2cStart();
    err |= i2cWriteByteRaw(SLAVE_ADDRESS << 1);
    err <<= 1;
    err |= i2cWriteByteRaw(cmd | I2C_WORD);
    err <<= 1;
    err |= i2cWriteByteRaw(val & 0xff);
    err <<= 1;
    err |= i2cWriteByteRaw(val >> 8);
    i2cStop();
    ATOMIC_END(intHandle);
    return err;
}

uint8_t apdsReadByte(uint8_t cmd, uint8_t *value) {
    uint8_t err = 0;
    Handle_t intHandle;
    ATOMIC_START(intHandle);
    i2cStart();
    err |= i2cWriteByteRaw(SLAVE_ADDRESS << 1 );
    err <<= 1;
    err |= i2cWriteByteRaw(cmd);
    err <<= 1;
    i2cStop();
    i2cStart();
    err |= i2cWriteByteRaw((SLAVE_ADDRESS << 1)  | 0x1);
    uint8_t tempVal = i2cReadByteRaw(I2C_ACK);
    *value = tempVal;
    i2cStop();
    ATOMIC_END(intHandle);
    return err;
}

uint8_t apdsReadWord(uint8_t cmd, uint16_t *value) {
    uint8_t err = 0;
    Handle_t intHandle;
    ATOMIC_START(intHandle);
    i2cStart();
    err |= i2cWriteByteRaw(SLAVE_ADDRESS << 1);
    err <<= 1;
    err |= i2cWriteByteRaw(cmd | I2C_WORD);
    err <<= 1;
    i2cStop();
    i2cStart();
    err |= i2cWriteByteRaw((SLAVE_ADDRESS << 1) | 0x1);
    uint8_t tempValLo = i2cReadByteRaw(I2C_ACK);
    uint8_t tempValHi = i2cReadByteRaw(I2C_ACK);
    *value = tempValLo | (tempValHi << 8);
    i2cStop();
    ATOMIC_END(intHandle);
    return err;
}

bool apdsOn(void) {
   uint8_t err;
   if ((err = apdsCommand(CONTROL_REG, POWER_UP))) {
       PRINTF("apdsOn: err=0x%x\n", err);
       return false;
   }
   return true;
}

void apdsOff(void) {
    apdsCommand(CONTROL_REG, POWER_DOWN);
}

// ISR(PORT2, apds_interrupt)
// {
//     if (pinReadIntFlag(APDS_INT_PORT, APDS_INT_PIN)) {
//         PRINTF("got apds interrupt!\n");
//         pinClearIntFlag(APDS_INT_PORT, APDS_INT_PIN);
//     } else {
//         PRINTF("got some other port1 interrupt!\n");
//     }
// }

// uint8_t apdsData0Read(uint16_t *data){
//     uint8_t err = false;
//     uint8_t datalow, datahigh;
//     err |= apdsReadByte(COMMAND | DATA0LOW_REG, &datalow);
//     err <<= 1;
//     err |= apdsReadByte(COMMAND | DATA0HIGH_REG, &datahigh);
//     *data = (datalow | (datahigh << 8));
//     return err;
// }

// uint8_t apdsData1Read(uint16_t *data){
//     uint8_t err = false;
//     uint8_t datalow, datahigh;
//     err |= apdsReadByte(COMMAND | DATA1LOW_REG, &datalow);
//     err <<= 1;
//     err |= apdsReadByte(COMMAND | DATA1HIGH_REG, &datahigh);
//     *data = (datalow | (datahigh << 8));
//     return err;
// }
