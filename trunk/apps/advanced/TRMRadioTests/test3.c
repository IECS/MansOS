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

#include "mansos.h"
#include "leds.h"
#include "dprint.h"
#include "udelay.h"
#include "trm433.h"

#include "timers.h"
#include "random.h"
#include <string.h>

#define SENDER 1

// length of single bit in usec
//#define BIT_LEN_USEC 50
//#define BIT_LEN_USEC 102
#define BIT_LEN_USEC 200
//#define BIT_LEN_USEC 500
//#define BIT_LEN_USEC 1000
//#define BIT_LEN_USEC 10000

// preamble size in bits
#define PREAMBLE_SIZE 1000

// frame delimiter size in bits
#define FRAME_DELIM_SIZE 6
//#define FRAME_DELIM_SIZE 12

// pause size in bits
#define PAUSE_SIZE 1000
//#define PAUSE_SIZE 10

#define bit_sizeof(x)   (sizeof(x) * 8)

#if SENDER

#define INLINE __attribute__((always_inline))

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

static inline void startTxBit(uint8_t bit) INLINE;
static inline void startTxBit(uint8_t bit) {
    TRM433_WRITE_DATA(bit);
}

//enum { TX_BUFFER_SIZE = 256 / 29 };
enum { TX_BUFFER_SIZE = 10 };
uint8_t txBuffer[TX_BUFFER_SIZE];

void initTxBuffer()
{
    uint16_t i;
    for (i = 0; i < TX_BUFFER_SIZE; ++i) {
//        txBuffer[i] = i * 7;
//        txBuffer[i] = i;
        txBuffer[i] = 0xff;
    }
}

void txNextBit()
{
    enum State {
        STATE_IDLE,
        STATE_PREAMBLE,
        STATE_DELIM,
        STATE_DATA,
    };

    static enum State state = STATE_IDLE;
    static uint16_t bitNum; // the number of bit being sent

    switch (state) {
    case STATE_IDLE:
        startTxBit(0); // XXX - needed ?
        ++bitNum;
        if (bitNum >= PAUSE_SIZE) {
            bitNum = 0;
            state = STATE_PREAMBLE;

            redLedOn();
            startTxBit(1);
        }
        break;

    case STATE_PREAMBLE:
        // startTxBit(bitNum & 0x1);// XXX
        ++bitNum;
        if (bitNum >= PREAMBLE_SIZE) {
            bitNum = 0;
            state = STATE_DELIM;
        }
        break;

    case STATE_DELIM:
        // send the bits less frequently
        startTxBit(bitNum & 0x2);
        ++bitNum;
        if (bitNum >= FRAME_DELIM_SIZE) {
            bitNum = 0;
            state = STATE_DATA;
        }
        break;

    case STATE_DATA:
    {
        // tx a data bit
        uint16_t byte = bitNum / 8;
        uint16_t bit = bitNum & 0x7;
        static uint8_t b;
        b = !b;
        const uint8_t bitToSend = txBuffer[byte] & (1 << bit);
        startTxBit(bitToSend ^ b);
        if (!b) {
            ++bitNum;
            if (bitNum >= bit_sizeof(txBuffer)) {
                redLedOff();
                bitNum = 0;
                state = STATE_IDLE;
                startTxBit(0);
            }
        }
    }
        break;
    }
}

void sendData() {
    TRM433_TX_MODE();
    TRM433_CLEAR_DATA();

    initTxBuffer();

    initTimer();
    startTimer();

    for (;;);
}

#else
// receiver


#define SAMPLE_COUNT 10
//#define GOOD_SAMPLE_COUNT 8
#define GOOD_SAMPLE_COUNT 10

#define SERIES_COUNT 8
#define GOOD_SERIES_COUNT 7

// #define SERIES_COUNT 10
// #define GOOD_SERIES_COUNT 9

// #define SERIES_COUNT 20
// #define GOOD_SERIES_COUNT 19

#if 0
static uint8_t series[SERIES_COUNT] = {0};

static inline bool detectBit()
{
    uint16_t c = 0;
    uint16_t i;
    for (i = 0; i < SERIES_COUNT; ++i) {
        c += series[i];
    }

    return c >= GOOD_SERIES_COUNT;
}

void recvData() {
    PRINT_INIT(127);

    randomInit();

    TRM433_RX_MODE();
    TRM433_ON();

    mdelay(500);

    uint32_t lastTime = getRealTime();
    bool currentBit = 0;
    uint16_t currentSeries = 0;
    for (;;) {
        uint16_t numSamplesOk = 0;
        numSamplesOk += TRM433_READ_DATA();
        numSamplesOk += TRM433_READ_DATA();
        numSamplesOk += TRM433_READ_DATA();
        numSamplesOk += TRM433_READ_DATA();
        numSamplesOk += TRM433_READ_DATA();
        numSamplesOk += TRM433_READ_DATA();
        numSamplesOk += TRM433_READ_DATA();
        numSamplesOk += TRM433_READ_DATA();
        numSamplesOk += TRM433_READ_DATA();
        numSamplesOk += TRM433_READ_DATA();

        if (numSamplesOk > SAMPLE_COUNT) blueLedOn();

        series[currentSeries] = (numSamplesOk >= GOOD_SAMPLE_COUNT);
        bool bitDetected = detectBit();

        if (bitDetected != currentBit) {
            uint32_t now = getRealTime();
            PRINTF("%s, time=%lu\n", (bitDetected ? "+" : "-"), now - lastTime);
            lastTime = now;

            currentBit = bitDetected;
            if (bitDetected) redLedOn();
            else redLedOff();
        }

        currentSeries = (currentSeries + 1) % SERIES_COUNT;

//         toggleRedLed();
//         mdelay(1000u + (randomRand() & 1023));

//         uint32_t now = getRealTime();
//         PRINTF("time=%lu\n", now - lastTime);
//         lastTime = now;
    }
}

