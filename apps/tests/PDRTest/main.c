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

#include "stdmansos.h"
#include <lib/codec/crc.h>
#include <string.h>
#include <kernel/threads/radio.h>

#define RECV 0

#define TEST_PACKET_SIZE            30
#define PAUSE_BETWEEN_TESTS_MS      3000 // ms
#define PACKETS_IN_TEST             100u
#define SEND_INTERVAL               20
#define MAX_TEST_TIME               ((SEND_INTERVAL + 20) *  PACKETS_IN_TEST)


#define NUM_AVG_RUNS  10

#define SAMPLE_BUFFER_SIZE  25

void sendCounter(void);
void recvCounter(void);

//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
#if RECV
//    alarmInit(&alarm, alarmCb, NULL);
//    alarmSchedule(&alarm, 1000);
    PRINTF("starting recv...\n");
    recvCounter();
#else
    radioOn(); // XXX
    sendCounter();
#endif
}

#if RECV

volatile bool testInProgress;
volatile uint32_t testStartTime;
volatile uint16_t currentTestNumber;
volatile uint16_t currentTestPacketsRx;
volatile uint16_t prevTestPacketsRx;
volatile uint16_t rssiSum;
volatile uint16_t lqiSum;

uint16_t avgPDR[SAMPLE_BUFFER_SIZE];
uint16_t avgRssi[SAMPLE_BUFFER_SIZE];
uint16_t avgLQI[SAMPLE_BUFFER_SIZE];

void addAvgStatistics(uint16_t prevTestNumber, uint16_t prevTestPacketsRx, uint8_t rssi, uint8_t lqi)
{
    int16_t i;

    static bool calledBefore;
    static int16_t prevPrevTestNumber;
    int16_t idx = prevTestNumber % SAMPLE_BUFFER_SIZE;
    int16_t startIdx = idx;

    avgPDR[idx] = prevTestPacketsRx;
    avgRssi[idx] = rssi;

    if (!calledBefore) {
        calledBefore = true;
        prevPrevTestNumber = prevTestNumber;
        memset(avgPDR, 0xff, sizeof(avgPDR));
        memset(avgRssi, 0xff, sizeof(avgRssi));
        memset(avgLQI, 0xff, sizeof(avgLQI));
    } else if (prevPrevTestNumber < prevTestNumber) {
        prevPrevTestNumber++;
        while (prevPrevTestNumber < prevTestNumber) {
            avgPDR[prevPrevTestNumber % SAMPLE_BUFFER_SIZE] = 0;
            avgRssi[prevPrevTestNumber % SAMPLE_BUFFER_SIZE] = 0;
            avgLQI[prevPrevTestNumber % SAMPLE_BUFFER_SIZE] = 0;
            prevPrevTestNumber++;
        }
    }

    uint16_t numSamples = 0;
    uint16_t sumSamples = 0;
    int16_t sumRssi = 0;
    uint16_t sumLqi = 0;

    uint16_t numSamplesNe = 0;
    uint16_t sumSamplesNe = 0;
    int16_t sumRssiNe = 0;
    uint16_t sumLqiNe = 0;

    for (i = 0; i < NUM_AVG_RUNS; ++i) {
        if (avgPDR[idx] == ~0u) break;
        sumSamples += avgPDR[idx];
        sumRssi += avgRssi[idx];
        sumLqi += avgLQI[idx];
        numSamples++;
        if (idx == 0) idx = SAMPLE_BUFFER_SIZE - 1;
        else idx--;
    }

    if (numSamples == 0) return;

    idx = startIdx;
    for (i = 0; i < NUM_AVG_RUNS; ++i) {
        if (avgPDR[idx] == ~0u) break;
        sumSamplesNe += avgPDR[idx];
        sumRssiNe += avgRssi[idx];
        sumLqiNe += avgLQI[idx];
        numSamplesNe++;
        if (numSamplesNe >= NUM_AVG_RUNS) break;
        if (idx == 0) idx = SAMPLE_BUFFER_SIZE - 1;
        else idx--;
    }

    uint16_t samplesAvg, samplesAvgNe;
    int16_t rssiAvg;
    uint16_t lqiAvg;

    samplesAvg = (sumSamples + (numSamples / 2)) / numSamples;

    PRINTF("avg (%d runs): %d\n", numSamples, samplesAvg);

    if (numSamplesNe == 0) return;

    samplesAvgNe = (sumSamplesNe + (numSamplesNe / 2)) / numSamplesNe;
    rssiAvg = (sumRssiNe + (numSamplesNe / 2)) / numSamplesNe;
    lqiAvg = (sumLqiNe + (numSamplesNe / 2)) / numSamplesNe;
#if PLATFROM_TELOSB
    rssiAvg -= 128;
#endif

    PRINTF("nonempty avg (%d runs): %d, rssi: %d, lqi: %d\n",
            numSamplesNe, rssiAvg, lqiAvg);

    RadioInfoPacket_t packet;
    packet.address = localAddress;
    packet.lastTestNo = prevTestNumber;
    packet.numTests = numSamples;
    packet.avgPdr = samplesAvg;
    packet.numTestsNe = numSamplesNe;
    packet.avgPdrNe = numSamplesNe;
    packet.avgRssiNe = rssiAvg;
    packet.avgLqiNe = lqiAvg;

    radioSetChannel(BS_CHANNEL);
    radioSend(&packet);
    radioSetChannel(TEST_CHANNEL);
}

