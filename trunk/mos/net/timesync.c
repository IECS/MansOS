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

#ifndef MANSOS_TIMESYNC_H
#define MANSOS_TIMESYNC_H

#include "timesync.h"
#include <print.h>
#include <timers.h>
#include <lib/codec/crc.h>

//
// TODO: lock the usart in rx mode (resource arbitration)
//

extern uint32_t lastRootSyncSeconds;
extern uint32_t lastRootClockSeconds;

#define DELIMITER '$'

typedef struct TimeSyncPacket_s {
    uint8_t delimiter1;
    uint8_t delimiter2;
    uint16_t crc;
    uint32_t time;
} TimeSyncPacket_t;

static uint8_t rxBytes;

static void parsePacket(TimeSyncPacket_t *packet) 
{
    uint16_t calcCrc = crc16((uint8_t *)&packet->time, sizeof(packet->time));
    if (packet->delimiter2 != 0
            || packet->crc != calcCrc) {
        PRINT("timesync: wrong format\n");
        return;
    }
    lastRootSyncSeconds = getUptime();
    lastRootClockSeconds = packet->time;
    // PRINTF("will use time from router: %lu\n", packet->time);
}

static void timesyncUsartReceive(uint8_t byte) {
    static TimeSyncPacket_t packet;
    if (rxBytes == 0) {
        if (byte != DELIMITER) return;
    }
    ((uint8_t *) (void *) &packet)[rxBytes] = byte;
    rxBytes++;
    if (rxBytes == sizeof(packet)) {
        parsePacket(&packet);
        rxBytes = 0;
    }
}

void timesyncInit(void) {
    USARTEnableRX(PRINTF_USART_ID);
    USARTSetReceiveHandle(PRINTF_USART_ID, timesyncUsartReceive);
}

#endif
