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
#include <isl29003/isl29003.h>
#include <ads1115/ads1115.h>

#include "datapacket.h"

// ---------------------------------------------

#define WRITE_TO_FLASH  1
#define PRINT_TO_SERIAL 0 // TODO
#define PRINT_PACKET    0 // TODO

// ---------------------------------------------

#if PRINT_TO_SERIAL
#define DPRINTF(...) PRINTF(__VA_ARGS__)
#define DPRINT(x)    PRINT(x)
#else
#define DPRINTF(...) do {} while (0)
#define DPRINT(x)    do {} while (0)
#endif

uint32_t extFlashAddress;
uint16_t dataSeqnum;

// ---------------------------------------------

#if PRINT_PACKET
void printPacket(DataPacket_t *packet)
{
    PRINT("===========================\n");
    PRINTF("dataSeqnum=%#x\n", packet->dataSeqnum);
    PRINTF("islLight=%#x\n", packet->islLight);
    PRINTF("sq100Light=%#x\n", packet->sq100Light);
    PRINTF("internalVoltage=%u\n", packet->internalVoltage);
    PRINTF("internalTemperature=%u\n", packet->internalTemperature);
    PRINTF("sht75Humidity=%#x\n", packet->sht75Humidity);
    PRINTF("sht75Temperature=%#x\n", packet->sht75Temperature);
}
#endif

#if WRITE_TO_FLASH
void prepareExtFlash(void)
{
    extFlashWake();

    // check if old records exist
    DataPacket_t packet;
    bool oneInvalid = false;

    extFlashAddress = EXT_FLASH_RESERVED;
    while (extFlashAddress < EXT_FLASH_SIZE) {
        extFlashRead(extFlashAddress, &packet, sizeof(packet));
        // PRINT("read packet:\n");
        // printPacket(&packet);
        bool valid = true;
        if (packet.crc != crc16((uint8_t *)&packet, sizeof(packet) - sizeof(uint16_t))) {
            valid = false;
        } else {
            if (packet.dataSeqnum == 0) valid = false;
        }

        if (valid) {
            oneInvalid = false;
        } else {
            // XXX: this is supposed to find first non-packet, but it will find first two invalid packets!!
            if (!oneInvalid) {
                oneInvalid = true;
            } else {
                extFlashAddress -= sizeof(packet);
                break;
            }
        }
        extFlashAddress += sizeof(packet);
    }
    PRINTF("flash packet offset=%lu\n", extFlashAddress - EXT_FLASH_RESERVED);
}
#endif

void readSensors(DataPacket_t *packet)
{
    DPRINTF("reading sensors...\n");

    ledOn();
    humidityOn();

    packet->timestamp = getJiffies();
    packet->sourceAddress = localAddress;
    packet->dataSeqnum = ++dataSeqnum;

    if (localAddress != 0x0796) {
        if (!islRead(&packet->islLight, true)) {
            PRINT("islRead failed\n");
            packet->islLight = 0xffff;
        }
        packet->sq100Light = 0xffff;
    } else {
        packet->islLight = 0xffff;
        if (!readAds(&packet->sq100Light)) {
            PRINT("readAdsRegister failed\n");
            packet->sq100Light = 0xffff;
        }
    }

    packet->internalVoltage = adcRead(ADC_INTERNAL_VOLTAGE);
    packet->internalTemperature = adcRead(ADC_INTERNAL_TEMPERATURE);

    DPRINT("read hum\n");
    packet->sht75Humidity = readHumidity();
    packet->sht75Temperature = readHTemperature();
    DPRINT("read done\n");

    packet->crc = crc16((uint8_t *) packet, sizeof(*packet) - 2);

#if WRITE_TO_FLASH
    if (extFlashAddress < EXT_FLASH_SIZE) {
        DPRINT("Writing to flash\n");
        extFlashWrite(extFlashAddress, packet, sizeof(*packet));
        DataPacket_t verifyRecord;
        memset(&verifyRecord, 0, sizeof(verifyRecord));
        extFlashRead(extFlashAddress, &verifyRecord, sizeof(verifyRecord));
        if (memcmp(packet, &verifyRecord, sizeof(verifyRecord))) {
            ASSERT("writing in flash failed!" && false);
        }
        extFlashAddress += sizeof(verifyRecord);
    }
#endif

    humidityOff();
    ledOff();
}

void appMain(void)
{
    uint16_t i;

    // ------------------------- serial number
    DPRINTF("Mote %#04x starting...\n", localAddress);

    // ------------------------- external flash
#if WRITE_TO_FLASH
    prepareExtFlash();
#endif

    // ------------------------- light sensors
    if (localAddress != 0x0796) {
        PRINT("init isl\n");
        islInit();
        islOn();
    } else {
        PRINT("init ads\n");
        adsInit();
        adsSelectInput(0);
    }

    // ------------------------- main loop
    mdelay(3000);
    DPRINTF("starting main loop...\n");
    for (i = 0; i < 6; ++i) {
        redLedToggle();
        mdelay(100);
    }
    ledOff();

    uint32_t nextDataReadTime = 0;
    uint32_t nextBlinkTime = 0;
    for (;;) {
        uint32_t now = getRealTime();
        if (timeAfter32(now, nextDataReadTime)) {
            if (getJiffies() < 300 * 1000ul ) {
                nextDataReadTime = now + DATA_INTERVAL_SMALL;
            } else {
                nextDataReadTime = now + DATA_INTERVAL;
            }
            DataPacket_t packet;
            readSensors(&packet);
#if PRINT_PACKET
            printPacket(&packet);
#endif
        }
        if (timeAfter32(now, nextBlinkTime)) {
            nextBlinkTime = now + BLINK_INTERVAL;
            ledOn();
            msleep(100);
            ledOff();
        }
        msleep(1000);
    }
}
