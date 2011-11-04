/**
 * Copyright (c) 2008-2010 Leo Selavo and the contributors. All rights reserved.
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

#include "test_stats.h"
#include "dprint.h"

extern uint16_t pckReceived; // packets received in this test
extern uint16_t rssiThreshold;

// RSSI statistics
static uint16_t rssi[PACKETS_IN_TEST];
static uint16_t rssiMin;
static uint16_t rssiMax;
//static uint16_t rssiAvg;


//static void calcRssiStats() {
    // calculate average = sum / count
//    uint32_t rssiSum = 0;
//    uint32_t rssiAvgSum = 0; // sum(rssi[i]^2)
//    uint16_t i;
//    for (i = 0; i < pckReceived; ++i) {
//        rssiSum += rssi[i];
//        rssiSqrSum += (uint32_t) rssi[i] * (uint32_t) rssi[i];
//    }
//    rssiAvg = (uint16_t) (rssiSum / (uint32_t) pckReceived);
//    rssiVar = rssiSqrSum / (uint32_t) pckReceived
//            - (uint32_t) rssiAvg * (uint32_t) rssiAvg;
//}

// calculate & display statistics of previous test
void showTestStats() {
//    PRINTF("--- TEST STATS ---\n");
    //calcRssiStats();
    PRINTF("%u; %u; %u;", pckReceived, PACKETS_IN_TEST, rssiThreshold);
    uint16_t i;
    for (i = 0; i < pckReceived && i < PACKETS_IN_TEST; ++i) {
        PRINTF(" %i;", rssi[i]);
    }
    PRINTF("\n");
}

// clear statistics
void clearTestStats() {
    pckReceived = 0;
    rssiMin = 0xffff;
    rssiMax = 0;
}

// add received packet to statistics
void updateTestStats(const Packet_t *pck, uint16_t r) {
    if (pckReceived < PACKETS_IN_TEST) {
        rssi[pckReceived] = r;
        if (r < rssiMin) rssiMin = r;
        if (r > rssiMax) rssiMax = r;
    }
    ++pckReceived;
}
