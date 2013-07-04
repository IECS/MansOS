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
// SAD routing, collector functionality
//

#include "../mac.h"
#include "../routing.h"
#include "../socket.h"
#include <alarms.h>
#include <timing.h>
#include <print.h>
#include <random.h>
#include <net/net_stats.h>

#include <leds.h>

typedef struct MoteInfo_s {
    MosShortAddr address;
    uint32_t validUntil;
} MoteInfo_t;

static MoteInfo_t motes[MAX_MOTES];

static Socket_t roSocket;
static Alarm_t roCheckTimer;
static Alarm_t roForwardTimer;
static Alarm_t roOutOfOrderForwardTimer;
static Alarm_t roRequestTimer;
static Alarm_t roStartListeningTimer;
static Alarm_t roStopListeningTimer;
static Alarm_t roGreenLedTimer;

static void roCheckTimerCb(void *);
static void roForwardTimerCb(void *);
static void roOutOfOrderForwardTimerCb(void *);
static void roRequestTimerCb(void *);
static void routingReceive(Socket_t *s, uint8_t *data, uint16_t len);
static void roGreenLedTimerCb(void *);

static uint32_t routingRequestTimeout = ROUTING_REQUEST_INIT_TIMEOUT;
static bool routingSearching;

static Seqnum_t lastSeenSeqnum;
static uint8_t hopCountToRoot = MAX_HOP_COUNT;
static uint32_t lastRootMessageTime = (uint32_t) -ROUTING_INFO_VALID_TIME;
static MosShortAddr nexthopToRoot;

static void roStartListeningTimerCb(void *);
static void roStopListeningTimerCb(void *);

static uint8_t gotRreq = 0xff;

static bool seenRoutingInThisFrame;

static bool isListening;
static bool isGreenLedOn;

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
    uint32_t t = (getSyncTimeMs() + 5) % SAD_SUPERFRAME_LENGTH;
    // at start of (next) frame
    if (t + 1000 > SAD_SUPERFRAME_LENGTH) {
        return 2 * SAD_SUPERFRAME_LENGTH - t;
    } else {
        return SAD_SUPERFRAME_LENGTH - t;
    }
}

static uint32_t calcNextForwardTime(uint8_t moteToProcess)
{
    uint32_t t = getSyncTimeMs() % SAD_SUPERFRAME_LENGTH;
    if (moteToProcess == 0) {
        t = SAD_SUPERFRAME_LENGTH - t;
        // leave 5 seconds for fwd stage
        t += 5000;
        // add random jitter
        t += randomInRange(100, 200);
    } else {
        uint32_t n = 5000; // 5 seconds for fwd stage
        // one second for each mote
        n += 1000 * moteToProcess;
        // add random jitter
        n += randomInRange(100, 200);
        if (n > t) {
            t = n - t;
        } else {
            t = 0;
        }
    }
    // PRINTF("%lu: calcNextForwardTime=%lu\n", getSyncTimeMs(), t);
    return t;
}

void routingInit(void)
{
    socketOpen(&roSocket, routingReceive);
    socketBind(&roSocket, ROUTING_PROTOCOL_PORT);

    alarmInit(&roCheckTimer, roCheckTimerCb, NULL);
    alarmInit(&roForwardTimer, roForwardTimerCb, NULL);
    alarmInit(&roOutOfOrderForwardTimer, roOutOfOrderForwardTimerCb, NULL);
    alarmInit(&roRequestTimer, roRequestTimerCb, NULL);
    alarmInit(&roStopListeningTimer, roStopListeningTimerCb, NULL);
    alarmInit(&roStartListeningTimer, roStartListeningTimerCb, NULL);
    alarmInit(&roGreenLedTimer, roGreenLedTimerCb, NULL);
    alarmSchedule(&roCheckTimer, randomInRange(1000, 3000));
    alarmSchedule(&roForwardTimer, calcNextForwardTime(0));
    alarmSchedule(&roStartListeningTimer, 110);
    alarmSchedule(&roGreenLedTimer, 10000);
}

