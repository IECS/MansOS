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
#include <kernel/expthreads/radio.h>
#include <kernel/expthreads/alarms.h>

#define RECV 1

#define TEST_PACKET_SIZE            30
#define PAUSE_BETWEEN_TESTS_MS      3000 // ms
#define PACKETS_IN_TEST             100u
#define SEND_INTERVAL               20
#define MAX_TEST_TIME               ((SEND_INTERVAL + 10) *  PACKETS_IN_TEST)

void sendCounter(void);
void recvCounter(void);

// Alarm_t alarm;
// static void alarmCb(void *);

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

// static void alarmCb(void *x)
// {
    // PRINT("alarmCb\n");
    // radioOff();
    // radioOn();
    // mdelay(1);
    // radioSend("hello world", sizeof("hello world"));

    // extern void mrf24j40Reset();
    // mrf24j40Reset();

//     alarmSchedule(&alarm, 5000);
// }

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
        PRINT("wrong CRC\n");
        radioOff();
        mdelay(1);
        radioOn();
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
                memcpy(recvBuffer, radioBuffer.buffer, recvLen);
            }
            radioBufferReset(radioBuffer);
            recvCallback(recvBuffer, recvLen);
        }

        if (testInProgress) {
            if (timeAfter32(getJiffies(), testStartTime + MAX_TEST_TIME)) {
                endTest();
            }
        }
        if (testInProgress != prevTestInProgress || prevTestNumber != currentTestNumber) {
            if (prevTestInProgress) {
                PRINTF("Test %u ended\n", prevTestNumber);
                PRINTF("PDR=%d%%\n", prevTestPacketsRx);
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
