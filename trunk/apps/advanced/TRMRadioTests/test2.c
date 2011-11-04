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
//   TRM433 Radio test #2. Sender sends periodic start sequences
//   Changing delays of each bit durations
//   Receiver samples received signal, and searches for start sequence
//-------------------------------------------

// FIXME - TEST NOT FINISHED - Not working properly

#include "mansos.h"
#include "leds.h"
#include "dprint.h"
#include "udelay.h"
#include "sleep.h"
#include "trm433.h"


#define SENDER 1

// sample length boundaries
#define SL_FROM 30
#define SL_TO 50

// sleep time boundaries
#define ST_FROM 10
#define ST_TO 10

// how many samples the first 0 of the start sequence must be
#define MIN_SS_BIT_LEN 30

// bit len must not differ from etalon bit (2nd bit in starting sequence)
// more than this value
#define MAX_ETALON_DELTA 3

// start sequence
static const uint8_t START_SEQ[] = { 0, 1, 0 };
#define START_SEQ_LEN sizeof(START_SEQ)


#if SENDER


static uint16_t sleepTimeout;
static uint16_t sampleHoldTimeout;

void randomize() {
    ++sleepTimeout;
    if (sleepTimeout > ST_TO) sleepTimeout = ST_FROM;
    ++sampleHoldTimeout;
    if (sampleHoldTimeout > SL_TO) sampleHoldTimeout = SL_FROM;
}

void sendStartSeq() {
    TRM433_ON();
    static uint_t i;
    for (i = 0; i < START_SEQ_LEN; ++i) {
        toggleRedLed();
        if (START_SEQ[i]) {
            TRM433_SET_DATA();
        } else {
            TRM433_CLEAR_DATA();
        }
        busyWait(sampleHoldTimeout << 7);
    }
    TRM433_OFF();
    redLedOff();
}

void init() {
    sleepTimeout = ST_FROM;
    sampleHoldTimeout = SL_FROM;
}

void sendData() {
    init();
    radioInit();
    TRM433_TX_MODE();
    TRM433_CLEAR_DATA();

    while (1) {
        randomize();
        sendStartSeq();
        toggleGreenLed();
        msleep(sleepTimeout << 8);
    }
}

#else
// receiver

void recvData() {
    PRINT_INIT(127);

    TRM433_RX_MODE();
    TRM433_ON();

    // how many bits of start sequence (SS) already received
    static uint_t bitsOk = 0;
    static uint_t currBit;
    static uint_t currBitLen = 0;
    static uint_t etalonBitLen = 0;

    // debug info - start bit lengths and RSSI
    static uint_t bitLens[START_SEQ_LEN];
    static uint_t rssi[START_SEQ_LEN];

    while (1) {
        currBit = TRM433_READ_DATA();
        if (currBit == START_SEQ[bitsOk]) {
            ++currBitLen;
        } else {
            // bit changed. If previous bit had enough samples, move to
            // next SS bit

            //rssi[1] = TRM433_READ_RSSI();

            if (currBitLen >= MIN_SS_BIT_LEN) {
                // first bit ok, move to next sample
                if (bitsOk == 1) {
                    // second SS sequence bit, save its length
                    // the remaining SS bits must be approximately the same length
                    etalonBitLen = currBitLen;
                    ++bitsOk;
                } else if (bitsOk > 1) {
                    // remaining bits, check their correspondence to etalon
                    if (abs(currBitLen - etalonBitLen) <= MAX_ETALON_DELTA) {
                        ++bitsOk;
                    } else {
                        // current bit differs too much from etalon bit len
                        // this sequence is considered burst, reset counters
                        bitsOk = 0;
                    }
                } else {
                    // first bit
                    ++bitsOk;
                }
            } else {
                // current bit very short, considered burst, reset counters
                bitsOk = 0;
            }

            if (bitsOk == START_SEQ_LEN) {
                // starting sequence received
                PRINTF("Got it! etalon = %i\n",
                        etalonBitLen);
                bitsOk = 0;
            }

            if (currBit == START_SEQ_LEN[bitsOk]) {
                // this is the first sample for this bit value
                currBitLen = 1;
            } else {
                // the last bit broke the parsing sequence, reset counter
                currBitLen = 0;
            }

            setLeds(bitsOk);
        }
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

