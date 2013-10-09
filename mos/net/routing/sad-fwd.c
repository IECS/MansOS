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
// SAD routing, forwarder functionality
//

#include "../mac.h"
#include "../routing.h"
#include "../socket.h"
#include <alarms.h>
#include <timing.h>
#include <print.h>
#include <random.h>
#include <watchdog.h>
#include <leds.h>
#include <net/net_stats.h>

static Socket_t roSocket;
static Alarm_t roCheckTimer;
static Alarm_t roForwardTimer;
static Alarm_t roRequestTimer;
static Alarm_t roStartListeningTimer;
static Alarm_t roStopListeningTimer;
static Alarm_t roGreenLedTimer;
static Alarm_t watchdogTimer;

static void roCheckTimerCb(void *);
static void roForwardTimerCb(void *);
static void roRequestTimerCb(void *);
static void roGreenLedTimerCb(void *);
static void watchdogTimerCb(void *);
static void routingReceive(Socket_t *s, uint8_t *data, uint16_t len);

static uint32_t routingRequestTimeout = ROUTING_REQUEST_INIT_TIMEOUT;
static bool routingSearching;
static bool isGreenLedOn;

static uint8_t forwardRetries = 1;

static Seqnum_t lastSeenSeqnum;
static uint8_t hopCountToRoot = MAX_HOP_COUNT;
static uint32_t lastRootMessageTime = (uint32_t) -ROUTING_INFO_VALID_TIME;
static MosShortAddr nexthopToRoot;

static void roStartListeningTimerCb(void *);
static void roStopListeningTimerCb(void *);

static bool gotRreq;

// this can serve just one downstream; use this specific address as the nexthop!
static uint16_t downstreamAddress = MOS_ADDR_BROADCAST;

static bool seenRoutingInThisFrame;

static bool isListening;

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

static uint32_t calcNextForwardTime(void)
{
    if (forwardRetries < 1) return 800;
    uint32_t passed = timeSinceFrameStart();
    uint32_t sinceFrameStart = randomInRange(2200, 3000);
    if (passed > 2000) return timeToNextFrame() + sinceFrameStart;
    return sinceFrameStart - passed;
}

static void roStartListeningTimerCb(void *x)
{
//    PRINTF("%lu: (%c) LISTEN START\n", getSyncTimeMs(), isRoutingInfoValid() ? '+' : '-');
    if (!isListening) {
        TPRINTF("+++ start\n");
    }
    alarmSchedule(&roStartListeningTimer, timeToNextFrame());

    isListening = true;
    radioOn();
    alarmSchedule(&roStopListeningTimer, 4000ul + 2 * MOTE_TIME_FULL * MAX_MOTES);
}

static void roStopListeningTimerCb(void *x)
{
//    PRINTF("%lu: (%c) turn radio off\n", getSyncTimeMs(), isRoutingInfoValid() ? '+' : '-');
    TPRINTF("--- stop\n");
    isListening = false;
    RADIO_OFF_ENERGSAVE();
    if (!seenRoutingInThisFrame) {
        TPRINTF("NO ROUTING PACKET THIS TIME!\n");
    }
    seenRoutingInThisFrame = false;
}

static void roCheckTimerCb(void *x)
{
    alarmSchedule(&roCheckTimer, 5000 + randomNumberBounded(1000));
    
    bool routingOk = isRoutingInfoValid();

    if (routingSearching) {
        // was searching for routing info
        if (routingOk) {
            routingSearching = false;
            alarmRemove(&roRequestTimer);
        }
    } else {
        // was not searching for routing info
        if (!routingOk) {
            routingSearching = true;
            routingRequestTimeout = ROUTING_REQUEST_INIT_TIMEOUT;
            roRequestTimerCb(NULL);
        }
    }
}

