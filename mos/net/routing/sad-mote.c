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

//
// SAD routing, mote (dats source) functionality
//

#include "../mac.h"
#include "../routing.h"
#include "../socket.h"
#include <hil/alarms.h>
#include <kernel/threads/timing.h>
#include <lib/unaligned.h>
#include <lib/dprint.h>
#include <lib/random.h>
#include <net/net-stats.h>

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
MosShortAddr rootAddress;
int32_t rootClockDelta;
static uint8_t moteNumber;

static void roStartListeningTimerCb(void *);
static void roStopListeningTimerCb(void *);

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

    uint32_t now = getFixedTime() % SAD_SUPERFRAME_LENGTH;

    if (t < now) {
        if (t + 900 > now) {
            return 0; // send immediately
        }
        t += SAD_SUPERFRAME_LENGTH;
    }
    t -= now;
    // add random jitter
    t += randomNumberBounded(1000);
    return t + getFixedTime();
}

static uint32_t calcListenStartTime(void)
{
    // leave 5 seconds for fwd stage
    uint32_t t = 5000;
    // leave 1 second for each mote
    t += 1000 * moteNumber;
    t = SAD_SUPERFRAME_LENGTH - t;

    uint32_t toEnd = SAD_SUPERFRAME_LENGTH - getFixedTime() % SAD_SUPERFRAME_LENGTH;
    if (toEnd < t) toEnd += SAD_SUPERFRAME_LENGTH;
    return toEnd - t;
}

void initRouting(void)
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
    PRINTF("start listening time slot, jiffies=%lu\n", getFixedTime());
    alarmSchedule(&roStartListeningTimer, calcListenStartTime());

    // listen to info only when routing info is already valid (?)
    // if (isRoutingInfoValid()) {
    radioOn();
    alarmSchedule(&roStopListeningTimer, 1000);
}

static void roStopListeningTimerCb(void *x)
{
    RADIO_OFF_ENERGSAVE();
}

static void roRequestTimerCb(void *x)
{
    alarmSchedule(&roRequestTimer, ROUTING_REQUEST_TIMEOUT + randomNumberBounded(1000));

    if (isRoutingInfoValid()) {
        return;
    }

    PRINT("send routing request\n");

    radioOn(); // wait for response

    RoutingRequestPacket_t req;
    req.packetType = ROUTING_REQUEST;
    req.senderType = SENDER_MOTE;
    socketSend(&roSocket, &req, sizeof(req));

    alarmSchedule(&roStopListeningTimer, ROUTING_REPLY_WAIT_TIMEOUT);
}

static void routingReceive(Socket_t *s, uint8_t *data, uint16_t len)
{
    PRINTF("routingReceive %d bytes from %#04x\n", len,
            s->recvMacInfo->originalSrc.shortAddr);

    if (len == 0) {
        PRINT("routingReceive: no data!\n");
        return;
    }

#if PRECONFIGURED_NH_TO_ROOT
    if (s->recvMacInfo->originalSrc.shortAddr != PRECONFIGURED_NH_TO_ROOT) {
        PRINTF("Dropping routing info: not from the nexthop, but from %#04x\n",
                s->recvMacInfo->originalSrc.shortAddr);
        return;
    }
    PRINTF("Got routing info from the nexthop\n");
#endif

    uint8_t type = data[0];
    if (type == ROUTING_REQUEST) {
        return;
    }

    if (type != ROUTING_INFORMATION) {
        PRINT("routingReceive: unknown type!\n");
        return;
    }

    if (len < sizeof(RoutingInfoPacket_t)) {
        PRINT("routingReceive: too short for info packet!\n");
        return;
    }

    RoutingInfoPacket_t ri;
    memcpy(&ri, data, sizeof(RoutingInfoPacket_t));

    bool update = false;
    if (!isRoutingInfoValid() || timeAfter16(ri.seqnum, lastSeenSeqnum)) {
        // XXX: theoretically should add some time to avoid switching to
        // worse path only because packets from it travel faster
        update = true;
        //PRINT("update routing info - more recent seqnum\n");
    } else if (ri.seqnum == lastSeenSeqnum) {
        if (ri.hopCount < hopCountToRoot) {
            update = true;
            //PRINT("update routing info - better metric\n");
        }
    }
    if (ri.hopCount > MAX_HOP_COUNT) update = false;

    if (update) {
        rootAddress = ri.rootAddress;
        nexthopToRoot = s->recvMacInfo->originalSrc.shortAddr;
        lastSeenSeqnum = ri.seqnum;
        hopCountToRoot = ri.hopCount;
        lastRootMessageTime = getJiffies();
        rootClockDelta = (int32_t)(ri.rootClock - getJiffies());
        moteNumber = ri.moteNumber;
    }
}

RoutingDecision_e routePacket(MacInfo_t *info)
{
    MosAddr *dst = &info->originalDst;

    // PRINTF("dst address=0x%04x, nexthop=0x%04x\n", dst->shortAddr, info->immedDst.shortAddr);
    // PRINTF("  localAddress=0x%04x\n", localAddress);

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

    if (dst->shortAddr == rootAddress) {
        if (isRoutingInfoValid()) {
            //PRINTF("using 0x%04x as nexthop to root\n", nexthopToRoot);
            if (!IS_LOCAL(info)) {
                return RD_DROP;
            }
            info->immedDst.shortAddr = nexthopToRoot;
            // delay until our frame
            info->timeWhenSend = calcSendTime();
            return RD_UNICAST;
        } else {
            PRINTF("root routing info not present or expired!\n");
            return RD_DROP;
        }
    }

    if (IS_LOCAL(info)) {
        //INC_NETSTAT(NETSTAT_PACKETS_SENT, dst->shortAddr);        // Done @ comm.c
        // send out even with an unknown nexthop, makes sense?
        return RD_UNICAST;
    }
    return RD_DROP;
}
