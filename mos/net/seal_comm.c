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

#include "seal_comm.h"
#include "addr.h"

#define UNUSED 0xffff

#if DEBUG
#define SEAL_DEBUG 1
#endif

#if SEAL_DEBUG
#include <lib/dprint.h>
#define DPRINTF(...) PRINTF(__VA_ARGS__)
#else
#define DPRINTF(...) do {} while (0)
#endif

// TODO: use a mac protocol for energy saving!
#define USE_MAC 0

static SealCommListener_t listeners[MAX_SEAL_LISTENERS];

static void sealRecv(MacInfo_t *mi, uint8_t *data, uint16_t length);

#define for_all_listeners(op)                                           \
    do {                                                                \
        const SealCommListener_t *end = listeners + MAX_SEAL_LISTENERS; \
        const SealCommListener_t *l = listeners;                        \
        for (; l != end; ++l) { op; }                                   \
    } while (0)

#if !USE_MAC
static void sealRecvRaw(void)
{
    static uint8_t buffer[RADIO_MAX_PACKET];
    int16_t length = radioRecv(buffer, sizeof(buffer));
    sealRecv(NULL, buffer, length);
}
#endif

void sealCommInit(void)
{
#if USE_MAC
    // use a mac protocol
    mac = getSimpleMac();
    mac->init(NULL, false, macBuffer, sizeof(macBuffer));
    mac->recvCb = sealRecv;
#else
    // use raw radio interface
    radioSetReceiveHandle(sealRecvRaw);
    radioOn();
#endif
    for_all_listeners(l->code = UNUSED);
}

static inline void rxSensorValue(uint16_t code, uint32_t value)
{
    for_all_listeners(
            if (l->code == code) {
                l->value = value;
                l->callback(value);
            });
}

static void sealRecv(MacInfo_t *mi, uint8_t *data, uint16_t length)
{
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
    uint16_t valueOffset = sizeof(SealHeader_t);
    while (typeMask & (1 << 31)) {
        // TODO: check len
        memcpy(&typeMask, data + valueOffset, sizeof(typeMask));
        valueOffset += sizeof(typeMask);
    }

    uint16_t code;
    typeMask = h.typeMask;
    for (code = 0; ; code++) {
        uint16_t bit = code % 32;
        if (bit == 31) {
            uint16_t numField = code / 32;
            memcpy(&typeMask, data + sizeof(SealHeader_t) + numField * 4, sizeof(typeMask));
        }
        if (typeMask & (1 << bit)) {
            uint32_t value;
            // TODO: check len
            memcpy(&value, data + valueOffset, sizeof(value));
            valueOffset += sizeof(value)
            rxSensorValue(code, value);
        }
    }
}

// register a listener to specific sensor (determined by code)
bool sealCommRegisterInterest(uint16_t code, CallbackFunction callback)
{
    for_all_listeners(
            if (l->code == UNUSED) {
                l->code = code;
                l->callback = callback;
                return true;
            });
    DPRINTF("sealCommRegisterInterest: all full!\n");
    return false;
}

// unregister a listener
bool sealCommUnregisterInterest(uint16_t code, CallbackFunction callback)
{
    for_all_listeners(
            if (l->code == code && l->callback == callback) {
                l->code = UNUSED;
                return true;
            });
    DPRINTF("sealCommUnregisterInterest: not found!\n");
    return false;
}

void sealCommSendValue(uint16_t code, uint32_t value)
{
    struct SealPacket_s {
        SealHeader_t header;
        uint32_t address;
        uint32_t value;
    } packet;

    ASSERT(code < 31); // XXX TODO: add support for larger codes

    packet.header.magic = SEAL_MAGIC;
    packet.header.typeMask = (1 << code);
    packet.header.typeMask |= (1 << PACKET_FIELD_ID_ADDRESS);
    if (code < PACKET_FIELD_ID_ADDRESS) {
        // hack
        packet.address = value;
        packet.value = localAddress;
    } else {
        packet.address = localAddress;
        packet.value = value;
    }
    packet.header.crc = crc16((const uint8_t *) &packet + 4, sizeof(packet) - 4);

#if USE_MAC
    const static MosAddr bcastAddr = {MOS_ADDR_TYPE_SHORT, {MOS_ADDR_BROADCAST}};
    macSend(&bcastAddr, &packet, sizeof(packet));
#else
    radioSend(&packet, sizeof(packet));
#endif
}

uint32_t sealCommReadValue(uint16_t code) 
{
    for_all_listeners(
            if (l->code == code) {
                return l->value;
            });
    return ~0u;
}
