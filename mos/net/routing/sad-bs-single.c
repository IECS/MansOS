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

//
// SAD routing, base station functionality (single hop network)
//

#include "../mac.h"
#include "../routing.h"
#include "../socket.h"
#include <alarms.h>
#include <timing.h>
#include <random.h>
#include <print.h>
#include <net/net_stats.h>

static Socket_t roSocket;
static Alarm_t roOriginateTimer;
static Seqnum_t mySeqnum;

static void roOriginateTimerCb(void *);
static void routingReceive(Socket_t *s, uint8_t *data, uint16_t len);

uint64_t lastRootSyncMilliseconds;
uint64_t lastRootClockMilliseconds;

// this can serve just one downstream; use this specific address as the nexthop!
static uint16_t downstreamAddress = MOS_ADDR_BROADCAST;

// -----------------------------------------------

void routingInit(void) {
    socketOpen(&roSocket, routingReceive);
    socketBind(&roSocket, ROUTING_PROTOCOL_PORT);
    socketSetDstAddress(&roSocket, MOS_ADDR_BROADCAST);

    alarmInit(&roOriginateTimer, roOriginateTimerCb, NULL);

    radioOn();
}

static void roOriginateTimerCb(void *x) {
    PRINTF("originate routing packet, downstreamAddress=%#04x\n", downstreamAddress);

    RoutingInfoPacket_t routingInfo;
    routingInfo.packetType = ROUTING_INFORMATION;
    routingInfo.senderType = SENDER_BS;
    routingInfo.rootAddress = localAddress;
    routingInfo.hopCount = 1;
    routingInfo.seqnum = ++mySeqnum;
    routingInfo.moteNumber = 0;
    if (lastRootSyncMilliseconds) {
        routingInfo.rootClockMs = lastRootClockMilliseconds
                + (getTimeMs64() - lastRootSyncMilliseconds);
    } else {
        routingInfo.rootClockMs = getTimeMs64();
    }

    socketSendEx(&roSocket, &routingInfo, sizeof(routingInfo), downstreamAddress);
}

static void routingReceive(Socket_t *s, uint8_t *data, uint16_t len)
{
    PRINTF("BS: routingReceive %d bytes\n", len);
    if (len < 2) {
        PRINTF("routingReceive: too short!\n");
        return;
    }

    uint8_t type = *data;
    if (type == ROUTING_REQUEST) {
        if (s->recvMacInfo->originalSrc.shortAddr) {
            downstreamAddress = s->recvMacInfo->originalSrc.shortAddr;
        }
        alarmSchedule(&roOriginateTimer, 1);
    }
}

RoutingDecision_e routePacket(MacInfo_t *info) {
    // This is simple. Base station never forwards packets,
    // just sends and receives.
    MosAddr *dst = &info->originalDst;
    if (!IS_LOCAL(info) && info->immedSrc.shortAddr) {
        downstreamAddress = info->immedSrc.shortAddr;
        // reply to the packet with routing info as soon as possible
        alarmSchedule(&roOriginateTimer, 1);
    }
    // TODO: move this!!!
    fillLocalAddress(&info->immedSrc);

    // PRINTF("dst address=0x%04x, nexthop=0x%04x\n", dst->shortAddr,
    //         info->immedDst.shortAddr);
    // PRINTF("  localAddress=0x%04x\n", localAddress);

    if (isLocalAddress(dst)) {
        INC_NETSTAT(NETSTAT_PACKETS_RECV, info->originalSrc.shortAddr);
        return RD_LOCAL;
    }
    // allow to receive packets sent to "root" (0x0000) too
    if (isBroadcast(dst) || isUnspecified(dst)) {
        if (!IS_LOCAL(info)){
            INC_NETSTAT(NETSTAT_PACKETS_RECV, info->originalSrc.shortAddr);
        }
        // don't forward broadcast packets
        return IS_LOCAL(info) ? RD_BROADCAST : RD_LOCAL;
    }
    // don't forward unicast packets either
    return IS_LOCAL(info) ? RD_UNICAST : RD_DROP;
}
