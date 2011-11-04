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

//-------------------------------------------
//   TRM433 Radio test #1. Sender sends periodic 010101...
//   Changing delays of each bit durations
//   Receiver samples received signal, counts samples for 0 and 1,
//   sends statistics periodically to serial port
//-------------------------------------------

#include "mansos.h"
#include "leds.h"
#include "dprint.h"
#include "udelay.h"
#include "trm433.h"


#define SENDER 0

// sample length boundaries
#define SL_FROM 1
#define SL_TO 30

// how many times to repeat TX with one sample rate
#define TX_BIT_COUNT 30000

// how many times to sample before each report
#define RX_BIT_COUNT 10000


#if SENDER

void sendData() {
    static uint_t sampleLen = SL_FROM;
    TRM433_TX_MODE();
    TRM433_CLEAR_DATA();

    while (1) {
        static uint32_t i;
        for (i = 0; i < TX_BIT_COUNT; ++i) {
            busyWait(sampleLen << 2);
            TRM433_TOGGLE_DATA();
        }
        setLeds(sampleLen >> 2);
        if (++sampleLen > SL_TO) sampleLen = SL_FROM;
    }
}

#else
// receiver

void recvData() {
    PRINT_INIT(127);

    TRM433_RX_MODE();
    TRM433_ON();

    while (1) {
        static uint32_t i;
        static uint_t prevBit; // previous data value
        static uint_t currBit; // current data value
        // how many samples hold the same value: last, min and max values
        static uint_t sampleCount, minSC[2], maxSC[2];
        static uint32_t sumSC[2]; // sum of all sample counts (to calculate avg)
        static uint_t bitCount[2];
        sampleCount = 0;
        minSC[0] = minSC[1] = 0xffff;
        maxSC[0] = maxSC[1] = 0;
        sumSC[0] = sumSC[1] = 0;
        bitCount[0] = bitCount[1] = 0;
        prevBit = 0; // to not match 0/1 at the beginning
        for (i = 0; i < RX_BIT_COUNT; ++i) {
            currBit = TRM433_READ_DATA();
            if (currBit == prevBit) {
                // the same signal level holds
                ++sampleCount;
            } else {
                // data signal changed
                if (sampleCount && sampleCount < minSC[prevBit]) {
                    minSC[prevBit] = sampleCount;
                }
                if (sampleCount > maxSC[prevBit]) {
                    maxSC[prevBit] = sampleCount;
                }
                sampleCount = 1;
                ++bitCount[prevBit];
                prevBit = currBit;
                toggleRedLed();
            }
            ++sumSC[currBit];
        }
        PRINTF("0 -> min, max, avg, cnt: %i, %i, %i, %i \t"
               "1 -> min, max, avg, cnt: %i, %i, %i, %i \n",
               minSC[0], maxSC[0], (uint16_t) (sumSC[0] / bitCount[0]),
               bitCount[0],
               minSC[1], maxSC[1], (uint16_t) (sumSC[1] / bitCount[1]),
               bitCount[1]
              );
    }
}

#endif



//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    TRM433_INIT();
#if SENDER
    sendData();
#else
    recvData();
#endif
}

