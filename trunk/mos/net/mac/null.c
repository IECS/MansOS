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

//
// NULL MAC protocol.
// Directly bridges upper-layer network stack with a radio driver.
// Does not add any network addressing information!
//

#include "../mac.h"
#include <lib/buffer.h>
#include <radio.h>
#include <errors.h>
#include <print.h>
#include <net/radio_packet_buffer.h>
#include <net/net-stats.h>

#if DEBUG
#define MAC_DEBUG 1
#endif

#if MAC_DEBUG
#include "dprint.h"
#define MPRINTF(...) PRINTF(__VA_ARGS__)
#else
#define MPRINTF(...) do {} while (0)
#endif


static void initNullMac(RecvFunction cb);
static int8_t sendNullMac(MacInfo_t *mi, const uint8_t *data, uint16_t length);
static void pollNullMac(void);
static bool nullMacBuildHeader(MacInfo_t *mi, const uint8_t **header /* out */,
        uint16_t *headerLength /* out */);

MacProtocol_t macProtocol = {
    .name = MAC_PROTOCOL_NULL,
    .init = initNullMac,
    .send = sendNullMac,
    .poll = pollNullMac,
    .buildHeader = nullMacBuildHeader,
    .isKnownDstAddress = defaultIsKnownDstAddress,
};

static void initNullMac(RecvFunction cb)
{
    macProtocol.recvCb = cb;
    // turn on radio listening
    radioOn();
}

static int8_t sendNullMac(MacInfo_t *mi, const uint8_t *data, uint16_t length) {
    INC_NETSTAT(NETSTAT_RADIO_TX, EMPTY_ADDR);
    int8_t ret = radioSend(data, length);
    if (ret == 0) ret = length;
    return ret;
}

static void pollNullMac(void) {
    MacInfo_t mi;
    memset(&mi, 0, sizeof(mi));
    INC_NETSTAT(NETSTAT_RADIO_RX, EMPTY_ADDR);
    if (isRadioPacketReceived()) {
//        MPRINTF("got a packet from radio, size=%u, first bytes=0x%02x 0x%02x 0x%02x 0x%02x\n",
//                 radioPacketBuffer->receivedLength,
//                 radioPacketBuffer->buffer[0], radioPacketBuffer->buffer[1],
//                 radioPacketBuffer->buffer[2], radioPacketBuffer->buffer[3]);
        if (macProtocol.recvCb) {
            //INC_NETSTAT(NETSTAT_PACKETS_RECV, EMPTY_ADDR);    // done @ dv.c
            // call user callback
            macProtocol.recvCb(&mi, radioPacketBuffer->buffer, radioPacketBuffer->receivedLength);
        } else{
            INC_NETSTAT(NETSTAT_PACKETS_DROPPED_RX, EMPTY_ADDR);
        }
    }
    else if (isRadioPacketError()) {
        INC_NETSTAT(NETSTAT_PACKETS_DROPPED_RX, EMPTY_ADDR);
        MPRINTF("got an error from radio: %s\n",
                strerror(-radioPacketBuffer->receivedLength));
    }
    radioBufferReset();
}

static bool nullMacBuildHeader(MacInfo_t *mi, const uint8_t **header /* out */, uint16_t *headerLength /* out */) {
    // null MAC has no header
    *header = NULL;
    headerLength = 0;
    return true;
}
