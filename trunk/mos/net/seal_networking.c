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

#include "seal_networking.h"
#include "address.h"
#include "socket.h"
#include <lib/codec/crc.h>
#include <lib/assert.h>
#include <print.h>

#if DEBUG
#define SEAL_DEBUG 1
#endif

#if SEAL_DEBUG
#define DPRINTF(...) PRINTF(__VA_ARGS__)
#else
#define DPRINTF(...) do {} while (0)
#endif

// local variables

static SealNetListener_t listeners[MAX_SEAL_LISTENERS];
static SealNetListener_t *listenerBeingProcessed;

static SealPacket_t *packetInProgress;
uint8_t packetInProgressNumFields;

#if USE_NET
static Socket_t socket;
#endif

// local functions

static void sealRecv(uint8_t *data, uint16_t length);

#define for_all_listeners(op)                                           \
    do {                                                                \
        const SealNetListener_t *end = listeners + MAX_SEAL_LISTENERS; \
        SealNetListener_t *l = listeners;                              \
        for (; l != end; ++l) { op; }                                   \
    } while (0)

#if USE_NET
static void sealRecvData(Socket_t *socket, uint8_t *data, uint16_t length)
{
    sealRecv(data, length);
}
#else
static void sealRecvRaw(void)
{
    static uint8_t buffer[RADIO_MAX_PACKET];
    int16_t length = radioRecv(buffer, sizeof(buffer));
    sealRecv(buffer, length);
}
#endif

void sealNetInit(void)
{
#if USE_NET
    // use sockets
    socketOpen(&socket, sealRecvData);
    socketBind(&socket, SEAL_DATA_PORT);
    socketSetDstAddress(&socket, MOS_ADDR_ROOT);
#else
    // use raw radio interface
    radioSetReceiveHandle(sealRecvRaw);
    radioOn();
#endif
}

static inline bool isSingleValued(SealNetListener_t *l)
{
    uint32_t bit;
    uint_t cnt = 0;
    for (bit = 1; bit; bit <<= 1) {
        if (l->typeMask & bit) {
            if (cnt) return false;
            else cnt = 1;
        }
    }
    return true;
}

static void receivePacketData(SealNetListener_t *l, uint32_t typeMask, const uint8_t *data)
{
    uint16_t code = ffs(l->typeMask) - 1; // XXX: warning on msp430
    bool isSingleValued = !(l->typeMask & ~(1 << code));

    int32_t *write;
    if (isSingleValued) write = &l->u.lastValue;
    else write = l->u.buffer;

    uint32_t bit;
    for (bit = 1; ; bit <<= 1) {
        if (typeMask & bit) {
            if (l->typeMask & bit) {
                // read memory can be unaligned!
                memcpy(write, data, 4);
                write++;
            }
            data += 4;
        }
        if (bit == (1ul << 31)) break;
    }
    listenerBeingProcessed = l;
    if (isSingleValued) {
        l->callback.sv(code, l->u.lastValue);
    } else {
        l->callback.mv(l->u.buffer);
    }
    listenerBeingProcessed = NULL;
}

static void sealRecv(uint8_t *data, uint16_t length)
{
    PRINTF("%lu: seal rx\n", (uint32_t) getTimeMs());

    if (length < sizeof(SealHeader_t)) {
        DPRINTF("sealRecv: too short!\n");
        return;
    }
    SealHeader_t h;
    memcpy(&h, data, sizeof(h));
    if (h.magic != SEAL_MAGIC) {
        DPRINTF("sealRecv: wrong magic (%#04x vs %#04x expected)!\n", h.magic, SEAL_MAGIC);
        return;
    }
    uint16_t calcCrc = crc16(data + 4, length - 4);
    if (h.crc != calcCrc) {
        DPRINTF("sealRecv: wrong crc (%#04x vs %#04x expected)!\n", h.crc, calcCrc);
        return;
    }
    uint32_t typeMask = h.typeMask;
    uint8_t valueOffset = sizeof(SealHeader_t);
    while (typeMask & (1ul << 31)) {
        // TODO: support multiple typemask extension!
        // TODO: check len
        memcpy(&typeMask, data + valueOffset, sizeof(typeMask));
        valueOffset += sizeof(typeMask);
    }
    typeMask = h.typeMask;

    PRINTF("^\n"); // XXX TODO: this is SAD specific
    for_all_listeners(
            if (l->typeMask && (l->typeMask & typeMask) == l->typeMask) {
                receivePacketData(l, typeMask, data + valueOffset);
            });
    PRINTF("$\n");
}

