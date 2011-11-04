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

//
// Constant pattern sending test;
// there are different patterns predefined;
// the receiver just samples at maximum frequency
// and prints out the results.
//

#include "mansos.h"
#include "leds.h"
#include "dprint.h"
#include "udelay.h"
#include "trm433.h"
#include "radio.h"
#include "timers.h"
#include "random.h"
#include <string.h>

#define SENDER 1

// for the older devices (the ones connected to Tmote Sky)
#undef TRM433_TRSEL_PIN
#define TRM433_TRSEL_PIN 6

#undef TRM433_RSSI_CH
#define TRM433_RSSI_CH 0

// length of single bit in usec
//#define BIT_LEN_USEC 40
#define BIT_LEN_USEC 100
//#define BIT_LEN_USEC 200
//#define BIT_LEN_USEC 500
//#define BIT_LEN_USEC 1000
//#define BIT_LEN_USEC 10000u

//
// timer A interrupt
//
interrupt (TIMERA1_VECTOR) timerA1Int()
{
    if (ALARM_TIMER_EXPIRED())
    {
        // increment the 'real time' value
        extern uint32_t realTime;
        ++realTime;
    }
}

#if SENDER

enum {
    PATTERN_ZEROS, // the same as not sending anything
                   // - if the radio is working corectly
    PATTERN_ONES,  // constant ones
    PATTERN_WALL,  // for lack of a better name - great wall: |-|_|-|_|-|_|-|
    PATTERN_TWO_ONE,
    PATTERN_MAX_GAPS,
    PATTERN_PACKETS,
};

//#define USED_PATTERN PATTERN_ONES
//#define USED_PATTERN PATTERN_ZEROS
//#define USED_PATTERN PATTERN_WALL
//#define USED_PATTERN PATTERN_TWO_ONE
//#define USED_PATTERN PATTERN_MAX_GAPS
#define USED_PATTERN PATTERN_PACKETS

void txNextBit(void);

interrupt (TIMERB1_VECTOR) timerB1Int()
{
    switch (TBIV) {
    case 2: // CCR1 not used
        break;
    case 4: // CCR2 not used
        break;
    case 14:
        txNextBit();
        break;
    }
}

static inline void initTimer()
{
    TBCTL = TBCLR;
    TBCCTL0 = CCIE;
    TBCCR0 = 4 * BIT_LEN_USEC;
    TBCTL = TBSSEL_SMCLK | TBIE | MC_UPTO_CCR0;
}

static inline void startTimer() INLINE;
static inline void startTimer()
{
    TBCTL |= MC_UPTO_CCR0;
}

static inline void stopTimer() INLINE;
static inline void stopTimer()
{
    TBCTL &= ~MC_UPTO_CCR0;
}

#define startTxBit(bit) TRM433_WRITE_DATA_FAST(bit)

void txNextBit()
{
    static uint16_t bitNum; // the number of bit being sent
    ++bitNum;

    switch (USED_PATTERN) {
    case PATTERN_ZEROS:
        startTxBit(0);
        break;
    case PATTERN_ONES:
        startTxBit(1);
        break;
    case PATTERN_WALL:
        startTxBit(bitNum & 0x1);
        break;
    case PATTERN_TWO_ONE: {
        static uint8_t i;
        ++i;
        if (i < 5) {
            startTxBit(0);
        } else {
            startTxBit(1);
            i = 0;
        }
    }
        break;
    case PATTERN_MAX_GAPS:
        if (bitNum <= 6) startTxBit(0);
        else startTxBit(bitNum & 0x1);
        if (bitNum > 12) bitNum = 0;
        break;
    case PATTERN_PACKETS:
        if (bitNum <= 10000) {
            // startTxBit(bitNum & 0x1);
            startTxBit(1);
            if (bitNum == 10000) redLedOff();
        }
        else if (bitNum <= 20000) {
            startTxBit(0);
        }
        else {
            bitNum = 0;
            redLedOn();
        }
        break;
    }
}

void sendData() {
    TRM433_TX_MODE();
    TRM433_CLEAR_DATA();

    initTimer();
    startTimer();

    for (;;) {
        toggleRedLed();
        mdelay(1000);
    }
}

#else
// receiver

#define MAX_SAMPLES_PER_BIT 8
//#define MAX_PACKET_SIZE 128
#define MAX_PACKET_SIZE 32

uint8_t rxBuffer[MAX_SAMPLES_PER_BIT * MAX_PACKET_SIZE];
static uint16_t rxByte;

static inline uint8_t countBits(uint8_t byte) INLINE;
static inline uint8_t countBits(uint8_t byte)
{
    return (byte & (1 << 0))
            + ((byte & (1 << 1)) >> 1)
            + ((byte & (1 << 2)) >> 2)
            + ((byte & (1 << 3)) >> 3)
            + ((byte & (1 << 4)) >> 4)
            + ((byte & (1 << 5)) >> 5)
            + ((byte & (1 << 6)) >> 6)
            + ((byte & (1 << 7)) >> 7);
}

uint16_t countAllBits()
{
    uint16_t i, result = 0;
    for (i = 0; i < sizeof(rxBuffer); ++i) {
        result += countBits(rxBuffer[i]);
    }
    return result;
}

//
// The target is to have 8 samples for each bits.
// These delays are far from perfect - 
// depending on DCO frequency they can be too small or too large.
//
#define DELAY() { NOP10(); NOP10(); NOP10(); NOP6(); };
// this delay is used after the last sample;
// all the processing after it takes ~10 CPU cycles, so discount those.
#define DELAY_SMALL() { NOP10(); NOP10(); NOP6(); };

//
// This algo uses ~16 cpu cycles per sample
//
static inline uint8_t rxOctet() INLINE;
static inline uint8_t rxOctet()
{
    register uint16_t rxBits = 0;
    rxBits |= TRM433_READ_DATA_FAST() >> 3;
    DELAY();
    // add extra nops to compensate for different shifts
    NOP5();
    rxBits |= TRM433_READ_DATA_FAST() >> 2;
    DELAY();
    NOP2();
    rxBits |= TRM433_READ_DATA_FAST() >> 1;
    DELAY();
    NOP3();
    rxBits |= TRM433_READ_DATA_FAST();
    DELAY();
    NOP5();
    rxBits |= TRM433_READ_DATA_FAST() << 1;
    DELAY();
    NOP4();
    rxBits |= TRM433_READ_DATA_FAST() << 2;
    DELAY();
    NOP3();
    rxBits |= TRM433_READ_DATA_FAST() << 3;
    DELAY();
    NOP2();
    rxBits |= TRM433_READ_DATA_FAST() << 4;
    return rxBits;
}

void rxData()
{
    for (;;) {
        rxBuffer[rxByte] = rxOctet();
        DELAY_SMALL();
        if (++rxByte == sizeof(rxBuffer)) {
            rxByte = 0;

            PRINTF("time=%lu, bits-on=%u\n",
                    getRealTime(), countAllBits());
            debugHexdump(rxBuffer, sizeof(rxBuffer));

            toggleRedLed();

            mdelay(1000 + (randomRand() % 512));
        }
    }
}

void recvData()
{
    PRINT_INIT(127);
    radioInit();
    randomInit();

    TRM433_RX_MODE();

    mdelay(500);

    rxData();

    // for (;;) {
    //     uint16_t rssi = TRM433_READ_RSSI(); // adcReadFast();
    //     PRINTF("rssi = %u\n", rssi);
    //     mdelay(1000 + (randomRand() % 512));
    // }
}

#endif

//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    TRM433_INIT();
    TRM433_ON();

#if SENDER
    sendData();
#else
    recvData();
#endif
}
