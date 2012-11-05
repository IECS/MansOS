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
// SAD routing, forwarder functionality
//


//
// TODO: turn on/off listening
//

#include "../mac.h"
#include "../routing.h"
#include "../socket.h"
#include <alarms.h>
#include <kernel/threads/timing.h>
#include <lib/unaligned.h>
#include <print.h>
#include <random.h>
#include <net/net-stats.h>

static Socket_t roSocket;
static Alarm_t roForwardTimer;
static Alarm_t roRequestTimer;
static Alarm_t roStartListeningTimer;
static Alarm_t roStopListeningTimer;

static void roForwardTimerCb(void *);
static void roRequestTimerCb(void *);
static void routingReceive(Socket_t *s, uint8_t *data, uint16_t len);

static Seqnum_t lastSeenSeqnum;
static uint8_t hopCountToRoot = MAX_HOP_COUNT;
static uint32_t lastRootMessageTime = (uint32_t) -ROUTING_INFO_VALID_TIME;
static MosShortAddr nexthopToRoot;

static void roStartListeningTimerCb(void *);
static void roStopListeningTimerCb(void *);

static bool gotRreq;

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

static uint32_t calcListenStartTime(void)
{
    // at start of frame
    return SAD_SUPERFRAME_LENGTH - getFixedTime() % SAD_SUPERFRAME_LENGTH;
}

static uint32_t calcNextForwardTime(void)
{
    uint32_t t = getFixedTime() % SAD_SUPERFRAME_LENGTH;
    t = SAD_SUPERFRAME_LENGTH - t;
    // add 1 second + random jitter
    t += randomInRange(1000, 2000);
    return t;
}

static void roStartListeningTimerCb(void *x)
{
    PRINTF("%lu: LISTEN START\n", getFixedTime());
    alarmSchedule(&roStartListeningTimer, calcListenStartTime());

    radioOn();
    alarmSchedule(&roStopListeningTimer, 5000 + 1000 * MAX_MOTES);
}

static void roStopListeningTimerCb(void *x)
{
    PRINTF("%lu: turn radio off\n", getFixedTime());
    RADIO_OFF_ENERGSAVE();
}

void initRouting(void)
{
    socketOpen(&roSocket, routingReceive);
    socketBind(&roSocket, ROUTING_PROTOCOL_PORT);
    socketSetDstAddress(&roSocket, MOS_ADDR_BROADCAST);

    alarmInit(&roForwardTimer, roForwardTimerCb, NULL);
    alarmInit(&roRequestTimer, roRequestTimerCb, NULL);
    alarmInit(&roStopListeningTimer, roStopListeningTimerCb, NULL);
    alarmInit(&roStartListeningTimer, roStartListeningTimerCb, NULL);
    alarmSchedule(&roRequestTimer, randomInRange(2000, 3000));
    alarmSchedule(&roForwardTimer, calcNextForwardTime());
    alarmSchedule(&roStartListeningTimer, 90);
}

static void roForwardTimerCb(void *x)
{
    gotRreq = false;

    alarmSchedule(&roForwardTimer, calcNextForwardTime());

    if (hopCountToRoot >= MAX_HOP_COUNT) return;
    if (!isRoutingInfoValid()) {
        roRequestTimerCb(NULL); // enter "search for root" state
        return;
    }

//    PRINTF("%lu: forward routing packet\n", getFixedTime());

    RoutingInfoPacket_t routingInfo;
    routingInfo.packetType = ROUTING_INFORMATION;
    routingInfo.senderType = SENDER_FORWARDER;
    routingInfo.rootAddress = rootAddress;
    routingInfo.hopCount = hopCountToRoot + 1;
    routingInfo.seqnum = lastSeenSeqnum;
//    routingInfo.rootClock = getFixedTime() + RADIO_TX_TIME;
    routingInfo.rootClock = getFixedUptime();
    routingInfo.moteNumber = 0;

    // XXX: INC_NETSTAT(NETSTAT_PACKETS_SENT, EMPTY_ADDR);
    socketSend(&roSocket, &routingInfo, sizeof(routingInfo));
}