static void roStartListeningTimerCb(void *x)
{
//    PRINTF("%lu: (%c) LISTEN START\n", getSyncTimeMs(), isRoutingInfoValid() ? '+' : '-');
    if (!isListening) {
        PRINTF("%lu: +++ start\n", getSyncTimeSec());
    }
    alarmSchedule(&roStartListeningTimer, calcListenStartTime());

    radioOn();
    isListening = true;
    alarmSchedule(&roStopListeningTimer, 5000 + 1000 * MAX_MOTES);
}

static void roStopListeningTimerCb(void *x)
{
//    PRINTF("%lu: (%c) turn radio off\n", getSyncTimeMs(), isRoutingInfoValid() ? '+' : '-');
    PRINTF("%lu: --- stop\n", getSyncTimeSec());
    RADIO_OFF_ENERGSAVE();
    isListening = false;
    if (!seenRoutingInThisFrame) {
        //PRINTF("%lu: NO ROUTING PACKET THIS TIME!\n", (uint32_t) getJiffies());
        PRINTF("%lu: NO ROUTING PACKET THIS TIME!\n", getSyncTimeSec());
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
        // was searching for routing info
        if (!routingOk) {
            routingSearching = true;
            routingRequestTimeout = ROUTING_REQUEST_INIT_TIMEOUT;
            roRequestTimerCb(NULL);
        }
    }
}

static void forwardRoutingInfo(uint8_t moteNumber) {
    // PRINTF("%lu: forward routing packet to mote %u (%#04x)\n",
    //         getSyncTimeMs(), moteNumber, motes[moteNumber].address);

    RoutingInfoPacket_t routingInfo;
    routingInfo.packetType = ROUTING_INFORMATION;
    routingInfo.senderType = SENDER_COLLECTOR;
    routingInfo.rootAddress = rootAddress;
    routingInfo.hopCount = hopCountToRoot + 1;
    routingInfo.seqnum = lastSeenSeqnum;
    routingInfo.rootClockMs = getSyncTimeMs64();
    routingInfo.moteNumber = moteNumber;

    // XXX: INC_NETSTAT(NETSTAT_PACKETS_SENT, EMPTY_ADDR);
    socketSendEx(&roSocket, &routingInfo, sizeof(routingInfo), motes[moteNumber].address);
}

static void roForwardTimerCb(void *x)
{
    static uint8_t moteToProcess;

    bool roOK = isRoutingInfoValid();

    // if (moteToProcess == 0) {
    //     PRINTF("roForwardTimerCb, roOK=%d\n", (int) roOK);
    // }

    if (motes[moteToProcess].address != 0
            && timeAfter32(motes[moteToProcess].validUntil, (uint32_t)getJiffies())
            && hopCountToRoot < MAX_HOP_COUNT
            && roOK) {
        forwardRoutingInfo(moteToProcess);
    }
        
    moteToProcess++;
    if (moteToProcess == MAX_MOTES) moteToProcess = 0;

    alarmSchedule(&roForwardTimer, calcNextForwardTime(moteToProcess));
}

