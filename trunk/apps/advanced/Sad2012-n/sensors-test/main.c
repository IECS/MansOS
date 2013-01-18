/*
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
#include <serial_number.h>
#include <extflash.h>
#include <lib/codec/crc.h>
#include <lib/assert.h>
#include <isl29003/isl29003.h>

struct DataPacket_s {
    uint32_t timestamp;
    uint16_t islLight;
    uint16_t internalVoltage;
    uint16_t internalTemperature;
    uint16_t sht75Humidity;
    uint16_t sht75Temperature;
    uint16_t crc;
} PACKED;

typedef struct DataPacket_s DataPacket_t;

void printPacket(DataPacket_t *packet)
{
    PRINT("===========================\n");
    PRINTF("timestamp=%lu\n", packet->timestamp);
    PRINTF("islLight=%#x\n", packet->islLight);
    PRINTF("internalVoltage=%u\n", packet->internalVoltage);
    PRINTF("internalTemperature=%u\n", packet->internalTemperature);
    PRINTF("sht75Humidity=%#x\n", packet->sht75Humidity);
    PRINTF("sht75Temperature=%#x\n", packet->sht75Temperature);
}

void readSensors(DataPacket_t *packet)
{
    PRINTF("reading sensors...\n");
    uint32_t start = getJiffies();

    ledOn();
    humidityOn();

    packet->timestamp = getUptime();
    if (!islRead(&packet->islLight, true)) {
        PRINT("islRead failed\n");
        packet->islLight = 0xffff;
    // } else {
    //     PRINT("islRead OK\n");
    }
    packet->internalVoltage = adcRead(ADC_INTERNAL_VOLTAGE);
    packet->internalTemperature = adcRead(ADC_INTERNAL_TEMPERATURE);
    packet->sht75Humidity = humidityRead();
    packet->sht75Temperature = temperatureRead();

    humidityOff();
    ledOff();

    uint32_t end = getJiffies();
    PRINTF("   time spent: %u ms\n", end - start);
}

void appMain(void)
{
    // ------------------------- serial number
    PRINTF("Mote %#04x starting...\n", localAddress);

    // ------------------------- light sensors
    islInit();
    islOn();

    for (;;) {
        DataPacket_t packet;
        readSensors(&packet);
        printPacket(&packet);
        mdelay(2000);
    }
}