#else

uint8_t rxBuffer[256];
static uint16_t rxBitNum;

static inline uint8_t countBits(uint8_t byte)
{
    uint16_t result = 0;
    if (byte & (1 << 0)) result++;
    if (byte & (1 << 1)) result++;
    if (byte & (1 << 2)) result++;
    if (byte & (1 << 3)) result++;
    if (byte & (1 << 4)) result++;
    if (byte & (1 << 5)) result++;
    if (byte & (1 << 6)) result++;
    if (byte & (1 << 7)) result++;
    return result;
}

uint16_t countAllBits()
{
    uint16_t i, result = 0;
    for (i = 0; i < sizeof(rxBuffer); ++i) {
        result += countBits(rxBuffer[i]);
    }
    return result;
}

// void rxBit(uint8_t bit)
// {
//     if (bit) rxBuffer[currByte] |= 1 << currBit;
//     else rxBuffer[currByte] &= ~(1 << currBit);

//     if (++currBit == 8) {
//         currBit = 0;
//         if (++currByte == sizeof(rxBuffer)) {
//             currByte = 0;
//             PRINTF("time=%lu\n", getRealTime());
//             PRINTF("bits on = %u\n", countAllBits());
//             debugHexdump(rxBuffer, sizeof(rxBuffer));

//             mdelay(1000 + (randomRand() & 511));
//         }
//     }
// }

static inline bool detectPreamble()
{
    enum {
        PREAMBLE_BIT_COUNT = 64,
//        PREAMBLE_GOOD_BIT_COUNT = 48,
        PREAMBLE_GOOD_BIT_COUNT = 52,
    };

    ASSERT(!(rxBitNum & 0x7));
    uint16_t bitPos = rxBitNum; //  & ~0x7;
    if (bitPos < PREAMBLE_BIT_COUNT) bitPos += bit_sizeof(rxBuffer);
    bitPos -= PREAMBLE_BIT_COUNT;

    uint16_t goodBits = 0;
    const uint16_t bytePos = bitPos / 8;
    uint16_t i;
    for (i = bytePos; i < bytePos + PREAMBLE_BIT_COUNT / 8; ++i) {
        goodBits += countBits(rxBuffer[i]);
    }
    return goodBits >= PREAMBLE_GOOD_BIT_COUNT;
}

static inline bool detectStartOfPacket()
{
    enum {
        DELIM_BIT_COUNT = 32,
        DELIM_GOOD_BIT_COUNT = 28,
    };

    uint16_t bitPos = rxBitNum;
    if (bitPos < DELIM_BIT_COUNT) bitPos += bit_sizeof(rxBuffer);
    bitPos -= DELIM_BIT_COUNT;

    uint16_t badBits = 0;
    const uint16_t bytePos = bitPos / 8;
    uint16_t i;
    for (i = bytePos; i < bytePos + DELIM_BIT_COUNT / 8; ++i) {
        badBits += countBits(rxBuffer[i]);
    }
    return DELIM_BIT_COUNT - badBits >= DELIM_GOOD_BIT_COUNT;
}

static inline bool detectEndOfPacket()
{
    enum {
         IDLE_BIT_COUNT = 64,
         IDLE_GOOD_BIT_COUNT = 56,
    };

    uint16_t bitPos = rxBitNum;
    if (bitPos < IDLE_BIT_COUNT) bitPos += bit_sizeof(rxBuffer);
    bitPos -= IDLE_BIT_COUNT;

    uint16_t badBits = 0;
    const uint16_t bytePos = bitPos / 8;
    uint16_t i;
    for (i = bytePos; i < bytePos + IDLE_BIT_COUNT / 8; ++i) {
        badBits += countBits(rxBuffer[i]);
    }
    return IDLE_BIT_COUNT - badBits >= IDLE_GOOD_BIT_COUNT;
}

void processPacket()
{
    // at this moment the rx buffer contains a delimiter sequence,
    // the data packet, and some zero bytes in the end.
    PRINTF("%lu: a packet received, total size in bytes=%u!\n",
            getRealTime(), rxBitNum / 8);
    debugHexdump(rxBuffer, rxBitNum / 8);

}

void rxBit(uint8_t bit)
{
    enum State {
        STATE_IDLE,
        STATE_PREAMBLE,
        STATE_PACKET, // frame delimiter and data
    };

    static enum State state = STATE_IDLE;

    if (bit) rxBuffer[rxBitNum / 8] |= 1 << (rxBitNum & 0x7);
    else rxBuffer[rxBitNum / 8] &= ~(1 << (rxBitNum & 0x7));

    ++rxBitNum;
    rxBitNum &= (bit_sizeof(rxBuffer) - 1);

    switch (state) {
    case STATE_IDLE:
        if (!(rxBitNum & 0x7) && detectPreamble()) {
            state = STATE_PREAMBLE;
        }
        break;

    case STATE_PREAMBLE:
        if (!(rxBitNum & 0x7) && detectStartOfPacket()) {
            state = STATE_PACKET;
            // start from beginning
            rxBitNum = 0;
        }
        break;

    case STATE_PACKET:
        if (rxBitNum > 64
                && !(rxBitNum & 0x7)
                && detectEndOfPacket()) {
            state = STATE_IDLE;
            processPacket();
            memset(rxBuffer, 0, sizeof(rxBuffer));
            rxBitNum = 0;
        }
        break;
    }
}

void recvData()
{
    PRINT_INIT(127);

    randomInit();

    TRM433_RX_MODE();
    TRM433_ON();

    mdelay(500);

    for (;;) {
        rxBit(TRM433_READ_DATA());
    }
}

#endif


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

