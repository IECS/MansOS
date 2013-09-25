/*
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
#include <radio.h>
#include <errors.h>
#include <alarms.h>
#include <timing.h>
#include <print.h>
#include <net/net_stats.h>
#include <net/routing.h>
#include <net/radio_packet_buffer.h>
#include <leds.h>
#include <codec.h>

#define TEST_FILTERS 1

static void initSadMac(RecvFunction cb);
static int8_t sendSadMac(MacInfo_t *, const uint8_t *data, uint16_t length);
static void pollSadMac(void);

extern void networkingForwardData(MacInfo_t *macInfo, uint8_t *data, uint16_t len);

TEXTDATA MacProtocol_t macProtocol = {
    .name = MAC_PROTOCOL_SAD,
    .init = initSadMac,
    .send = sendSadMac,
    .poll = pollSadMac,
    .buildHeader = defaultBuildHeader,
    .isKnownDstAddress = defaultIsKnownDstAddress,
    .recvCb = networkingForwardData,
};

static uint8_t lastNexthop = 0xff;

#ifdef USE_ROLE_BASE_STATION
#define DELAYED_SEND 0
#else
#define DELAYED_SEND 1
#endif

#if DELAYED_SEND
static uint8_t delayedData[RADIO_MAX_PACKET];
static volatile uint16_t delayedDataLength;
static volatile uint8_t delayedNexthop;

static Alarm_t delayTimer;

static void delayTimerCb(void *);
#endif

// -----------------------------------------------

static void initSadMac(RecvFunction recvCb) {
    // use crc8 of localAddress as the address on AMB8420
    uint8_t calcAddress = crc8((const uint8_t *) &localAddress, sizeof(localAddress));
    if (calcAddress == 0xff) calcAddress = 0xfe; // avoid using the broadcast address
#if USE_PRINT
    PRINTF("Using 0x%02x as radio address\n", calcAddress);
#endif
    amb8420EnterAddressingMode(AMB8420_ADDR_MODE_ADDR, calcAddress);

#if DELAYED_SEND
    alarmInit(&delayTimer, delayTimerCb, NULL);
#endif
}

static int8_t sendSadMac(MacInfo_t *mi, const uint8_t *data, uint16_t length) {
    int8_t ret;
#if DELAYED_SEND
    if (mi->timeWhenSend) {
        // PRINTF("delayed send!\n");
        if (delayedDataLength) {
            return -1; // busy
        }
        uint32_t now = getSyncTimeMs();
        if (timeAfter32(mi->timeWhenSend, now)) {
            // PRINTF("delayed send after %ld\n", mi->timeWhenSend - now);
            uint16_t nh = getNexthop(mi);
            if (nh == MOS_ADDR_BROADCAST) {
                delayedNexthop = 0xff;
            } else {
                delayedNexthop = crc8((const uint8_t *) &nh, sizeof(nh));
            }
            memcpy(delayedData, mi->macHeader, mi->macHeaderLen);
            delayedDataLength = mi->macHeaderLen;
            memcpy(delayedData + delayedDataLength, data, length);
            delayedDataLength += length;
            alarmSchedule(&delayTimer, mi->timeWhenSend - now);
            return length;
        }
    }
#endif
    // use least significant byte
    uint16_t nh = getNexthop(mi);
    uint8_t nhAddr;
    if (nh == MOS_ADDR_BROADCAST) {
        nhAddr = 0xff;
    } else {
        nhAddr = crc8((const uint8_t *) &nh, sizeof(nh));
    }
    if (lastNexthop != nhAddr) {
        amb8420SetDstAddress(nhAddr);
        lastNexthop = nhAddr;
    }
    // PRINTF("%lu: mac tx %u bytes\n", getSyncTimeMs(), length);
    INC_NETSTAT(NETSTAT_RADIO_TX, EMPTY_ADDR);
    ret = radioSendHeader(mi->macHeader, mi->macHeaderLen, data, length);
    if (ret) return ret;
    return length;
}

#if DELAYED_SEND
static void delayTimerCb(void *unused)
{
    // PRINTF("%lu: mac tx %u bytes\n", getSyncTimeMs(), delayedDataLength);
    // redLedToggle();
    INC_NETSTAT(NETSTAT_RADIO_TX, EMPTY_ADDR);
#ifndef PLATFORM_ARDUINO
    if (lastNexthop != delayedNexthop) {
        amb8420SetDstAddress(delayedNexthop);
        lastNexthop = delayedNexthop;
    }
#endif
    radioSendHeader(NULL, 0, delayedData, delayedDataLength);
    delayedDataLength = 0;
}
#endif

#if TEST_FILTERS

//
// filter out unwanted communications
//
static bool filterPass(MacInfo_t *mi, uint16_t len)
{
    // TPRINTF("rx %u bytes from %#04x (%#04x)\n",
    //         len, mi->immedSrc.shortAddr, mi->originalSrc.shortAddr);

    if (!mi->immedSrc.shortAddr) return true; // XXX

#if SINGLE_HOP
    return true;
#endif

#define BASE_STATION_ADDRESS 0x0001
#define FORWARDER_ADDRESS    0x15CE
#define COLLECTOR_ADDRESS    0x0871

#define FORWARDER_ADDRESS1   0x2
#define FORWARDER_ADDRESS2   0x3

#define COLLECTOR_ADDRESS1   0x4
#define COLLECTOR_ADDRESS2   0x5

#if 0
    // network with all four mote roles, two intermediate hops
    switch (localAddress) {
    case BASE_STATION_ADDRESS:
        if (mi->immedSrc.shortAddr != FORWARDER_ADDRESS) return false;
        break;
    case FORWARDER_ADDRESS:
        if (mi->immedSrc.shortAddr != BASE_STATION_ADDRESS
                && mi->immedSrc.shortAddr != COLLECTOR_ADDRESS) return false;
        break;
    case COLLECTOR_ADDRESS:
        if (mi->immedSrc.shortAddr == BASE_STATION_ADDRESS) return false;
        break;
    default: // mote
        if (mi->immedSrc.shortAddr != COLLECTOR_ADDRESS) return false;
        break;
    }
#elif 1
    // network with all four mote roles, two intermediate hops, multipath
    switch (localAddress) {
    case BASE_STATION_ADDRESS:
        if (mi->immedSrc.shortAddr != FORWARDER_ADDRESS1
                && mi->immedSrc.shortAddr != FORWARDER_ADDRESS2) return false;
        break;
    case FORWARDER_ADDRESS1:
    case FORWARDER_ADDRESS2:
        if (mi->immedSrc.shortAddr != BASE_STATION_ADDRESS
                && mi->immedSrc.shortAddr != COLLECTOR_ADDRESS1
                && mi->immedSrc.shortAddr != COLLECTOR_ADDRESS2) return false;
        break;
    case COLLECTOR_ADDRESS1:
        if (mi->immedSrc.shortAddr == BASE_STATION_ADDRESS
                || mi->immedSrc.shortAddr == COLLECTOR_ADDRESS2) return false;
        break;
    case COLLECTOR_ADDRESS2:
        if (mi->immedSrc.shortAddr == BASE_STATION_ADDRESS
                || mi->immedSrc.shortAddr == COLLECTOR_ADDRESS1) return false;
        break;
    default: // mote
        if (mi->immedSrc.shortAddr != COLLECTOR_ADDRESS1
                && mi->immedSrc.shortAddr != COLLECTOR_ADDRESS2) return false;
        break;
    }
#elif 0
    // smaller network with three mote roles, one intermediate hop
    switch (localAddress) {
    case BASE_STATION_ADDRESS:
        if (mi->immedSrc.shortAddr != COLLECTOR_ADDRESS) return false;
        break;
    case COLLECTOR_ADDRESS:
        // receive from all
        break;
    default:
        if (mi->immedSrc.shortAddr != COLLECTOR_ADDRESS) return false;
        break;
    }
#elif 1
    // smaller network with three mote roles, one intermediate hop (two collectors)
    switch (localAddress) {
    case BASE_STATION_ADDRESS:
        if (mi->immedSrc.shortAddr != COLLECTOR_ADDRESS1
                && mi->immedSrc.shortAddr != COLLECTOR_ADDRESS2) return false;
        break;
    case COLLECTOR_ADDRESS1:
    case COLLECTOR_ADDRESS2:
        // receive from all
        break;
    default:
        if (mi->immedSrc.shortAddr != COLLECTOR_ADDRESS1
                && mi->immedSrc.shortAddr != COLLECTOR_ADDRESS2) return false;
        break;
    }
#else
    // network with direct base station reachability, do nothing
#endif
    return true;
}

#endif

static void pollSadMac(void)
{
    // XXX: stack overflow possible if stack size is too small!
    MacInfo_t mi;
        
    INC_NETSTAT(NETSTAT_RADIO_RX, EMPTY_ADDR);
    if (isRadioPacketReceived()) {
        if (macProtocol.recvCb) {
            uint8_t *data = defaultParseHeader(radioPacketBuffer->buffer,
                    radioPacketBuffer->receivedLength, &mi);

            // TPRINTF("mac rx %u bytes from %#04x\n",
            //         radioPacketBuffer->receivedLength,
            //         mi.immedSrc.shortAddr);

#if TEST_FILTERS
            if (!filterPass(&mi, radioPacketBuffer->receivedLength)) {
                // PRINTF("  filtered out\n");
                data = NULL;
            }
#endif
            if (data) {
                //INC_NETSTAT(NETSTAT_PACKETS_RECV, mi.originalSrc.shortAddr);  // done @dv.c
                // call user callback
                macProtocol.recvCb(&mi, data,
                        radioPacketBuffer->receivedLength - mi.macHeaderLen);
            } else {
                // PRINTF("no data\n");
                INC_NETSTAT(NETSTAT_PACKETS_DROPPED_RX, EMPTY_ADDR);
            }
        } else {
            // PRINTF("no cb\n");
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
