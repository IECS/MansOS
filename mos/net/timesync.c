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

#ifndef MANSOS_TIMESYNC_H
#define MANSOS_TIMESYNC_H

#include "timesync.h"
#include <print.h>
#include <timing.h>
#include <lib/codec/crc.h>

//
// TODO: lock the usart in rx mode (resource arbitration)
//

extern uint64_t lastRootSyncMilliseconds;
extern uint64_t lastRootClockMilliseconds;

#define DELIMITER '$'

struct TimeSyncPacket_s {
    uint8_t delimiter1;
    uint8_t isExtended;
    uint16_t crc;
    uint32_t time;
    uint32_t milliseconds;
} PACKED;

typedef struct TimeSyncPacket_s TimeSyncPacket_t;

static uint8_t rxBytes;

static void parsePacket(TimeSyncPacket_t *packet) 
{
    uint16_t calcCrc = crc16((uint8_t *)&packet->time, packet->isExtended ? 8 : 4);
    if (packet->crc != calcCrc) {
        PRINTF("timesync: wrong format\n");
        return;
    }
    lastRootSyncMilliseconds = getTimeMs64();
    lastRootClockMilliseconds = packet->time * 1000;
    if (packet->isExtended) {
        lastRootClockMilliseconds += packet->milliseconds;
    }
    // PRINTF("will use time from router: %lu\n", packet->time);
}

static void timesyncUsartReceive(uint8_t byte) {
    static TimeSyncPacket_t packet;
    if (rxBytes == 0) {
        if (byte != DELIMITER) return;
    }
    ((uint8_t *) (void *) &packet)[rxBytes] = byte;
    rxBytes++;
    if (rxBytes == sizeof(packet) - 4) {
        if (!packet.isExtended) {
            parsePacket(&packet);
            rxBytes = 0;
        }
    }
    if (rxBytes == sizeof(packet)) {
        parsePacket(&packet);
        rxBytes = 0;
    }
}

void timesyncOn(void) {
    serialEnableRX(PRINTF_SERIAL_ID);
    serialSetReceiveHandle(PRINTF_SERIAL_ID, timesyncUsartReceive);
}

void timesyncOff(void) {
    serialDisableRX(PRINTF_SERIAL_ID);
}

void timesyncInit(void) {
    // turn it on by default
    timesyncOn();
}

#endif
