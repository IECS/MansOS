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
// SAD routing, mote (data source) functionality (single hop network)
//

#include "../mac.h"
#include "../routing.h"
#include "../socket.h"
#include <alarms.h>
#include <timing.h>
#include <print.h>
#include <random.h>
#include <net/net_stats.h>

static Socket_t roSocket;
static Alarm_t roRequestTimer;
static Alarm_t roStartListeningTimer;
static Alarm_t roStopListeningTimer;

static void routingReceive(Socket_t *s, uint8_t *data, uint16_t len);

static Seqnum_t lastSeenSeqnum;
static uint8_t hopCountToRoot = MAX_HOP_COUNT;
static uint32_t lastRootMessageTime;
static MosShortAddr nexthopToRoot = MOS_ADDR_BROADCAST;
static uint8_t moteNumber;

static void roRequestTimerCb(void *);
static void roStartListeningTimerCb(void *);
static void roStopListeningTimerCb(void *);

#if DEBUG
#define ROUTE_DEBUG 1
#endif

#define ROUTE_DEBUG 1

#if ROUTE_DEBUG
#define RPRINTF(...) PRINTF(__VA_ARGS__)
#else
#define RPRINTF(...) do {} while (0)
#endif


// -----------------------------------------------

static inline bool isRoutingInfoValid(void)
{
    if (hopCountToRoot >= MAX_HOP_COUNT) return false;
    bool old = timeAfter32((uint32_t)getJiffies(), lastRootMessageTime + ROUTING_INFO_VALID_TIME);
    if (old) {
        hopCountToRoot = MAX_HOP_COUNT;
        return false;
    }
    return true;
}

static uint32_t calcSendTime(void)
{
    // from 0 to 0xffff miliseconds
    return randomNumber();
}

void routingInit(void)
{
    socketOpen(&roSocket, routingReceive);
    socketBind(&roSocket, ROUTING_PROTOCOL_PORT);
    socketSetDstAddress(&roSocket, MOS_ADDR_BROADCAST);

    alarmInit(&roStopListeningTimer, roStopListeningTimerCb, NULL);
    alarmInit(&roStartListeningTimer, roStartListeningTimerCb, NULL);
    alarmInit(&roRequestTimer, roRequestTimerCb, NULL);

    alarmSchedule(&roRequestTimer, 100);
}

static void roRequestTimerCb(void *x)
{
    // check if already found the info
    if (isRoutingInfoValid()) return;

    // if tried too long, give up
    if (timeAfter(getJiffies(), 1000ul * NETWORK_STARTUP_TIME_SEC)) return;

    alarmSchedule(&roRequestTimer, 9000 + randomNumberBounded(1000));

    RPRINTF("send routing request\n");

    radioOn(); // wait for response

    RoutingRequestPacket_t req;
    req.packetType = ROUTING_REQUEST;
    req.senderType = SENDER_MOTE;
    socketSend(&roSocket, &req, sizeof(req));

    alarmSchedule(&roStopListeningTimer, ROUTING_REPLY_WAIT_TIMEOUT);
}

static void roStartListeningTimerCb(void *x)
{
    TPRINTF("-- start listening\n");
    radioOn();
    alarmSchedule(&roStopListeningTimer, 1000);
}

static void roStopListeningTimerCb(void *x)
{
    TPRINTF("-- stop listening\n");
    radioOff();
}

static void routingReceive(Socket_t *s, uint8_t *data, uint16_t len)
{
    // RPRINTF("routingReceive %d bytes from %#04x\n", len,
    //         s->recvMacInfo->originalSrc.shortAddr);

    if (len == 0) {
        RPRINTF("routingReceive: no data!\n");
        return;
    }

    uint8_t type = data[0];
    if (type == ROUTING_REQUEST) {
        return;
    }

    if (type != ROUTING_INFORMATION) {
        RPRINTF("routingReceive: unknown type!\n");
        return;
    }

    if (len < sizeof(RoutingInfoPacket_t)) {
        RPRINTF("routingReceive: too short for info packet!\n");
        return;
    }

    RoutingInfoPacket_t ri;
    memcpy(&ri, data, sizeof(RoutingInfoPacket_t));

    TPRINTF("got valid routing info\n");
    rootAddress = ri.rootAddress;
    nexthopToRoot = s->recvMacInfo->originalSrc.shortAddr;
    lastSeenSeqnum = ri.seqnum;
    hopCountToRoot = ri.hopCount;
    lastRootMessageTime = (uint32_t)getJiffies();
    int64_t oldRootClockDeltaMs = rootClockDeltaMs;
    rootClockDeltaMs = ri.rootClockMs - getTimeMs64();
    moteNumber = ri.moteNumber;
    // PRINTF("%lu: ++++++++++++ fixed local time\n", getSyncTimeMs());
    PRINTF("delta: old=%ld, new=%ld\n", (int32_t)oldRootClockDeltaMs, (int32_t)rootClockDeltaMs);
    if (abs((int32_t)oldRootClockDeltaMs - (int32_t)rootClockDeltaMs) > 500) {
        PRINTF("large delta=%ld, time sync off?!\n", (int32_t)oldRootClockDeltaMs - (int32_t)rootClockDeltaMs);
    }

    // stop listening immediately
    radioOff();
}

RoutingDecision_e routePacket(MacInfo_t *info)
{
    MosAddr *dst = &info->originalDst;
    fillLocalAddress(&info->immedSrc);

    // RPRINTF("dst address=0x%04x, nexthop=0x%04x\n", dst->shortAddr, info->immedDst.shortAddr);
    // RPRINTF("  localAddress=0x%04x\n", localAddress);

    // fix root address if we are sending it to the root
    if (IS_LOCAL(info) && dst->shortAddr == MOS_ADDR_ROOT) {
        intToAddr(info->originalDst, rootAddress);
        // info->hoplimit = hopCountToRoot;
        info->hoplimit = MAX_HOP_COUNT;
    }

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
    if (!IS_LOCAL(info)) {
        // mote never forwards packets
        return RD_DROP;
    }
    if (!isRoutingInfoValid()) {
        // send anyway
        nexthopToRoot = MOS_ADDR_BROADCAST;
        // RPRINTF("root routing info not present or expired!\n");
        // return RD_DROP;
    }
    //RPRINTF("using 0x%04x as nexthop to root\n", nexthopToRoot);
    info->immedDst.shortAddr = nexthopToRoot;
    // delay until our time
    uint16_t sendTime = calcSendTime();
    info->timeWhenSend = sendTime + getSyncTimeMs();
    alarmSchedule(&roStartListeningTimer, sendTime);
    return RD_UNICAST;
}