void endTest(void) {
    testInProgress = false;
    prevTestPacketsRx = currentTestPacketsRx;
    currentTestPacketsRx = 0;
    greenLedOff();
    toggleRedLed();
}

void startTest(uint16_t newTestNumber) {
    testInProgress = true;
    currentTestNumber = newTestNumber;
    testStartTime = getJiffies();
    greenLedOn();
    toggleRedLed();
    rssiSum = 0;
    lqiSum = 0;
}

void recvCallback(uint8_t *data, int16_t len)
{
    if (len != TEST_PACKET_SIZE) {
        PRINTF("rcvd length=%d\n", len);
        return;
    }

    uint16_t calcCrc, recvCrc;
    calcCrc = crc16(data, TEST_PACKET_SIZE - 2);
    memcpy(&recvCrc, data + TEST_PACKET_SIZE - 2, sizeof(uint16_t));
    if (calcCrc != recvCrc) {
        static uint32_t nextCrcErrorReportTime;
        uint32_t now = getJiffies();
        if (timeAfter32(now, nextCrcErrorReportTime)) {
            PRINT("wrong CRC\n");
            nextCrcErrorReportTime = now + MAX_TEST_TIME;
        }
//      debugHexdump(data, len);
//      radioOff();
//      mdelay(100);
//      radioOn();
        return;
    }

    uint8_t rssi;
#if PLATFROM_TELOSB
    rssi = radioGetLastRSSI() + 128;
#else
    rssi = radioGetLastRSSI();
#endif
    rssiSum += rssi;
    lqiSum += lqi;

    uint16_t testNum;
    memcpy(&testNum, data, sizeof(uint16_t));
    if (testNum != currentTestNumber) {
        if (testInProgress) endTest();
    }
    if (!testInProgress) {
        startTest(testNum);
    }
    ++currentTestPacketsRx;
}

void recvCounter(void)
{
    static uint8_t recvBuffer[RADIO_MAX_PACKET];
    RADIO_PACKET_BUFFER(radioBuffer, RADIO_MAX_PACKET);
    radioOn();

    bool prevTestInProgress = false;
    uint16_t prevTestNumber = 0;
    for (;;) {
        if (radioBuffer.receivedLength != 0) {
            // free the buffer as soon as possible
            int16_t recvLen = radioBuffer.receivedLength;
            if (recvLen > 0) {
                memcpy(recvBuffer, radioBuffer.buffer, recvLen);
                radioBufferReset(radioBuffer);
                recvCallback(recvBuffer, recvLen);
            } else {
                PRINTF("radio rx error: %s\n", strerror(-recvLen));
                radioBufferReset(radioBuffer);
            }
        }

        if (testInProgress) {
            if (timeAfter32(getJiffies(), testStartTime + MAX_TEST_TIME)) {
                endTest();
            }
        }
        if (testInProgress != prevTestInProgress || prevTestNumber != currentTestNumber) {
            if (prevTestInProgress) {
                uint16_t avgRssi, avgLqi;
                if (prevTestPacketsRx == 0) {
                    avgRssi = 0;
                    avgLqi = 0;
                } else {
                    avgRssi = rssiSum / prevTestPacketsRx;
                    avgLqi = lqiSum / prevTestPacketsRx;
                }
                PRINTF("Test %u: %d%%, %d avg RSSI\n", prevTestNumber, prevTestPacketsRx, avgRssi, avgLqi);
                addAvgStatistics(prevTestNumber, prevTestPacketsRx, avgRssi);
            }
            if (testInProgress) {
                // PRINT("\n===================================\n");
                // PRINTF("Starting test %u...\n", currentTestNumber);
            }
        }
        prevTestInProgress = testInProgress;
        if (testInProgress) {
            prevTestNumber = currentTestNumber;
        } else {
            prevTestNumber = 0;
        }
        yield(); // yield manually
    }
}

#else

void sendCounter(void)
{
    static uint8_t sendBuffer[TEST_PACKET_SIZE];
    uint16_t testNumber;
    for (testNumber = 1; ; testNumber++) {
        uint8_t i;
        uint16_t crc;

        PRINTF("Send test %u packets\n", testNumber);
        redLedOn();
        for (i = 0; i < PACKETS_IN_TEST; ++i) {
            memcpy(sendBuffer, &testNumber, sizeof(testNumber));
            sendBuffer[2] = i;
            crc = crc16(sendBuffer, TEST_PACKET_SIZE - 2);
            memcpy(sendBuffer + TEST_PACKET_SIZE - 2, &crc, sizeof(crc));

            radioSend(sendBuffer, sizeof(sendBuffer));
            mdelay(SEND_INTERVAL);
        }
        redLedOff();

        mdelay(PAUSE_BETWEEN_TESTS_MS);
    }
}

#endif