bool sealNetPacketRegisterInterest(uint32_t typeMask,
                                    MultiValueCallbackFunction callback,
                                    int32_t *buffer)
{
    for_all_listeners(
            if (l->callback.mv == NULL) {
                l->callback.mv = callback;
                l->typeMask = typeMask;
                l->u.buffer = buffer;
                return true;
            });
    DPRINTF("sealNetRegisterInterest: all full!\n");
    return false;
}

bool sealNetRegisterInterest(uint16_t code,
                              SingleValueCallbackFunction callback)
{
    for_all_listeners(
            if (l->callback.sv == NULL) {
                l->callback.sv = callback;
                l->typeMask = 1 << code;
                l->u.buffer = NULL;
                return true;
            });
    DPRINTF("sealNetRegisterInterest: all full!\n");
    return false;
}

bool sealNetPacketUnregisterInterest(uint32_t typeMask,
                                      MultiValueCallbackFunction callback)
{
    for_all_listeners(
            if (l->typeMask == typeMask && l->callback.mv == callback) {
                l->callback.sv = NULL;
                return true;
            });
    DPRINTF("sealNetUnregisterInterest: not found!\n");
    return false;
}

bool sealNetUnregisterInterest(uint16_t code,
                                SingleValueCallbackFunction callback)
{
    uint32_t typeMask = 1 << code;
    for_all_listeners(
            if (l->typeMask == typeMask && l->callback.sv == callback) {
                l->callback.sv = NULL;
                return true;
            });
    DPRINTF("sealNetUnregisterInterest: not found!\n");
    return false;
}

void sealNetPacketStart(SealPacket_t *buffer)
{
    packetInProgress = buffer;
    packetInProgress->header.magic = SEAL_MAGIC;
    packetInProgress->header.typeMask = 0;
    packetInProgressNumFields = 0;
}

void sealNetPacketAddField(uint16_t code, int32_t value)
{
    ASSERT(packetInProgress);
    ASSERT(code < 31); // XXX TODO: add support for larger codes

    packetInProgress->header.typeMask |= (1 << code);
    memcpy(packetInProgress->fields + packetInProgressNumFields, &value, sizeof(value));
    packetInProgressNumFields++;
}

void sealNetPacketFinish(void)
{
    ASSERT(packetInProgress);
    uint16_t length = 8 + packetInProgressNumFields * 4;
    packetInProgress->header.crc = 
            crc16((const uint8_t *) packetInProgress + 4, length - 4);

#if USE_NET
    socketSend(&socket, packetInProgress, length);
#else
    radioSend(packetInProgress, length);
#endif
}

void sealNetSendValue(uint16_t code, int32_t value)
{
    SealPacket_t packet;
    sealNetPacketStart(&packet);
    sealNetPacketAddField(code, value);
    sealNetPacketFinish();
}

int32_t sealNetReadValue(uint16_t code) 
{
    ASSERT(listenerBeingProcessed);
    ASSERT(listenerBeingProcessed->typeMask & (1 << code));

    int32_t *read;
    if (isSingleValued(listenerBeingProcessed)) {
        read = &listenerBeingProcessed->u.lastValue;
    } else {
        read = listenerBeingProcessed->u.buffer;
    }
    uint32_t bit;
    for (bit = 1; bit; bit <<= 1) {
        if (bit == 1ul << code) {
            return *read;
        }
        if (listenerBeingProcessed->typeMask & bit) {
            read++;
        }
    }
    ASSERT(false);
    return 0;
}
