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

#include "../mac.h"
#include "../queue.h"
#include <radio.h>
#include <errors.h>
#include <alarms.h>
#include <lib/unaligned.h>
#include <lib/byteorder.h>
#include <lib/buffer.h>
#include <print.h>
#include <random.h>
#include <lib/assert.h>
#include <net/radio_packet_buffer.h>
#include <net/net-stats.h>

#define TEST_FILTERS 1

static void initCsmaMac(RecvFunction cb);
static int8_t sendCsmaMac(MacInfo_t *, const uint8_t *data, uint16_t length);
static void pollCsmaMac(void);
static void sendTimerCb(void *);

MacProtocol_t macProtocol = {
    .name = MAC_PROTOCOL_CSMA,
    .init = initCsmaMac,
    .send = sendCsmaMac,
    .poll = pollCsmaMac,
    .buildHeader = defaultBuildHeader,
    .isKnownDstAddress = defaultIsKnownDstAddress,
};

static Alarm_t sendTimer;
static uint8_t sendTries;

// -----------------------------------------------

static void initCsmaMac(RecvFunction recvCb) {
    macProtocol.recvCb = recvCb;

    alarmInit(&sendTimer, sendTimerCb, NULL);

    // turn on radio listening
    radioOn();
}

static int8_t sendCsmaMac(MacInfo_t *mi, const uint8_t *data, uint16_t length) {
    int8_t ret;
    if (sendTries) {
        // remove old timers.
        alarmRemove(&sendTimer);
        sendTries = 0;
    }

#if MAC_FORWARDING_DELAY
    if (!IS_LOCAL(mi)) {
        // add random backoff for forwarded packets
        ret = queueAddPacket(mi, data, length, true, NULL);
        if (ret) return ret;
        alarmSchedule(&sendTimer, randomNumberBounded(MAC_PROTOCOL_MAX_INITIAL_BACKOFF));
        // do NOT increase send tries, because we are not trying to send!
        return length;
    }
#endif // MAC_FORWARDING_DELAY

    INC_NETSTAT(NETSTAT_RADIO_TX, EMPTY_ADDR);
    if (radioSendHeader(mi->macHeader, mi->macHeaderLen, data, length) == 0) {
        return length;
    }
    PRINT("*************** channel NOT free\n");

    ret = queueAddPacket(mi, data, length, true, NULL);
    if (ret) return ret;

    alarmSchedule(&sendTimer, MAC_PROTOCOL_RETRY_TIMEOUT);
    sendTries = 1;

    return length;
}

static void sendTimerCb(void *x) {
    QueuedPacket_t *p;
    uint16_t nextTimeout = MAC_PROTOCOL_RETRY_TIMEOUT * (1 << sendTries);

    if (sendTries) {
        PRINTF("************** retry to send (try %u)\n", sendTries + 1);
    }
    ++sendTries;

    p = queueHead();
    ASSERT(p);
    INC_NETSTAT(NETSTAT_RADIO_TX, EMPTY_ADDR);
    // XXX: not the best way, need to get address more simply
    MacInfo_t mi;
    defaultParseHeader(p->buffer.data, p->buffer.length, &mi);
    INC_NETSTAT(NETSTAT_PACKETS_RTX, mi.originalSrc.shortAddr);
    if (radioSend(p->buffer.data, p->buffer.length) == 0) {
        queuePop();
        sendTries = 0;
        return;
    }

    if (sendTries > MAC_PROTOCOL_MAX_TRIES) {
        // tx failed
        // XXX: return error code to user...
        INC_NETSTAT(NETSTAT_PACKETS_DROPPED_TX, EMPTY_ADDR);
        PRINT("CSMA mac send failed: too many retries!");
        queuePop();
        sendTries = 0;
        return;
    }

    // setup next timer, using exponential backoff
    alarmSchedule(&sendTimer, nextTimeout);
}

#if TEST_FILTERS

//
// filter out unwanted communications
//
static bool filterPass(MacInfo_t *mi)
{
    switch (localAddress) {
    case 0x0079:
        if (mi->originalSrc.shortAddr == 0x4876) return false;
        break;
    case 0x4876:
        if (mi->originalSrc.shortAddr == 0x0079) return false;
        break;
    }
    // case 0x5b5a:
    //     if (mi->originalDst.shortAddr == 0x4850
    //             || mi->originalDst.shortAddr == 0x4876) {
    //         return false;
    //     }
    //     break;
    // case 0x4850:
    // case 0x4876:
    //     if (mi->originalDst.shortAddr == 0x5b5a) return false;
    //     break;
    // }
    return true;
}

#endif

static void pollCsmaMac(void) {
    MacInfo_t mi;
    INC_NETSTAT(NETSTAT_RADIO_RX, EMPTY_ADDR);
    if (isRadioPacketReceived()) {
        // PRINTF("got a packet from radio, size=%u\n", radioPacketBuffer->receivedLength);
        
        if (macProtocol.recvCb) {
            uint8_t *data = defaultParseHeader(radioPacketBuffer->buffer,
                    radioPacketBuffer->receivedLength, &mi);
#if TEST_FILTERS
            if (!filterPass(&mi)) data = NULL;
#endif
            if (data) {
                //INC_NETSTAT(NETSTAT_PACKETS_RECV, mi.originalSrc.shortAddr);  // done @dv.c
                // call user callback
                macProtocol.recvCb(&mi, data,
                        radioPacketBuffer->receivedLength - mi.macHeaderLen);
            } else{
                INC_NETSTAT(NETSTAT_PACKETS_DROPPED_RX, EMPTY_ADDR);
            }
        } else{
           INC_NETSTAT(NETSTAT_PACKETS_DROPPED_RX, EMPTY_ADDR);
        }
    }
    else if (isRadioPacketError()) {
        INC_NETSTAT(NETSTAT_PACKETS_DROPPED_RX, EMPTY_ADDR);
        PRINTF("got an error from radio: %s\n",
                strerror(-radioPacketBuffer->receivedLength));
    }
    radioBufferReset();
}
