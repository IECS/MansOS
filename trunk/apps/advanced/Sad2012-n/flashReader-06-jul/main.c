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
#include <hil/snum.h>
#include <hil/extflash.h>
#include <lib/codec/crc.h>
#include <lib/assert.h>

#include "../datalogger-06-jul/datapacket.h"

uint32_t extFlashAddress;

// ---------------------------------------------

void printPacket1(DataPacket_t *packet)
{
    PRINTF("timestamp=%lu\n", packet->timestamp);
    PRINTF("src=%#04x\n", packet->sourceAddress);
    PRINTF("dataSeqnum=%#x\n", packet->dataSeqnum);
    PRINTF("islLight=%#x\n", packet->islLight);
    PRINTF("internalVoltage=%u\n", packet->internalVoltage);
    PRINTF("internalTemperature=%u\n", packet->internalTemperature);
    PRINTF("sht75Humidity=%#x\n", packet->sht75Humidity);
    PRINTF("sht75Temperature=%#x\n", packet->sht75Temperature);
}

void printPacket(DataPacket_t *packet)
{
    PRINTF("%lu 0x%04x %#x %#x %#x %#x %#x %#x\n",
            packet->timestamp,
            packet->sourceAddress,
            packet->dataSeqnum,
            packet->islLight,
            packet->internalVoltage,
            packet->internalTemperature,
            packet->sht75Humidity,
            packet->sht75Temperature);
}

void readExtFlash(void)
{
    DataPacket_t packet;
    bool prevMissed = false;

    while (extFlashAddress < EXT_FLASH_SIZE) {
        extFlashRead(extFlashAddress, &packet, sizeof(packet));
        if (packet.crc == crc16((uint8_t *)&packet, sizeof(packet) - sizeof(uint16_t))) {
            if (prevMissed) {
                PRINT("corrupt packet\n");
            }
            prevMissed = false;
            printPacket(&packet);
        } else {
            // XXX: this is supposed to find first non-packet, but it will find first two invalid packets!!
            if (!prevMissed) {
                prevMissed = true;
            } else {
                break;
            }
        }
        extFlashAddress += sizeof(packet);
    }
}


bool prepareExtFlash(void)
{
     extFlashWake();
     extFlashAddress = EXT_FLASH_RESERVED;
     return true;
}

void appMain(void)
{
    PRINTF("Mote %#04x data...\n", localAddress);
    ledOn();
    prepareExtFlash();
    readExtFlash();
    //PRINTF("External flash is empty!\n");
    ledOff();
}
