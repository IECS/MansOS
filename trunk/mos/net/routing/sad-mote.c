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
#include <watchdog.h>
#include <net/net_stats.h>

#include <leds.h>

static Socket_t roSocket;
static Alarm_t roCheckTimer;
static Alarm_t roRequestTimer;
static Alarm_t roStartListeningTimer;
static Alarm_t roStopListeningTimer;
static Alarm_t roGreenLedTimer;
static Alarm_t watchdogTimer; // XXX: not enabled for motes!

static void roCheckTimerCb(void *);
static void roRequestTimerCb(void *);
static void routingReceive(Socket_t *s, uint8_t *data, uint16_t len);
static void roGreenLedTimerCb(void *);
static void watchdogTimerCb(void *);

static Seqnum_t lastSeenSeqnum;
static uint8_t hopCountToRoot = MAX_HOP_COUNT;
static uint32_t lastRootMessageTime;
static MosShortAddr nexthopToRoot;
static uint8_t moteNumber;

static bool isListening;
static bool isGreenLedOn;

static uint32_t routingRequestTimeout = ROUTING_REQUEST_INIT_TIMEOUT;
static bool routingSearching;

static bool seenRoutingInThisFrame;

static void roStartListeningTimerCb(void *);
static void roStopListeningTimerCb(void *);

#define IS_ODD_COLLECTOR (nexthopToRoot & 0x1)

#if DEBUG
#define ROUTE_DEBUG 1
#endif

#define ROUTE_DEBUG 1

#if ROUTE_DEBUG
#define RPRINTF(...) TPRINTF(__VA_ARGS__)
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
        nexthopToRoot = 0;
        return false;
    }
    return true;
}

static inline bool hasRecentlySyncedWithRoot(void)
{
    if (lastRootMessageTime == 0) return false;

    return !timeAfter32((uint32_t)getJiffies(), lastRootMessageTime + 4 * ROUTING_INFO_VALID_TIME);
}

static inline bool isCollectorMaybeListening(uint32_t atTime)
{
    uint32_t passed = timeSinceFrameStart();
    if (passed < 2000) return false;
    if (passed > 4000 + 2 * MOTE_TIME_FULL * MAX_MOTES) return false;
    return true;
}

static uint32_t calcListenStartTime(void)
{
    uint32_t result = timeToNextFrame() + 4000ul + MOTE_TIME_FULL * moteNumber;
    if (IS_ODD_COLLECTOR) result += MOTE_TIME_FULL * MAX_MOTES;
    if (result < TIMESLOT_IMPRECISION) result = 0;
    else result -= TIMESLOT_IMPRECISION;
    return result;
}

static uint32_t calcSendTime(void)
{
    // decrease the random offset in order to speed up things!
    uint32_t expected = 4000ul + MOTE_TIME_FULL * moteNumber + MOTE_TIME - 200 + randomInRange(0, 100);
    if (IS_ODD_COLLECTOR) expected += MOTE_TIME_FULL * MAX_MOTES;
    uint32_t now = timeSinceFrameStart();
    if (now > expected) expected += SAD_SUPERFRAME_LENGTH;
    expected -= now;
    return expected + getSyncTimeMs();
}

void routingInit(void)
{
    socketOpen(&roSocket, routingReceive);
    socketBind(&roSocket, ROUTING_PROTOCOL_PORT);
    socketSetDstAddress(&roSocket, MOS_ADDR_BROADCAST);

    alarmInit(&roCheckTimer, roCheckTimerCb, NULL);
    alarmInit(&roRequestTimer, roRequestTimerCb, NULL);
    alarmInit(&roStopListeningTimer, roStopListeningTimerCb, NULL);
    alarmInit(&roStartListeningTimer, roStartListeningTimerCb, NULL);
    alarmInit(&roGreenLedTimer, roGreenLedTimerCb, NULL);
    alarmInit(&watchdogTimer, watchdogTimerCb, NULL);
    alarmSchedule(&roCheckTimer, randomInRange(1000, 3000));
    alarmSchedule(&roStartListeningTimer, 110);
    alarmSchedule(&roGreenLedTimer, 10000);
//    alarmSchedule(&watchdogTimer, 1000);
}

static void roStartListeningTimerCb(void *x)
{
    alarmSchedule(&roStartListeningTimer, calcListenStartTime());

    // listen to info only when routing info is already valid (?)
    if (isRoutingInfoValid()) {
        TPRINTF("+++ mote #%u start\n", moteNumber);
        isListening = true;
        radioOn();
        alarmSchedule(&roStopListeningTimer, TIMESLOT_IMPRECISION * 2 + MOTE_TIME * 2);
    }
}

static void roStopListeningTimerCb(void *x)
{
    TPRINTF("--- stop\n");
    seenRoutingInThisFrame = false;
    RADIO_OFF_ENERGSAVE();
    isListening = false;
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
            moteNumber = 0; // reset
            routingSearching = true;
            routingRequestTimeout = ROUTING_REQUEST_INIT_TIMEOUT;
            roRequestTimerCb(NULL);
        }
    }
}