void routingInit(void)
{
    socketOpen(&roSocket, routingReceive);
    socketBind(&roSocket, ROUTING_PROTOCOL_PORT);
    socketSetDstAddress(&roSocket, MOS_ADDR_BROADCAST);

    alarmInit(&roCheckTimer, roCheckTimerCb, NULL);
    alarmInit(&roForwardTimer, roForwardTimerCb, NULL);
    alarmInit(&roRequestTimer, roRequestTimerCb, NULL);
    alarmInit(&roStopListeningTimer, roStopListeningTimerCb, NULL);
    alarmInit(&roStartListeningTimer, roStartListeningTimerCb, NULL);
    alarmInit(&roGreenLedTimer, roGreenLedTimerCb, NULL);
    alarmInit(&watchdogTimer, watchdogTimerCb, NULL);
    alarmSchedule(&roCheckTimer, randomInRange(1000, 3000));
    alarmSchedule(&roForwardTimer, calcNextForwardTime());
    alarmSchedule(&roStartListeningTimer, 90);
    alarmSchedule(&roGreenLedTimer, 10000);
//    alarmSchedule(&watchdogTimer, 1000);
}

static void roForwardTimerCb(void *x)
{
    gotRreq = false;

    if (forwardRetries) {
        forwardRetries--;
    } else {
        forwardRetries = 1;
    }
    alarmSchedule(&roForwardTimer, calcNextForwardTime());

    if (hopCountToRoot >= MAX_HOP_COUNT) return;

    if (!isRoutingInfoValid()) {
        roRequestTimerCb(NULL); // enter "search for root" state
        return;
    }

//    TPRINTF("send routing\n");

    RoutingInfoPacket_t routingInfo;
    routingInfo.packetType = ROUTING_INFORMATION;
    routingInfo.senderType = SENDER_FORWARDER;
    routingInfo.rootAddress = rootAddress;
    routingInfo.hopCount = hopCountToRoot + 1;
    routingInfo.seqnum = lastSeenSeqnum;
    routingInfo.rootClockMs = getSyncTimeMs64();
    routingInfo.moteNumber = 0;

    // XXX: INC_NETSTAT(NETSTAT_PACKETS_SENT, EMPTY_ADDR);
    socketSendEx(&roSocket, &routingInfo, sizeof(routingInfo), downstreamAddress);
}

static void roRequestTimerCb(void *x)
{
    // check if already found the info
    static uint8_t cnt;
    if (isRoutingInfoValid() && cnt > 5) return;
    cnt++;

    // add jitter
    routingRequestTimeout += randomNumberBounded(100);
    alarmSchedule(&roRequestTimer, routingRequestTimeout);
    if (routingRequestTimeout < ROUTING_REQUEST_MAX_EXP_TIMEOUT) {
        // use exponential increments
        routingRequestTimeout *= 2;
    } else if (routingRequestTimeout < ROUTING_REQUEST_MAX_TIMEOUT) {
        // use linear increments
        routingRequestTimeout += ROUTING_REQUEST_MAX_EXP_TIMEOUT;
    } else {
        // move back to initial (small) timeout
        routingRequestTimeout = ROUTING_REQUEST_INIT_TIMEOUT;
    }

    TPRINTF("SEND ROUTING REQUEST\n");

    radioOn(); // wait for response
    isListening = true;

    RoutingRequestPacket_t req;
    req.packetType = ROUTING_REQUEST;
    req.senderType = SENDER_FORWARDER;
    socketSendEx(&roSocket, &req, sizeof(req), MOS_ADDR_BROADCAST);

    alarmSchedule(&roStopListeningTimer, ROUTING_REPLY_WAIT_TIMEOUT);
}

static void roGreenLedTimerCb(void *x) {
    isGreenLedOn = !isGreenLedOn;
    if (isGreenLedOn) {
        if (isRoutingInfoValid()) greenLedOn();
    } else {
        greenLedOff();
    }
    alarmSchedule(&roGreenLedTimer, isGreenLedOn ? 100 : 5000);
}

static void watchdogTimerCb(void *x) {
    watchdogStart(WATCHDOG_EXPIRE_1000MS);
    alarmSchedule(&watchdogTimer, 500);
}

