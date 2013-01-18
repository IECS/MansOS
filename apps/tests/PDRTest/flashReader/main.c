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

#include "stdmansos.h"
#include <string.h>
#include <serial_number.h>
#include <extflash.h>
#include <lib/codec/crc.h>
#include <lib/assert.h>
#include "../common.h"

// #define EXT_FLASH_RESERVED  (256 * 1024ul) // 256kb

// #define MAGIC_NUMBER 0xcafe

uint32_t extFlashAddress;

#define EXT_FLASH_RESERVED 0

// struct DataPacket_s {
//     uint32_t timestamp;
//     uint16_t sourceAddress;
//     uint16_t dataSeqnum;
//     uint16_t islLight;
//     uint16_t apdsLight0;
//     uint16_t apdsLight1;
//     uint16_t sq100Light;
//     uint16_t internalVoltage;
//     uint16_t internalTemperature;
//     uint16_t sht75Humidity;
//     uint16_t sht75Temperature;
//     uint16_t crc;
// } PACKED;

// ---------------------------------------------

/* void printPacket1(DataPacket_t *packet) */
/* { */
/*     PRINTF("timestamp=%lu\n", packet->timestamp); */
/*     PRINTF("src=%#04x\n", packet->sourceAddress); */
/*     PRINTF("dataSeqnum=%#x\n", packet->dataSeqnum); */
/*     PRINTF("islLight=%#x\n", packet->islLight); */
/*     PRINTF("apdsLight=%#x/%#x\n", packet->apdsLight0, packet->apdsLight1); */
/*     PRINTF("sq100Light=%#x\n", packet->sq100Light); */
/*     PRINTF("internalVoltage=%u\n", packet->internalVoltage); */
/*     PRINTF("internalTemperature=%u\n", packet->internalTemperature); */
/*     PRINTF("sht75Humidity=%#x\n", packet->sht75Humidity); */
/*     PRINTF("sht75Temperature=%#x\n", packet->sht75Temperature); */
/* } */

    /* uint16_t address; */
	/* uint16_t testId; */
    /* uint16_t lastTestNo; */
    /* uint16_t numTests; */
    /* uint16_t avgPdr; */
    /* uint16_t numTestsNe; */
    /* uint16_t avgPdrNe; */
    /* uint16_t avgRssiNe; */
    /* uint16_t avgLqiNe; */
    /* uint16_t crc; */

void printPacket(RadioInfoPacket_t *packet)
{
    PRINTF("0x%04x %u %u %u %u %u %u %u %u\n",
			packet->address,
			packet->testId,
			packet->lastTestNo,
			packet->numTests,
			packet->avgPdr,
			packet->numTestsNe,
			packet->avgPdrNe,
			packet->avgRssiNe,
			packet->avgLqiNe);
}

void readExtFlash(void)
{
    RadioInfoPacket_t packet;
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
//    PRINTF("flash packet offset=%lu\n", extFlashAddress - EXT_FLASH_RESERVED);
}


bool prepareExtFlash(void)
{
     extFlashWake();
     extFlashAddress = EXT_FLASH_RESERVED;

//     // check if old records exist
//     Record_t old;
//     extFlashRead(EXT_FLASH_RESERVED, &old, sizeof(old));
//     if (old.magicNumber == MAGIC_NUMBER
//             && old.crc == crc16((uint8_t *)&old, sizeof(Record_t) - sizeof(uint16_t))) {
//         return true;
//     }
//     return false;

     return true;
}

// Record_t endRecord;

// void readExtFlash(void)
// {
//     Record_t currRecord;
//     PRINTF("External flash contents:\n");
//     memset(&endRecord, -1, sizeof(endRecord));

//     ledOn();
//     for (;;) {
//         emset(&currRecord, 0, sizeof(Record_t));
//         extFlashRead(extFlashAddress, &currRecord, sizeof(Record_t));
//         if (currRecord.magicNumber == MAGIC_NUMBER
//                 && currRecord.crc == crc16((uint8_t *)&currRecord, sizeof(Record_t) - sizeof(uint16_t))) {
// //			PRINTF("%lu\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",currRecord.timestamp, currRecord.magicNumber, currRecord.isl,
// //				currRecord.battery, currRecord.humidity, currRecord.temperature, currRecord.crc);
//             PRINTF("%lu\t%d\t%d\t%d\n", currRecord.timestamp, currRecord.battery,
//                     currRecord.humidity, currRecord.temperature);
//         } else {
//             if (!memcmp(&currRecord, &endRecord, sizeof(Record_t))) break;
//             PRINTF("Bad CRC or magic number\n");
// //			PRINTF("End of records.\n");
// //			ledOff();
// //			return;
//         }
//         extFlashAddress += sizeof(Record_t);
//         // uint8_t buffer[16];
//         // extFlashRead(extFlashAddress, buffer, sizeof(buffer));
//         // debugHexdump(buffer, sizeof(buffer));
//         // extFlashAddress += sizeof(buffer);
//     }
// }

void appMain(void)
{
    PRINTF("Mote %#04x data...\n", localAddress);
    ledOn();
    prepareExtFlash();
    readExtFlash();
    //PRINTF("External flash is empty!\n");
    ledOff();
}