static void roRequestTimerCb(void *x)
{
    // check if already found the info
    static uint8_t cnt;
    if (isRoutingInfoValid() && cnt > 5) return;
    cnt++;

    if (hasRecentlySyncedWithRoot()) {
        // try to guess the time depending on the last information about collector
        uint32_t passed = timeSinceFrameStart();
        if (passed < 2000) {
            routingRequestTimeout = 2500 - passed;
        }
        else if (passed + 5000 >= 4000 + 2 * MOTE_TIME_FULL * MAX_MOTES) {
            routingRequestTimeout = timeToNextFrame() + 2500;
        }
        else {
            routingRequestTimeout = 5000;
        }
        routingRequestTimeout += randomNumberBounded(100);
        alarmSchedule(&roRequestTimer, routingRequestTimeout);
    }
    else {
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
    }

    TPRINTF("SEND ROUTING REQUEST\n");

    radioOn(); // wait for response
    isListening = true;

    RoutingRequestPacket_t req;
    req.packetType = ROUTING_REQUEST;
    req.senderType = SENDER_MOTE;
    socketSend(&roSocket, &req, sizeof(req));

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
    // RPRINTF("routingReceive %d bytes from %#04x\n", len,
    //          s->recvMacInfo->originalSrc.shortAddr);

    if (len == 0) {
        PRINTF("routingReceive: no data!\n");
        return;
    }

#if PRECONFIGURED_NH_TO_ROOT
    if (s->recvMacInfo->originalSrc.shortAddr != PRECONFIGURED_NH_TO_ROOT) {
        TPRINTF("Dropping routing info: not from the nexthop, but from %#04x\n",
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
        TPRINTF("routingReceive: unknown type!\n");
        return;
    }

    if (len < sizeof(RoutingInfoPacket_t)) {
        TPRINTF("routingReceive: too short for info packet!\n");
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
        RPRINTF("RI updated: > seqnum\n");
    }
    else if (ri.seqnum == lastSeenSeqnum) {
        if (ri.hopCount < hopCountToRoot) {
            update = true;
            RPRINTF("RI updated: < metric\n");
        }
        else if (ri.hopCount == hopCountToRoot) {
            if (s->recvMacInfo->originalSrc.shortAddr == nexthopToRoot) {
                // same adress, update if not seen already
                update = !seenRoutingInThisFrame;
                RPRINTF("RI updated: == nh && == metric\n");
            }
            else if (s->recvMacInfo->originalSrc.shortAddr > nexthopToRoot) {
                // At the moment nexthop is selected by "the largest address wins" algorithm.
                // TODO: add a special "priority" field? or select nexhop in random?
                update = true;
                RPRINTF("RI updated: > nh && == metric\n");
            }
        }
    }
    if (ri.hopCount > MAX_HOP_COUNT) update = false;

    if (update) {
        // TPRINTF("got valid routing info\n");
        seenRoutingInThisFrame = true;
        rootAddress = ri.rootAddress;
        nexthopToRoot = s->recvMacInfo->originalSrc.shortAddr;
        lastSeenSeqnum = ri.seqnum;
        hopCountToRoot = ri.hopCount;
        lastRootMessageTime = (uint32_t)getJiffies();
        int64_t oldRootClockDeltaMs = rootClockDeltaMs;
        rootClockDeltaMs = ri.rootClockMs - getTimeMs64();
        bool numberChanged = moteNumber != ri.moteNumber;
        moteNumber = ri.moteNumber;
        // PRINTF("%lu: ++++++++++++ fixed local time\n", getSyncTimeMs());
        if (abs((int32_t)oldRootClockDeltaMs - (int32_t)rootClockDeltaMs) > 500) {
            PRINTF("large delta change=%ld, time sync off?!\n", (int32_t)rootClockDeltaMs - (int32_t)oldRootClockDeltaMs);
            PRINTF("delta: old=%ld, new=%ld\n", (int32_t)oldRootClockDeltaMs, (int32_t)rootClockDeltaMs);
        }
        // TPRINTF("OK!%s\n", isListening ? "" : " (not listening)");

        if (numberChanged) {
            // reschedule start of next listening
            alarmSchedule(&roStartListeningTimer, calcListenStartTime());
        }
    }
    else {
        RPRINTF("RI not updated!\n");
    }
}

RoutingDecision_e routePacket(MacInfo_t *info)
{
    MosAddr *dst = &info->originalDst;
    fillLocalAddress(&info->immedSrc);

    // RPRINTF("dst address=0x%04x, nexthop=0x%04x\n", dst->shortAddr, info->immedDst.shortAddr);
    // RPRINTF("  is_local=%u localAddress=0x%04x\n", IS_LOCAL(info) ? 1 : 0, localAddress);

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
        TPRINTF("root routing info not present or expired!\n");
        return RD_DROP;
    }
    // RPRINTF("using 0x%04x as nexthop to root\n", nexthopToRoot);
    info->immedDst.shortAddr = nexthopToRoot;
    // delay until our frame
    info->timeWhenSend = calcSendTime();
    return RD_UNICAST;
}