static void roRequestTimerCb(void *x)
{
    alarmSchedule(&roRequestTimer, ROUTING_REQUEST_TIMEOUT + randomNumberBounded(1000));

    if (isRoutingInfoValid()) {
        return;
    }

    PRINTF("send routing request\n");

    radioOn(); // wait for response

    RoutingRequestPacket_t req;
    req.packetType = ROUTING_REQUEST;
    req.senderType = SENDER_FORWARDER;
    socketSend(&roSocket, &req, sizeof(req));

    alarmSchedule(&roStopListeningTimer, ROUTING_REPLY_WAIT_TIMEOUT);
}

static void routingReceive(Socket_t *s, uint8_t *data, uint16_t len)
{
    // PRINTF("routingReceive %d bytes from %#04x\n", len,
    //         s->recvMacInfo->originalSrc.shortAddr);

    if (len == 0) {
        PRINTF("routingReceive: no data!\n");
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
        if (!gotRreq) {
            gotRreq = true;
            alarmSchedule(&roForwardTimer, randomInRange(100, 400));
        }
        return;
    }

    if (type != ROUTING_INFORMATION) {
        PRINTF("routingReceive: unknown type!\n");
        return;
    }

    if (len < sizeof(RoutingInfoPacket_t)) {
        PRINTF("routingReceive: too short for info packet!\n");
        return;
    }

    RoutingInfoPacket_t ri;
    memcpy(&ri, data, sizeof(RoutingInfoPacket_t));

    bool update = false;
    if (!isRoutingInfoValid() || timeAfter16(ri.seqnum, lastSeenSeqnum)) {
        // XXX: theoretically should add some time to avoid switching to
        // worse path only because packets from it travel faster
        update = true;
        //PRINTF("update routing info - more recent seqnum\n");
    } else if (ri.seqnum == lastSeenSeqnum) {
        if (ri.hopCount < hopCountToRoot) {
            update = true;
            //PRINTF("update routing info - better metric\n");
        }
    }
    if (ri.hopCount > MAX_HOP_COUNT) update = false;

    if (update) {
        PRINTF("got valid routing info\n");
        rootAddress = ri.rootAddress;
        nexthopToRoot = s->recvMacInfo->originalSrc.shortAddr;
        lastSeenSeqnum = ri.seqnum;
        hopCountToRoot = ri.hopCount;
        lastRootMessageTime = (uint32_t) getJiffies();
//        rootClockDelta = (int32_t)(ri.rootClock - (uint32_t)getJiffies());
        rootClockDelta = (int32_t)(ri.rootClock - getUptime());
        rootClockDeltaMs = rootClockDelta * 1000;
    }
}

static bool checkHoplimit(MacInfo_t *info)
{
    if (IS_LOCAL(info)) return true; // only for forwarding
    if (!info->hoplimit) return true; // hoplimit is optional
    if (--info->hoplimit) return true; // shold be larger than zero after decrement
    return false;
}

RoutingDecision_e routePacket(MacInfo_t *info)
{
    MosAddr *dst = &info->originalDst;
    fillLocalAddress(&info->immedSrc);

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

    // check if hop limit allows the packet to be forwarded
    if (!checkHoplimit(info)) {
        PRINTF("hoplimit reached!\n");
        return RD_DROP;
    }

    if (dst->shortAddr == rootAddress) {
        if (isRoutingInfoValid()) {
            //PRINTF("using 0x%04x as nexthop to root\n", nexthopToRoot);
            if (!IS_LOCAL(info)) {
#if PRECONFIGURED_NH_TO_ROOT
                if (info->immedDst.shortAddr != localAddress) {
                    PRINTF("Dropping, I'm not a nexthop for sender %#04x\n",
                            info->originalSrc.shortAddr);
                    INC_NETSTAT(NETSTAT_PACKETS_DROPPED_RX, EMPTY_ADDR);
                    return RD_DROP;
                }
#endif
                // if (info->hoplimit < hopCountToRoot){
                //     PRINTF("Dropping, can't reach host with left hopCounts\n");
                //     return RD_DROP;
                // } 
                PRINTF("****************** Forwarding a packet to root for %#04x!\n",
                        info->originalSrc.shortAddr);
                // delay a bit
                info->timeWhenSend = getFixedTime() + randomInRange(50, 150);
                INC_NETSTAT(NETSTAT_PACKETS_FWD, nexthopToRoot);
            } else{
                //INC_NETSTAT(NETSTAT_PACKETS_SENT, nexthopToRoot);     // Done @ comm.c
            }
            info->immedDst.shortAddr = nexthopToRoot;
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