static void roOutOfOrderForwardTimerCb(void *x)
{
    if (gotRreq != 0xff && isRoutingInfoValid()) {
        forwardRoutingInfo(gotRreq);
    }
    gotRreq = 0xff;
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
    // use exponential backoff
    routingRequestTimeout *= 2;
    if (routingRequestTimeout > ROUTING_REQUEST_MAX_TIMEOUT) {
        // move back to initial (small) timeout
        routingRequestTimeout = ROUTING_REQUEST_INIT_TIMEOUT;
    }

    PRINTF("%lu: send routing request\n", (uint32_t)getJiffies());

    radioOn(); // wait for response
    isListening = true;

    RoutingRequestPacket_t req;
    req.packetType = ROUTING_REQUEST;
    req.senderType = SENDER_COLLECTOR;
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

static uint8_t markAsSeen(MosShortAddr address, bool addNew)
{
    uint32_t now = (uint32_t) getJiffies();
    // MOTE_INFO_VALID_TIME - ?
    uint8_t i;
    for (i = 0; i < MAX_MOTES; ++i) {
        if (motes[i].address == address) {
            break;
        }
    }
    if (i == MAX_MOTES && addNew) {
        for (i = 0; i < MAX_MOTES; ++i) {
            if (motes[i].address == 0
                    || timeAfter32(now, motes[i].validUntil)) {
                break;
            }
        }
        // save its address
        motes[i].address = address;
    }
    if (i == MAX_MOTES) {
        if (addNew) PRINTF("recv rreq: no more space!\n");
        return 0xff;
    }

    // mark it as seen
    motes[i].validUntil = now + MOTE_INFO_VALID_TIME;
    return i;
}

static void routingReceive(Socket_t *s, uint8_t *data, uint16_t len)
{
//    if (!isListening) PRINTF("%lu: not listening\n", (uint32_t)getJiffies());

    // uint32_t rrTime = (uint32_t)getJiffies();
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
    PRINTF("Got routing info from the nexthop\n");
#endif

    uint8_t type = data[0];
    if (type == ROUTING_REQUEST) {
        uint8_t senderType = data[1];
        if (senderType != SENDER_MOTE) return;
        uint8_t idx = markAsSeen(s->recvMacInfo->originalSrc.shortAddr, true);
        if (gotRreq == 0xff) {
            gotRreq = idx;
            alarmSchedule(&roOutOfOrderForwardTimer, randomNumberBounded(400));
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
        PRINTF("update routing info - more recent seqnum\n");
    } else if (ri.seqnum == lastSeenSeqnum) {
        if (ri.hopCount < hopCountToRoot) {
            update = true;
            PRINTF("update routing info - better metric\n");
        }
    }
    if (ri.hopCount > MAX_HOP_COUNT) update = false;

    if (update) {
//        PRINTF("got valid routing info\n");
        // PRINTF("%lu: OK!%s\n", (uint32_t) getJiffies(), 
        //         isListening ? "" : " (not listening)");
        seenRoutingInThisFrame = true;
        rootAddress = ri.rootAddress;
        nexthopToRoot = s->recvMacInfo->originalSrc.shortAddr;
        lastSeenSeqnum = ri.seqnum;
        hopCountToRoot = ri.hopCount;
        lastRootMessageTime = (uint32_t) getJiffies();
        int64_t oldRootClockDeltaMs = rootClockDeltaMs;
        rootClockDeltaMs = ri.rootClockMs - getTimeMs64();
        greenLedToggle();
        PRINTF("delta: old=%ld, new=%ld\n", (int32_t)oldRootClockDeltaMs, (int32_t)rootClockDeltaMs);
        if (abs((int32_t)oldRootClockDeltaMs - (int32_t)rootClockDeltaMs) > 500) {
            PRINTF("large delta=%ld, time sync off?!\n", (int32_t)oldRootClockDeltaMs - (int32_t)rootClockDeltaMs);
        }
        PRINTF("%lu: OK!%s\n", getSyncTimeSec(), isListening ? "" : " (not listening)");
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

    // PRINTF("route: dst address=0x%04x, nexthop=0x%04x\n", dst->shortAddr, info->immedDst.shortAddr);
    // PRINTF("  localAddress=0x%04x\n", localAddress);

    if (IS_LOCAL(info)) {
        // fix root address if we are sending it to the root
        if (dst->shortAddr == MOS_ADDR_ROOT) {
            intToAddr(info->originalDst, rootAddress);
            // info->hoplimit = hopCountToRoot;
            info->hoplimit = MAX_HOP_COUNT;
        }
    } else {
        markAsSeen(info->immedSrc.shortAddr, false);
    }

    if (isLocalAddress(dst)) {
        INC_NETSTAT(NETSTAT_PACKETS_RECV, info->originalSrc.shortAddr);
        return RD_LOCAL;
    }
    if (isBroadcast(dst)) {
        fillLocalAddress(&info->immedSrc); // XXX
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

    // fill address: may forward it
    fillLocalAddress(&info->immedSrc);

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
                INC_NETSTAT(NETSTAT_PACKETS_FWD, nexthopToRoot);
                // delay a bit
                info->timeWhenSend = getSyncTimeMs() + randomInRange(50, 150);
            } else{
                //INC_NETSTAT(NETSTAT_PACKETS_SENT, nexthopToRoot);     // Done @ comm.c
            }
            info->immedDst.shortAddr = nexthopToRoot;
            return RD_UNICAST;
        } else {
            // PRINTF("root routing info not present or expired!\n");
            PRINTF("DROP\n");
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