static void routingReceive(Socket_t *s, uint8_t *data, uint16_t len)
{
//    PRINTF("routing rx\n");
    // uint32_t rrTime = (uint32_t) getJiffies();
    // PRINTF("radio: %lu to %lu, routing: %lu\n", radioStartTime, radioEndTime, rrTime);

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
    TPRINTF("Got routing info from the nexthop\n");
#endif

    uint8_t type = data[0];
    if (type == ROUTING_REQUEST) {
        if (!gotRreq) {
            gotRreq = true;
            alarmSchedule(&roForwardTimer, randomInRange(400, 800));
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
        TPRINTF("RI updated: > seqnum\n");
    }
    else if (ri.seqnum == lastSeenSeqnum) {
        if (ri.hopCount < hopCountToRoot) {
            update = true;
            TPRINTF("RI updated: < metric\n");
        }
        else if (ri.hopCount == hopCountToRoot && !seenRoutingInThisFrame) {
            update = true;
            TPRINTF("RI updated: == metric\n");
        }
    }
    if (ri.hopCount > MAX_HOP_COUNT) update = false;

    if (update) {
        if (timeSinceFrameStart() > 2000) {
            PRINTF("*** base station (?) sends out of time!\n");
        }
        //PRINTF("got valid routing info\n");
//        PRINTF("%lu: OK!\n", (uint32_t) getJiffies());
        seenRoutingInThisFrame = true;
        rootAddress = ri.rootAddress;
        nexthopToRoot = s->recvMacInfo->originalSrc.shortAddr;
        lastSeenSeqnum = ri.seqnum;
        hopCountToRoot = ri.hopCount;
        lastRootMessageTime = (uint32_t) getJiffies();
        int32_t oldRootClockDeltaMs = rootClockDeltaMs;
        rootClockDeltaMs = ri.rootClockMs - getTimeMs64();
        //TPRINTF("process packet, rx time=%lu\n", (uint32_t) ri.rootClockMs);
        if (abs((int32_t)oldRootClockDeltaMs - (int32_t)rootClockDeltaMs) > 500) {
            PRINTF("large delta change=%ld, time sync off?!\n", (int32_t)rootClockDeltaMs - (int32_t)oldRootClockDeltaMs);
            PRINTF("delta: old=%ld, new=%ld\n", (int32_t)oldRootClockDeltaMs, (int32_t)rootClockDeltaMs);
        }
        // TPRINTF("OK!%s\n", isListening ? "" : " (not listening)");

        // reschedule next listen start after this timesync
        alarmSchedule(&roStartListeningTimer, timeToNextFrame());
    }
    else {
        PRINTF("RI not updated!\n");
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
    if (!IS_LOCAL(info)) {
//        PRINTF("route packet\n");
#if 0
        if (info->immedSrc.shortAddr) {
            if (rootAddress != 0 && info->immedSrc.shortAddr != rootAddress) {
                downstreamAddress = info->immedSrc.shortAddr;
            }
        }
#endif // 0
    }
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
                    TPRINTF("Dropping, I'm not a nexthop for sender %#04x\n",
                            info->originalSrc.shortAddr);
                    INC_NETSTAT(NETSTAT_PACKETS_DROPPED_RX, EMPTY_ADDR);
                    return RD_DROP;
                }
#endif
                // if (info->hoplimit < hopCountToRoot){
                //     PRINTF("Dropping, can't reach host with left hopCounts\n");
                //     return RD_DROP;
                // } 
                TPRINTF("****** Forwarding a packet to root for %#04x!\n",
                        info->originalSrc.shortAddr);
                // delay a bit
                info->timeWhenSend = getSyncTimeMs() + randomNumberBounded(150);
                INC_NETSTAT(NETSTAT_PACKETS_FWD, nexthopToRoot);
            } else{
                //INC_NETSTAT(NETSTAT_PACKETS_SENT, nexthopToRoot);     // Done @ comm.c
            }
            info->immedDst.shortAddr = nexthopToRoot;
            return RD_UNICAST;
        } else {
            // PRINTF("root routing info not present or expired!\n");
            TPRINTF("DROP!\n");
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
