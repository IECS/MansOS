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

#include "flash.h"
#include "mansos.h"
#include "dprint.h"
#include "leds.h"
#include "platform_hpl.h"
#include "udelay.h"
#include "radio.h"
#include "timers.h"
#include "../sense.h"
#include <string.h>
#include <stdbool.h>


#define MODE_INIT   1
#define MODE_WRITE  2
#define MODE_READ   3


// which mode to use?
#define MODE   2

#define MAX_PACKETS  1000000ul

// this address is almost from the example...
#define COUNTER_ADDR 6250 // 0x1870
#define DATA_ADDR    6300 // 0x1880


static uint8_t buffer[100];
bool bufferBusy;
bool printAndExit;

typedef struct {
    uint32_t timestamp;
    SadPacket_t packet;
} __attribute__((packed)) TimestampSadPacket_t ;


void clearFlash(void) {
    uint32_t packetCounter = 0;

    st_flash_bulk_erase();

    flashSeek(COUNTER_ADDR);
    flashWrite(&packetCounter, sizeof(packetCounter));
}


// receive counter and display it on leds
void recvSadPacket(void)
{
    int16_t len;

    toggleGreenLed();
    if (bufferBusy) {
        radioDiscard();
        return;
    }

    len = radioRecv(buffer, sizeof(buffer));

    if (len < sizeof(SadPacket_t)) {
        toggleRedLed();
        return;
    }

    if (((SadPacket_t *) buffer)->id != SAD_STAMP) {
        uint32_t key = 0xdeadbeef;
        if (!memcmp(buffer, &key, sizeof(key))) {
            printAndExit = true;
            return;
        }
        toggleRedLed();
        return;
    }

    bufferBusy = true; // atomic write
}


void writePacket(SadPacket_t *p) {
    TimestampSadPacket_t tp;
    uint32_t packetCounter;
    uint32_t dataAddr;

    tp.timestamp = getRealTime();
    memcpy(&tp.packet, p, sizeof(*p));
    tp.packet.crc = crc16_data((uint8_t *) &tp, sizeof(tp) - 2, 0);

    flashRead(COUNTER_ADDR, &packetCounter, sizeof(packetCounter));
    if (packetCounter > MAX_PACKETS) {
        toggleRedLed();
        packetCounter = 0; // XXX: smth went wrong?
    }
    dataAddr = DATA_ADDR + packetCounter * sizeof(tp);
    ++packetCounter;
    udelay(100);
    flashSeek(COUNTER_ADDR);
    flashWrite(&packetCounter, sizeof(packetCounter));

    udelay(100);
    flashSeek(dataAddr);
    flashWrite(&tp, sizeof(tp));
}

void printPacket(TimestampSadPacket_t *p) {
    uint16_t calcCrc = crc16_data((uint8_t *) p, sizeof(*p) - 2, 0);
    if (p->packet.crc != calcCrc) {
        PRINTF("printPacket: bad CRC 0x%04x, expected 0x%04x\n",
                p->packet.crc, calcCrc);
        return;
    }
    PRINTF("printPacket: addr=%u voltage=%u\n",
            p->packet.address, p->packet.data.voltage);
}

void printResults(void) {
    uint8_t i;
    TimestampSadPacket_t *p;
#define PACKETS_IN_BUFFER  10 // XXX
    static TimestampSadPacket_t buffer[PACKETS_IN_BUFFER];
    uint32_t packetCounter;

    flashRead(COUNTER_ADDR, &packetCounter, sizeof(packetCounter));

    // read in buffer
    for (i = 0; i < PACKETS_IN_BUFFER && i < packetCounter; ++i) {
        flashRead(DATA_ADDR + i * sizeof(*p), buffer + i, sizeof(*p));
    }

    // turn flash off
    setFlashMode(DEV_MODE_OFF);

    blueLedOn();

    // print results
    PRINT_INIT(128);
    PRINTF("totalPackets: %u\n", packetCounter);
    for (i = 0; i < PACKETS_IN_BUFFER && i < packetCounter; ++i) {
        p = &buffer[i];
        printPacket(p);
    }
}

void pause(void) {
    uint16_t i;
    for (i = 0; i < 30; ++i) {
        busyWait(50000);
    }
}

void storeIncomingPackets(void) {
    for (;;) {
        SadPacket_t *p;
        uint16_t calcCrc;

        udelay(1000);

        if (printAndExit) {
            radioOff();
            printResults();
            break;
        }

        if (!bufferBusy) continue;

        // there is a packet is the buffer; extract it, check CRC, and write in flash
        p = (SadPacket_t *) buffer;
        calcCrc = crc16_data((uint8_t *) p, sizeof(*p) - 2, 0);
        if (p->crc != calcCrc) {
            bufferBusy = false;
            continue;
        }
        writePacket(p);

        toggleBlueLed();

        // free space for a new packet
        bufferBusy = false;
    }
}


//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    /* Initalize ports and SPI. */
    st_flash_init();

    pause();

    // turn flash on
    setFlashMode(DEV_MODE_ON);

    greenLedOn();

#if MODE == MODE_INIT

    clearFlash();

#elif MODE == MODE_WRITE

    clearFlash();

    radioInit();
    radioSetReceiveHandle(recvSadPacket);
    radioOn();
    
    toggleGreenLed();

    storeIncomingPackets();

#elif MODE == MODE_READ

    printResults();

#else
#error
#endif
}


/*
    {
        SadPacket_t packet = {0};
        uint32_t key = 0xdeadbeef;
        memcpy(&packet, &key, sizeof(key));
        radioSend(&packet, sizeof(packet));
    }
*/
