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

#include "comm.h"
#include "routing.h"
#include "socket.h"
#include "queue.h"
#include <hil/snum.h>
#include <lib/dprint.h>
#include <net/net-stats.h>

// ---------- place for global net variables

MosShortAddr localAddress;

#ifdef DEBUG
uint32_t netstats[TOTAL_NETSTAT];
#endif

// -----------------------------------

void initComm(void) {
    queueInit();
    initArchComm();
    macProtocol.init(commForwardData);
    socketsInit();
    initRouting();
}

void fillLocalAddress(MosAddr *result) {
    if (localAddress) {
        result->type = MOS_ADDR_TYPE_SHORT;
        result->shortAddr = localAddress;
    } else {
        result->type = MOS_ADDR_TYPE_LONG;
        halGetSerialNumber(result->longAddr);
    }
}

bool isLocalAddress(MosAddr *addr) {
    switch (addr->type) {
    case MOS_ADDR_TYPE_SHORT:
        return addr->shortAddr == localAddress;
    case MOS_ADDR_TYPE_LONG:
        return halSerialNumberMatches(addr->longAddr);
    default:
        return false;
    }
}

#define NUM_SENDERS 10

typedef struct Sender_s {
    uint32_t address;
    uint32_t timestamp;
} Sender_t;

Sender_t senders[NUM_SENDERS];

static bool findDuplicate(uint16_t address, uint32_t timestamp)
{
    uint16_t i;
    for (i = 0; i < NUM_SENDERS; ++i) {
        if (senders[i].address == address) {
            if (senders[i].timestamp == timestamp) {
                return true;
            }
            senders[i].timestamp = timestamp;
            return false;
        }
    }

    for (i = 0; i < NUM_SENDERS; ++i) {
        if (senders[i].address == 0) {
            senders[i].address = address;
            senders[i].timestamp = timestamp;
            break;
        }
    }
    return false;
}

static bool isDuplicate(MacInfo_t *macInfo, uint8_t *data, uint16_t len)
{
    uint32_t timestamp;
    if (len < sizeof(timestamp)) return false;
    memcpy(&timestamp, data, sizeof(timestamp));
    return findDuplicate(macInfo->originalSrc.shortAddr, timestamp);
}

// send smth to address 'addr', port 'port' 
void commForwardData(MacInfo_t *macInfo, uint8_t *data, uint16_t len) {
    // PRINT("commForwardData\n");

    switch (routePacket(macInfo)) {
    case RD_DROP:
        //PRINTF("RD_DROP\n");
        if (IS_LOCAL(macInfo)){
            INC_NETSTAT(NETSTAT_PACKETS_DROPPED_TX, EMPTY_ADDR);
        } else{
            INC_NETSTAT(NETSTAT_PACKETS_DROPPED_RX, EMPTY_ADDR);
        }
        break;

    case RD_LOCAL:
        //PRINTF("RD_LOCAL\n");
        socketInputData(macInfo, data, len);
        break;

    case RD_UNICAST:
        //PRINTF("RD_UNICAST\n");
        // force header rebuild
        macInfo->macHeaderLen = 0;
        // XXX: for SAD
        if (!IS_LOCAL(macInfo) && isDuplicate(macInfo, data, len)) {
            PRINTF("not forwarding, duplicate...\n");
            break;
        }
        if (IS_LOCAL(macInfo)){
            INC_NETSTAT(NETSTAT_PACKETS_SENT, macInfo->originalDst.shortAddr);
        }
        macSendEx(macInfo, data, len);
        break;

    case RD_BROADCAST:
        //PRINTF("RD_BROADCAST\n");
        if (!IS_LOCAL(macInfo)) {
            socketInputData(macInfo, data, len);
        }
        // and forward to all
        intToAddr(macInfo->originalDst, MOS_ADDR_BROADCAST);
        // force header rebuild
        macInfo->macHeaderLen = 0;
        INC_NETSTAT(NETSTAT_PACKETS_SENT, EMPTY_ADDR);
        macSendEx(macInfo, data, len);
        break;
    }
}
