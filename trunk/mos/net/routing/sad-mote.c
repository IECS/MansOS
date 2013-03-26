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
// SAD routing, mote (data source) functionality
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

static void roRequestTimerCb(void *);
static void routingReceive(Socket_t *s, uint8_t *data, uint16_t len);

static Seqnum_t lastSeenSeqnum;
static uint8_t hopCountToRoot = MAX_HOP_COUNT;
static uint32_t lastRootMessageTime;
static MosShortAddr nexthopToRoot;
static uint8_t moteNumber;

static bool isListening;

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
    // leave 5 seconds for fwd stage
    uint32_t t = 5000;
    // leave 1 second for each mote
    t += 1000 * moteNumber;

    uint32_t now = getSyncTimeMs() % SAD_SUPERFRAME_LENGTH;

    if (t < now) {
        if (t + 1000 > now) {
            return 0; // send immediately
        }
        t += SAD_SUPERFRAME_LENGTH;
    }
    t -= now;
    // add random jitter [300..900]
    t += randomInRange(300, 900);
    return t + getSyncTimeMs();
}

static uint32_t calcListenStartTime(void)
{
    // leave 5 seconds for fwd stage
    uint32_t t = 5000;
    // leave 1 second for each mote
    t += 1000 * moteNumber;
    t = SAD_SUPERFRAME_LENGTH - t;

    //PRINTF("  l=%lu, t=%lu\n", SAD_SUPERFRAME_LENGTH, t);

    uint32_t toEnd = SAD_SUPERFRAME_LENGTH - getSyncTimeMs() % SAD_SUPERFRAME_LENGTH;
    if (toEnd < t + TOTAL_LISTENING_TIME) toEnd += SAD_SUPERFRAME_LENGTH;
    //PRINTF("  toEnd=%lu\n", toEnd);
    t = toEnd - t;
    //PRINTF("  result=%lu\n", t < TIMESLOT_IMPRECISION ? 0 : t - TIMESLOT_IMPRECISION);
    if (t < TIMESLOT_IMPRECISION) return 0;
    return t - TIMESLOT_IMPRECISION;
}

void routingInit(void)
{
    socketOpen(&roSocket, routingReceive);
    socketBind(&roSocket, ROUTING_PROTOCOL_PORT);
    socketSetDstAddress(&roSocket, MOS_ADDR_BROADCAST);

    alarmInit(&roRequestTimer, roRequestTimerCb, NULL);
    alarmInit(&roStopListeningTimer, roStopListeningTimerCb, NULL);
    alarmInit(&roStartListeningTimer, roStartListeningTimerCb, NULL);
    alarmSchedule(&roRequestTimer, randomInRange(2000, 3000));
    alarmSchedule(&roStartListeningTimer, 110);
}

static void roStartListeningTimerCb(void *x)
{
    alarmSchedule(&roStartListeningTimer, calcListenStartTime());

    // listen to info only when routing info is already valid (?)
    if (isRoutingInfoValid()) {
        RPRINTF("%lu: --- START LISTENING\n", getSyncTimeMs());
        isListening = true;
        radioOn();
        alarmSchedule(&roStopListeningTimer, TOTAL_LISTENING_TIME);
    }
}

static void roStopListeningTimerCb(void *x)
{
    RPRINTF("%lu: turn radio off\n", getSyncTimeMs());
    RADIO_OFF_ENERGSAVE();
    isListening = false;
}

static void roRequestTimerCb(void *x)
{
    alarmSchedule(&roRequestTimer, ROUTING_REQUEST_TIMEOUT + randomNumberBounded(1000));

    if (isRoutingInfoValid()) {
        return;
    }

    RPRINTF("send routing request\n");

    radioOn(); // wait for response
    isListening = true;

    RoutingRequestPacket_t req;
    req.packetType = ROUTING_REQUEST;
    req.senderType = SENDER_MOTE;
    socketSend(&roSocket, &req, sizeof(req));

    alarmSchedule(&roStopListeningTimer, ROUTING_REPLY_WAIT_TIMEOUT);
}

static void routingReceive(Socket_t *s, uint8_t *data, uint16_t len)
{
    // RPRINTF("routingReceive %d bytes from %#04x\n", len,
    //         s->recvMacInfo->originalSrc.shortAddr);

    if (len == 0) {
        RPRINTF("routingReceive: no data!\n");
        return;
    }

#if PRECONFIGURED_NH_TO_ROOT
    if (s->recvMacInfo->originalSrc.shortAddr != PRECONFIGURED_NH_TO_ROOT) {
        RPRINTF("Dropping routing info: not from the nexthop, but from %#04x\n",
                s->recvMacInfo->originalSrc.shortAddr);
        return;
    }
    RPRINTF("Got routing info from the nexthop\n");
#endif

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

    bool update = false;
    if (!isRoutingInfoValid() || timeAfter16(ri.seqnum, lastSeenSeqnum)) {
        // XXX: theoretically should add some time to avoid switching to
        // worse path only because packets from it travel faster
        update = true;
        //RPRINTF("update routing info - more recent seqnum\n");
    } else if (ri.seqnum == lastSeenSeqnum) {
        if (ri.hopCount < hopCountToRoot) {
            update = true;
            //RPRINTF("update routing info - better metric\n");
        }
    }
    if (ri.hopCount > MAX_HOP_COUNT) update = false;

    if (update) {
        PRINTF("got valid routing info\n");
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
        PRINTF("%lu: OK!%s\n", getSyncTimeSec(), isListening ? "" : " (not listening)");
    }
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
        RPRINTF("root routing info not present or expired!\n");
        return RD_DROP;
    }
    //RPRINTF("using 0x%04x as nexthop to root\n", nexthopToRoot);
    info->immedDst.shortAddr = nexthopToRoot;
    // delay until our frame
    info->timeWhenSend = calcSendTime();
    return RD_UNICAST;
}
