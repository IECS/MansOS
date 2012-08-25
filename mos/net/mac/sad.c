/**
 * Copyright (c) 2012 the MansOS team. All rights reserved.
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
#include <hil/radio.h>
#include <hil/errors.h>
#include <hil/alarms.h>
#include <hil/timers.h>
#include <lib/dprint.h>
#include <lib/radio_packet_buffer.h>
#include <net/net-stats.h>
#include <net/routing.h>

#define TEST_FILTERS 1

static void initSadMac(RecvFunction cb);
static int8_t sendSadMac(MacInfo_t *, const uint8_t *data, uint16_t length);
static void pollSadMac(void);
static void delayTimerCb(void *);

MacProtocol_t macProtocol = {
    .name = MAC_PROTOCOL_SAD,
    .init = initSadMac,
    .send = sendSadMac,
    .poll = pollSadMac,
    .buildHeader = defaultBuildHeader,
    .isKnownDstAddress = defaultIsKnownDstAddress,
};

static uint8_t delayedData[RADIO_MAX_PACKET];
static volatile uint16_t delayedDataLength;
static volatile uint8_t delayedNexthop;

static Alarm_t delayTimer;

// -----------------------------------------------

static void initSadMac(RecvFunction recvCb) {
    macProtocol.recvCb = recvCb;

    // use least significant byte of localAddress AMB8420 address  
    amb8420EnterAddressingMode(AMB8420_ADDR_MODE_ADDR, localAddress & 0xff);

    alarmInit(&delayTimer, delayTimerCb, NULL);
}

static int8_t sendSadMac(MacInfo_t *mi, const uint8_t *data, uint16_t length) {
    int8_t ret;
#if !(USE_ROLE_BASE_STATION || USE_ROLE_COLLECTOR || USE_ROLE_FORWARDER)
    MESSAGE("mote's code included!")
    if (mi->timeWhenSend) {
        if (delayedDataLength) {
            return -1; // busy
        }
        uint32_t now = getFixedTime();
        if (now < mi->timeWhenSend) {
            // PRINTF("delayed send after %ld\n", mi->timeWhenSend - now);
            delayedNexthop = getNexthop(mi) & 0xff;
            memcpy(delayedData, mi->macHeader, mi->macHeaderLen);
            delayedDataLength = mi->macHeaderLen;
            memcpy(delayedData + delayedDataLength, data, length);
            delayedDataLength += length;
            alarmSchedule(&delayTimer, mi->timeWhenSend - now);
            return length;
        }
    }
#else
    MESSAGE("mote's code excluded!")
#endif
    PRINTF("%lu: mac tx %u bytes\n", getFixedTime(), length);
    INC_NETSTAT(NETSTAT_RADIO_TX, EMPTY_ADDR);
    // use least significant byte
    amb8420SetDstAddress(getNexthop(mi) & 0xff);
    ret = radioSendHeader(mi->macHeader, mi->macHeaderLen, data, length);
    if (ret) return ret;
    return length;
}

static void delayTimerCb(void *unused)
{
    PRINTF("%lu: mac tx %u bytes\n", getFixedTime(), delayedDataLength);
    INC_NETSTAT(NETSTAT_RADIO_TX, EMPTY_ADDR);
    amb8420SetDstAddress(delayedNexthop);
    radioSendHeader(NULL, 0, delayedData, delayedDataLength);
    delayedDataLength = 0;
}

#if TEST_FILTERS

//
// filter out unwanted communications
//
static bool filterPass(MacInfo_t *mi)
{
#if USE_ROLE_BASE_STATION
    PRINTF("filter: from=%#04x, orig=%#04x\n",
            mi->immedSrc.shortAddr, mi->originalSrc.shortAddr);
#endif

    if (!mi->immedSrc.shortAddr) return true; // XXX

#if 0
    // 0x7BAA - base station
    // 0x3B88 - forwarder
    // 0x3BA5 - collector
    switch (localAddress) {
    case 0x7BAA:
        if (mi->immedSrc.shortAddr != 0x3BA5) return false;
        break;
    case 0x3B88:
        if (mi->immedSrc.shortAddr != 0x7BAA
                && mi->immedSrc.shortAddr != 0x3BA5) return false;
        break;
    case 0x3BA5:
    default:
        if (mi->immedSrc.shortAddr == 0x7BAA) return false;
        break;
    }
#else
    // 0x7BAA - base station
    // 0x3B88 - collector
    switch (localAddress) {
    case 0x7BAA:
        if (mi->immedSrc.shortAddr != 0x3B88) return false;
        break;
    case 0x3B88:
        break;
    default:
        if (mi->immedSrc.shortAddr == 0x7BAA) return false;
        break;
    }
#endif
    return true;
}

#endif

static void pollSadMac(void) {
    static MacInfo_t mi;
    INC_NETSTAT(NETSTAT_RADIO_RX, EMPTY_ADDR);
    if (isRadioPacketReceived()) {
//#if USE_ROLE_BASE_STATION
        PRINTF("%lu: mac rx %u bytes\n",
                getFixedTime(),
                radioPacketBuffer->receivedLength);
//#endif
        
        if (macProtocol.recvCb) {
            uint8_t *data = defaultParseHeader(radioPacketBuffer->buffer,
                    radioPacketBuffer->receivedLength, &mi);
#if TEST_FILTERS
            if (!filterPass(&mi)) {
                // PRINT("filtered out\n");
                data = NULL;
            }
#endif
            if (data) {
                //INC_NETSTAT(NETSTAT_PACKETS_RECV, mi.originalSrc.shortAddr);  // done @dv.c
                // call user callback
                macProtocol.recvCb(&mi, data,
                        radioPacketBuffer->receivedLength - mi.macHeaderLen);
            } else {
                // PRINT("no data\n");
                INC_NETSTAT(NETSTAT_PACKETS_DROPPED_RX, EMPTY_ADDR);
            }
        } else {
            // PRINT("no cb\n");
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
