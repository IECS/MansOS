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

#include "stdmansos.h"
#include <string.h>
#include <hil/i2c_soft.h>
#include <hil/snum.h>
#include <hil/extflash.h>
#include <lib/codec/crc.h>
#include <lib/assert.h>
#include <apds9300/apds9300.h>

//#define DATA_INTERVAL (1000ul * 60 * 5)  // -- once in every five minutes
#define DATA_INTERVAL (1000ul * 1)

#define EXT_FLASH_RESERVED  (256 * 1024ul) // 256kb

#define MAGIC_NUMBER 0xCAFE

uint32_t extFlashAddress;

struct Record_s
{
    uint32_t timestamp;
    uint16_t magicNumber;
    uint16_t apds0;
    uint16_t apds1;
    uint16_t battery;
    uint16_t humidity;
    uint16_t temperature;
    uint16_t __reserved;
    uint16_t crc;
} PACKED;

typedef struct Record_s Record_t;

// ---------------------------------------------

void prepareExtFlash(void)
{
    extFlashWake();

    // check if old records exist
    Record_t old;
    extFlashRead(EXT_FLASH_RESERVED, &old, sizeof(old));
    debugHexdump(&old, sizeof(old));
    if (old.magicNumber == MAGIC_NUMBER
            && old.crc == crc16((uint8_t *)&old, sizeof(Record_t) - sizeof(uint16_t))) {
        ASSERT("flash already contains data!" && false);
    }

    uint32_t address = EXT_FLASH_RESERVED;
    ledOn();
    for (; address < EXT_FLASH_SIZE; address += EXT_FLASH_SECTOR_SIZE) {
        extFlashEraseSector(address);
    }
    ledOff();
    extFlashAddress = EXT_FLASH_RESERVED;
}

void readSensors(void)
{
    PRINTF("reading sensors...\n");

    ledOn();

    humidityOn();
    mdelay(SHT11_RESPONSE_TIME * 1000);

    Record_t record;
    record.magicNumber = MAGIC_NUMBER;
    record.timestamp = getRealTime();
    if (apdsReadWord(COMMAND | DATA0LOW_REG, &record.apds0) != 0) {
        record.apds0 = 0xffff; // error value
    }
    if (apdsReadWord(COMMAND | DATA1LOW_REG, &record.apds1) != 0) {
        record.apds1 = 0xffff; // error value
    }
    record.battery = adcRead(ADC_INTERNAL_VOLTAGE);
    record.__reserved = 0;
    record.crc = crc16((uint8_t *)&record, sizeof(Record_t) - sizeof(uint16_t));

    extFlashWrite(extFlashAddress, &record, sizeof(Record_t));
    Record_t verifyRecord;
    memset(&verifyRecord, 0, sizeof(Record_t));
    extFlashRead(extFlashAddress, &verifyRecord, sizeof(Record_t));
    if (memcmp(&record, &verifyRecord, sizeof(Record_t))) {
        ASSERT("writing in flash failed!" && false);
    }
    extFlashAddress += sizeof(Record_t);

    humidityOff();

    ledOff();
}

void appMain(void)
{
    uint16_t i;

    // ------------------------- serial number
    uint8_t snumBuffer[SERIAL_NUMBER_SIZE];
    halGetSerialNumber(snumBuffer);
    PRINT("Initializing, serial number=");
    for (i = 0; i < SERIAL_NUMBER_SIZE; ++i) {
        PRINTF("0x%02x ", snumBuffer[i]);
    }
    PRINT("...\n");

    // ------------------------- external flash
    prepareExtFlash();

    // ------------------------- light sensor
    apdsInit();
    apdsOn();
    // disable interrupt generation
    apdsCommand(INTERRUPT_REG, DISABLE_INTERRUPT);
    // set normal integration mode (maximal sensitivity)
    apdsCommand(TIMING_REG, INTEGRATION_TIME_NORMAL);

    // ------------------------- main loop
    PRINTF("starting main loop...\n");
    for (i = 0; i < 6; ++i) {
        toggleRedLed();
        mdelay(100);
    }
    ledOff();

    uint32_t nextDataReadTime = 0;
    for (;;) {
        uint32_t now = getRealTime();
        if (timeAfter32(now, nextDataReadTime)) {
            nextDataReadTime = now + DATA_INTERVAL;
            readSensors();
        }
        toggleLed();
        msleep(1000);
    }
}
