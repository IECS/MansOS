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
// Distance-vector routing, base station algorithm
//

#include "../mac.h"
#include "../routing.h"
#include "../socket.h"
#include <alarms.h>
#include <timing.h>
#include <print.h>
#include <net/net_stats.h>

static Socket_t roSocket;
static Alarm_t roOriginateTimer;
static Seqnum_t mySeqnum;

static void roOriginateTimerCb(void *);
static void routingReceive(Socket_t *s, uint8_t *data, uint16_t len);

uint32_t lastRootSyncMilliseconds;
uint32_t lastRootClockMilliseconds;

// -----------------------------------------------

void initRouting(void) {
    socketOpen(&roSocket, routingReceive);
    socketBind(&roSocket, ROUTING_PROTOCOL_PORT);
    socketSetDstAddress(&roSocket, MOS_ADDR_BROADCAST);

    alarmInit(&roOriginateTimer, roOriginateTimerCb, NULL);
    alarmSchedule(&roOriginateTimer, 500);
}

static void roOriginateTimerCb(void *x) {
    // PRINTF("originate routing packet, root=local=%#04x\n", localAddress);

    RoutingInfoPacket_t routingInfo;
    routingInfo.packetType = ROUTING_INFORMATION;
    routingInfo.senderType = 0;
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

    socketSend(&roSocket, &routingInfo, sizeof(routingInfo));
    alarmSchedule(&roOriginateTimer, ROUTING_ORIGINATE_TIMEOUT);
}

static void routingReceive(Socket_t *s, uint8_t *data, uint16_t len)
{
    // PRINTF("BS: routingReceive %d bytes, ignoring\n", len);
    if (len < 2) {
        PRINTF("routingReceive: too short!\n");
        return;
    }

    uint8_t type = *data;
    if (type == ROUTING_REQUEST) {
        // reschedule the origination timer sooner
        if (getAlarmTime(&roOriginateTimer) > 200) {
            alarmSchedule(&roOriginateTimer, 200);
        }
    }
}

RoutingDecision_e routePacket(MacInfo_t *info) {
    // This is simple. Base station never forwards packets,
    // just sends and receives.
    MosAddr *dst = &info->originalDst;

    // PRINTF("dst address=0x%04x, nexthop=0x%04x\n", dst->shortAddr,
    //         info->immedDst.shortAddr);
    // PRINTF("  localAddress=0x%04x\n", localAddress);

    if (isLocalAddress(dst)) {
        INC_NETSTAT(NETSTAT_PACKETS_RECV, info->originalSrc.shortAddr);
        return RD_LOCAL;
    }
    if (isBroadcast(dst)) {
        if (!IS_LOCAL(info)){
            INC_NETSTAT(NETSTAT_PACKETS_RECV, info->originalSrc.shortAddr);
        }
        // don't forward broadcast packets
        return IS_LOCAL(info) ? RD_BROADCAST : RD_LOCAL;
    }
    // don't forward unicast packets either
    return IS_LOCAL(info) ? RD_UNICAST : RD_DROP;
}
