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
#define MAX_TEST_TIME               ((SEND_INTERVAL + 10) *  PACKETS_IN_TEST)


#define NUM_AVG_RUNS_SMALL    5
#define NUM_AVG_RUNS_MEDIUM  10
#define NUM_AVG_RUNS_LARGE   20

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

uint16_t avgPDR[SAMPLE_BUFFER_SIZE];
int16_t avgRssi[SAMPLE_BUFFER_SIZE];

void addAvgStatistics(uint16_t prevTestNumber, uint16_t prevTestPacketsRx, int8_t rssi)
{
    int16_t i;

    static bool calledBefore;
    static int16_t prevPrevTestNumber;
    int16_t idx = prevTestNumber % SAMPLE_BUFFER_SIZE;
    int16_t startIdx = idx;

    avgPDR[idx] = prevTestPacketsRx;
    avgRssi[idx] = ((int16_t) rssi) + 128;

    if (!calledBefore) {
        calledBefore = true;
        prevPrevTestNumber = prevTestNumber;
        memset(avgPDR, 0xff, sizeof(avgPDR));
        memset(avgRssi, 0xff, sizeof(avgRssi));
    } else if (prevPrevTestNumber < prevTestNumber) {
        prevPrevTestNumber++;
        while (prevPrevTestNumber < prevTestNumber) {
            avgPDR[prevPrevTestNumber % SAMPLE_BUFFER_SIZE] = 0;
            avgRssi[prevPrevTestNumber % SAMPLE_BUFFER_SIZE] = 0;
            prevPrevTestNumber++;
        }
    }

    uint16_t numSmall, numMed, numLarge;
    uint16_t sumSmall, sumMed, sumLarge;
    int16_t sumRssiSmall, sumRssiMed, sumRssiLarge;
    uint16_t numSmallNe, numMedNe, numLargeNe;
    uint16_t sumSmallNe, sumMedNe, sumLargeNe;
    int16_t sumRssiSmallNe, sumRssiMedNe, sumRssiLargeNe;
    numSmall = numMed = numLarge = 0;
    sumSmall = sumMed = sumLarge = 0;
    sumRssiSmall = sumRssiMed = sumRssiLarge = 0;
    numSmallNe = numMedNe = numLargeNe = 0;
    sumSmallNe = sumMedNe = sumLargeNe = 0;
    sumRssiSmallNe = sumRssiMedNe = sumRssiLargeNe = 0;

    for (i = 0; i < NUM_AVG_RUNS_LARGE; ++i) {
        if (avgPDR[idx] == ~0u) break;
        sumLarge += avgPDR[idx];
        sumRssiLarge += avgRssi[idx];
        numLarge++;
        if (i < NUM_AVG_RUNS_MEDIUM) {
            sumMed += avgPDR[idx];
            sumRssiMed += avgRssi[idx];
            numMed++;
        }
        if (i < NUM_AVG_RUNS_SMALL) {
            sumSmall += avgPDR[idx];
            sumRssiSmall += avgRssi[idx];
            numSmall++;
        }
        if (idx == 0) idx = SAMPLE_BUFFER_SIZE - 1;
        else idx--;
    }

    if (numLarge == 0) return;

    idx = startIdx;
    for (i = 0; i < SAMPLE_BUFFER_SIZE; ++i) {
        if (avgPDR[idx] == ~0u) break;
        if (avgPDR[idx] != 0) {
            sumLargeNe += avgPDR[idx];
            sumRssiLargeNe += avgRssi[idx];
            numLargeNe++;
            if (numLargeNe == NUM_AVG_RUNS_LARGE) break;
        }
        if (numMedNe < NUM_AVG_RUNS_MEDIUM) {
            if (avgPDR[idx] != 0) {
                sumMedNe += avgPDR[idx];
                sumRssiMedNe += avgRssi[idx];
                numMedNe++;
            }
        }
        if (numSmallNe < NUM_AVG_RUNS_SMALL) {
            if (avgPDR[idx] != 0) {
                sumSmallNe += avgPDR[idx];
                sumRssiSmallNe += avgRssi[idx];
                numSmallNe++;
            }
        }
        if (idx == 0) idx = SAMPLE_BUFFER_SIZE - 1;
        else idx--;
    }

    uint16_t s, m, l;
    int16_t s1, m1, l1;
    s = sumSmall * 100ul / numSmall + 1;
    m = sumMed * 100ul / numMed + 1;
    l = sumLarge * 100ul / numLarge + 1;
    // s1 = sumRssiSmall * 100ul / numSmall + 1;
    // m1 = sumRssiMed * 100ul / numMed + 1;
    // l1 = sumRssiLarge * 100ul / numLarge + 1;
    PRINTF("avg (%d/%d/%d runs): %d/%d/%d\n",
            numSmall, numMed, numLarge,
            s / 100, m / 100, l / 100);
    s = sumSmallNe * 100ul / numSmallNe + 1;
    m = sumMedNe * 100ul / numMedNe + 1;
    l = sumLargeNe * 100ul / numLargeNe + 1;
    s1 = sumRssiSmallNe * 100ul / numSmallNe + 1;
    m1 = sumRssiMedNe * 100ul / numMedNe + 1;
    l1 = sumRssiLargeNe * 100ul / numLargeNe + 1;
    PRINTF("nonempty avg (%d/%d/%d runs): %d/%d/%d, rssi: %d/%d/%d\n",
            numSmallNe, numMedNe, numLargeNe,
            s / 100, m / 100, l / 100,
            s1 / 100 - 128, m1 / 100 - 128, l1 / 100 - 128);
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
                int16_t rssi = radioGetLastRSSI() + 128;
                rssiSum += (uint16_t) rssi;
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
                int16_t avgRssi;
                if (prevTestPacketsRx == 0) {
                    avgRssi = 0;
                } else {
                    avgRssi = rssiSum / prevTestPacketsRx;
                    avgRssi -= 128;
                }
                PRINTF("Test %u: %d%%, %d avg RSSI\n", prevTestNumber, prevTestPacketsRx, avgRssi);
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
