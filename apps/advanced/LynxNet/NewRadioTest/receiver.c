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

#include "common.h"
#include "test_stats.h"
#include "dprint.h"
#include <user_button.h>
#include <hil/radio.h>
#include <lib/random.h>

#if 0
#define RPRINTF PRINTF
#define rdebugHexdump debugHexdump
#else
#define RPRINTF(...) // nothing
#define rdebugHexdump(...) // nothing
#endif

// this should also account for preamble + frame delimiter size
uint8_t rxBuffer[MAX_FRAME_SIZE * SAMPLES_PER_BIT / 8];

uint8_t rxDataBuffer[MAX_PACKET_SIZE];
uint16_t rxDataBitNum;
//uint16_t rxDataZeroBits;

void initTimerB();
void userButtonStateChanged();
void rxData(void);
void rxRssi(void);
void packetRx(Packet_t *);

enum {
    RSSI_START_THRESHOLD = 3000,
    RSSI_MIN_THRESHOLD = 1500,
    RSSI_STEP = 50,
};

static uint16_t rssi = 0;
uint16_t rssiThreshold = RSSI_START_THRESHOLD;
static uint16_t lastPacketRxTime;
static uint16_t firstTestPacketRxTime;
uint16_t pckReceived;

//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    PRINT_INIT(127);
    radioInit();

    randomInit();

    initTimerB();

    TRM433_INIT();

    adcSetChannel(TRM433_RSSI_CH);

    TRM433_RX_MODE();
    TRM433_ON();

    // userButtonEnable(userButtonStateChanged);

    PRINTF("Init done\n");

    rxData();

//    for (;;) {
//        rxRssi();
//    }
}



//
// timer A interrupt
//
interrupt (TIMERA1_VECTOR) timerA1Int()
{
    if (ALARM_TIMER_EXPIRED())
    {
        // increment the 'real time' value
        extern volatile uint32_t realTime;
        ++realTime;
    }
}

//
// timer B interrupt
//
interrupt (TIMERB1_VECTOR) sleepTimerInterrupt()
{
    if (TBIV != 14) return;
}

void initTimerB()
{
    // clear
    TBCTL = TBCLR;
    // init
    TBCTL = TBSSEL_ACLK | ID_DIV8 | TBIE | CNTL_0;
    // start
    TBCTL |= MC_CONT;
}

#if 0
void userButtonStateChanged() {
    // decrease rssi threshold
    if (rssiThreshold > RSSI_MIN_THRESHOLD) {
        PRINTF("dec rssi threshold\n");
        rssiThreshold -= RSSI_STEP;
    }
}

void rxRssi()
{
#define NUM_RSSI_SAMPLES 100
    static uint16_t rssiArray[NUM_RSSI_SAMPLES];
    uint16_t i;
    uint32_t startTime, endTime;
    extern volatile uint32_t realTime;

    startTime = realTime;
    for (i = 0; i < NUM_RSSI_SAMPLES; ++i) {
        rssiArray[i] = adcReadFast();
        mdelay(1);
    }
    endTime = realTime;

    PRINTF("time passed = %lu\n", endTime - startTime);
    // PRINTF("rssi = %u\n", rssiArray[0]);
    for (i = 0; i < NUM_RSSI_SAMPLES; ++i) {
        PRINTF("%d ", rssiArray[i]);
        if (((i + 1) & 0xf) == 0) PRINTF("\n");
        mdelay(100);
    }

    mdelay(5000 + (randomRand() % 1000));
}
#endif

// returns true on end of packet
static inline bool rxDataBit(uint8_t bit)
{
    if (bit) rxDataBuffer[rxDataBitNum / 8] |= (1 << (rxDataBitNum % 8));
    else rxDataBuffer[rxDataBitNum / 8] &= ~(1 << (rxDataBitNum % 8));

    ++rxDataBitNum;
    return (rxDataBitNum >= sizeof(rxDataBuffer) * 8);
}

static bool rxDataProcess()
{
    // cut it down to byte boundary
    uint16_t length = rxDataBitNum / 8;

    RPRINTF("a packet sucessfully received!\n");
    rdebugHexdump(rxDataBuffer, length);

#if ENCODING_HAMMING
    length = hammingDecodeInplace(rxDataBuffer, length);
    if (length) {
        RPRINTF("packet after decoding: \n");
        rdebugHexdump(rxDataBuffer, length);
    } else {
        RPRINTF("Hamming decoding failed\n");
    }
#endif
    RPRINTF("rssi=%u, threshold=%u\n", rssi, rssiThreshold);
    RPRINTF("(%u bytes)\n\n", length);

    if (length != sizeof(Frame_t)) {
        RPRINTF("bad length: %u\n", length);
        ++errorIncorrectLength;
        return false;
    }

    Frame_t *frame = (Frame_t *) rxDataBuffer;
    if (crc16((uint8_t *) frame, sizeof(frame->data)) != frame->crc) {
        RPRINTF("bad checksum\n");
        ++errorIncorrectCrc;
        return false;
    }

    packetRx(&frame->data);
    return true;
}

enum { BYTE_ARRAY_SIZE = 32 };

static inline void decodeOctet(uint8_t octet, uint8_t byteArray[], uint16_t offset)
{
#define IDX(x) ((offset + (x)) % BYTE_ARRAY_SIZE)
    byteArray[IDX(0)] = (octet & (1 << 7)) ? 1 : 0;
    byteArray[IDX(1)] = (octet & (1 << 6)) ? 1 : 0;
    byteArray[IDX(2)] = (octet & (1 << 5)) ? 1 : 0;
    byteArray[IDX(3)] = (octet & (1 << 4)) ? 1 : 0;
    byteArray[IDX(4)] = (octet & (1 << 3)) ? 1 : 0;
    byteArray[IDX(5)] = (octet & (1 << 2)) ? 1 : 0;
    byteArray[IDX(6)] = (octet & (1 << 1)) ? 1 : 0;
    byteArray[IDX(7)] = (octet & (1 << 0)) ? 1 : 0;
}

// XXX: this could be optimized
static inline uint8_t detectBit(uint8_t *array)
{
    uint16_t i;
    uint16_t numOnes = 0;
    for (i = 0; i < SAMPLES_PER_BIT; ++i) {
        numOnes += array[i];
    }
    return numOnes >= SAMPLES_PER_BIT / 2 + 1;
}

static inline uint8_t detectMeBit(uint8_t array[], bool *over)
{
    uint16_t i;
    uint16_t numOnes1 = 0, numOnes2 = 0;

    i = 1;
    numOnes1 += array[i++];
    numOnes1 += array[i++];
    numOnes1 += array[i++];
    numOnes1 += array[i++];
    numOnes1 += array[i++];
    numOnes1 += array[i];

    i = SAMPLES_PER_BIT + 1;
    numOnes2 += array[i++];
    numOnes2 += array[i++];
    numOnes2 += array[i++];
    numOnes2 += array[i++];
    numOnes2 += array[i++];
    numOnes2 += array[i];

    // the frame ends with all ones (but count zeros as ending as well)
    *over = ((numOnes1 >= 5 && numOnes2 >= 5)
            || (numOnes1 <= 0 && numOnes2 <= 0));
    return numOnes1 > numOnes2;
}

static inline uint16_t detectEdge(uint8_t *array, uint16_t offset, uint8_t bit)
{
    uint16_t score = 0;
    uint8_t *p = array + offset;
    score += *p++ ^ bit;
    score += *p++ ^ bit;
    score += *p++ ^ bit;
    score += *p++ ^ bit;
    bit ^= 1;
    score += *p++ ^ bit;
    score += *p++ ^ bit;
    score += *p++ ^ bit;
    score += *p++ ^ bit;
    return score;
}

static bool detectDelimiterStart(uint8_t *array, uint16_t *arrayPos) 
{
    // search for the first zero
    static uint8_t edge[SAMPLES_PER_BIT * 2];
    uint16_t i, idx;
    idx = *arrayPos;
    for (i = 0; i < sizeof(edge); ++i) {
        edge[i] = array[idx];
        idx = (idx + 1) % BYTE_ARRAY_SIZE;
    }

    uint16_t numOnes = 0;
    i = SAMPLES_PER_BIT + 1;
    numOnes += edge[i++];
    numOnes += edge[i++];
    numOnes += edge[i++];
    numOnes += edge[i++];
    numOnes += edge[i++];
    numOnes += edge[i];

    if (numOnes > 1) {
        // no delimiter yet
        *arrayPos = (*arrayPos + 1) % BYTE_ARRAY_SIZE;
        return false;
    }

    uint16_t bestScore = 0;
    uint16_t bestPos = 0;
    for (i = 2; i <= SAMPLES_PER_BIT; ++i) { // do not start from the very start to save time
        uint16_t score = detectEdge(edge, i, 0);
        if (score > bestScore) {
            bestScore = score;
            bestPos = i;
        }
    }

    // rdebugHexdump(edge, sizeof(edge));
    // RPRINTF("delimiterStart: %u %u %u %u\n", 1, bestPos, bestScore, *arrayPos);

    *arrayPos += (uint16_t)(SAMPLES_PER_BIT + bestPos - 4);
    *arrayPos %= BYTE_ARRAY_SIZE;
    return true;
}

static bool detectDelimiterBit(uint8_t *array, uint16_t *arrayPos,
        uint16_t delimiterBitNum) 
{
    const uint8_t expectedBit = delimiterBitNum & 0x1;

    // copy to 'edge' array
    static uint8_t edge[SAMPLES_PER_BIT * 2];
    uint16_t i, idx;
    idx = *arrayPos;
    for (i = 0; i < sizeof(edge); ++i) {
        edge[i] = array[idx];
        idx = (idx + 1) % BYTE_ARRAY_SIZE;
    }
    uint16_t bestScore = 0;
    uint16_t bestPos = 0;
    for (i = 0; i <= SAMPLES_PER_BIT; ++i) {
        uint16_t score = detectEdge(edge, i, expectedBit);
        if (score > bestScore) {
            bestScore = score;
            bestPos = i;
        }
    }
    // rdebugHexdump(edge, sizeof(edge));
    // RPRINTF("delimiterBit: %u %u %u %u\n", expectedBit, bestPos, bestScore, *arrayPos);

    *arrayPos += (uint16_t)(SAMPLES_PER_BIT + bestPos - 4);
    *arrayPos %= BYTE_ARRAY_SIZE;
    return true;
}

static uint8_t detectDataBit(uint8_t *array, uint16_t *arrayPos, bool *over) 
{
    // copy to 'edge' array
    static uint8_t edge[SAMPLES_PER_BIT * 2];
    uint16_t i, idx;
    idx = *arrayPos;
    for (i = 0; i < sizeof(edge); ++i) {
        edge[i] = array[idx];
        idx = (idx + 1) % BYTE_ARRAY_SIZE;
    }

    // rdebugHexdump(edge, sizeof(edge));

    // detect the bit
    uint8_t result = detectMeBit(edge, over);
    if (*over) {
        //RPRINTF("detectDataBit: over!\n");
        return 0;
    }

    uint16_t bestScore = 0;
    uint16_t bestPos = 0;
    for (i = 0; i <= SAMPLES_PER_BIT; ++i) {
        uint16_t score = detectEdge(edge, i, result ^ 1);
        if (score >= bestScore) {
            bestScore = score;
            bestPos = i;
        }
    }

    // 
//    static uint16_t test;
//    RPRINTF("ddb: %u %u %u %u\n", result, bestScore, bestPos, *arrayPos);
//    if (++test > 20) {
//        test = 0;
//        *over = true;
//    }
    // 

    *arrayPos += (uint16_t)(2 * SAMPLES_PER_BIT + bestPos - 4);
    *arrayPos %= BYTE_ARRAY_SIZE;
    return result;
}

//#define MAX_SAMPLES() (SAMPLES_PER_BIT * 2 + 4)

// #define MAX_SAMPLES() (SAMPLES_PER_BIT * 2)

// #define ENOUGH_SPACE(readPos, writePos)                                 
//     (writePos > readPos ?                                               
//             (writePos >= readPos + MAX_SAMPLES()) :                     
//             (writePos + BYTE_ARRAY_SIZE >= readPos + MAX_SAMPLES()))


#define CAN_ADVANCE_WRITE_POS(writePos, readPos)        \
    ((readPos >= writePos) ?                            \
            (readPos - writePos >= 8) :                 \
            (readPos + BYTE_ARRAY_SIZE - writePos >= 8))

//
// Take the packet received and convert it from samples to bits.
// End of packet is signaled by n zero bits 
// (count depends on whether ME is used)
// The precise ending is determined by the end of the last octet.
//
void processPacket()
{
    // at this moment the rx buffer contains delimiter sequence,
    // the data packet, and some zero samples in the end.
    RPRINTF("%lu: a potential packet received\n", getRealTime());
    rdebugHexdump(rxBuffer, 64);

    enum {
        STATE_PREAMBLE,
        STATE_DELIM,
        STATE_DATA,
    } state = STATE_PREAMBLE;
    uint16_t delimiterBit = 0;

    // decode 4 octets we are currently processing in this array
    static uint8_t byteArray[BYTE_ARRAY_SIZE];
    uint16_t writePos, readPos;

    uint8_t *pbuf = rxBuffer;
    const uint8_t *pend = pbuf + sizeof(rxBuffer);
    decodeOctet(*pbuf++, byteArray, 0);
    decodeOctet(*pbuf++, byteArray, 8);
    decodeOctet(*pbuf++, byteArray, 16);
    decodeOctet(*pbuf++, byteArray, 24);

    // gap sequence must precede everything else
    if (detectBit(byteArray) != 1
            || detectBit(byteArray + SAMPLES_PER_BIT) != 1) {
        RPRINTF("the packet does not start with preamble!\n");
        ++errorIncorrectPreamble;
        return;
    }

    writePos = 0; // (this much has been decoded) mod BYTE_ARRAY_SIZE
    readPos = 8; // (this much has been processed) mod BYTE_ARRAY_SIZE

    while (pbuf != pend) {
        while (CAN_ADVANCE_WRITE_POS(writePos, readPos)) {
            // RPRINTF("d.o., %u %u\n", writePos, readPos);
            decodeOctet(*pbuf++, byteArray, writePos);
            writePos = (writePos + 8) % sizeof(byteArray);
        }
//        if (readPos > writePos) {
//        }
//        if (readPos
//        decodeOctet(*pbuf, byteArray + writePos);
//        writePos = (writePos + 8) % sizeof(byteArray);

        // there may be 0, 1, or 2 inner 'while' loops per 'for' loop
//        while (!CAN_ADVANCE_WRITE_POS(readPos, writePos)) {
        switch (state) {
        case STATE_PREAMBLE:
// {
//                uint16_t tempRp = (readPos + SAMPLES_PER_BIT) % BYTE_ARRAY_SIZE;
//                if (detectBit(byteArray, tempRp) != 0) {
                    // nothing good here yet.
                    // (increment the position by 2 to be a little faster)
//                    readPos = (readPos + 2) % BYTE_ARRAY_SIZE;
//                    break;
//                }
//                // move to the start of the 0
//                while (byteArray[tempRp] == 1) {
//                    tempRp = (tempRp + 1) % BYTE_ARRAY_SIZE;
//                    readPos++;
//                }
//                readPos %= BYTE_ARRAY_SIZE;
//                state = STATE_DELIM;
//            }
            if (detectDelimiterStart(byteArray, &readPos)) {
                state = STATE_DELIM;
                ++delimiterBit; // first bit already detected
            }
            break;

        case STATE_DELIM:
            if (!detectDelimiterBit(byteArray, &readPos, delimiterBit)) {
                // delimiter broken, cannot proceed
                RPRINTF("processPacket: delim broken, bit=%u\n", delimiterBit);
                ++errorIncorrectDelimiter;
                return;
            }
            if (++delimiterBit == FRAME_DELIM_SIZE) {
                //RPRINTF("processPacket: process data\n");
                state = STATE_DATA;
                rxDataBitNum = 0;
                // advance one extra bit forward
                readPos = (readPos + SAMPLES_PER_BIT) % BYTE_ARRAY_SIZE;
            }
            break;

        case STATE_DATA: {
            bool over;
            const uint8_t bit = detectDataBit(
                    byteArray, &readPos, &over);
            if (over || rxDataBit(bit)) {
                // RPRINTF("data is over\n");
                if (!rxDataProcess()) {
                    // increase packet detection threshold
                    if (rssiThreshold < RSSI_START_THRESHOLD) {
                        PRINTF("inc rssi threshold\n");
                        rssiThreshold += RSSI_STEP;
                    }
                }
                return;
            }
            break;
        }
        }
    }
    // RPRINTF("buffer over\n");
    // rxDataProcess();
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

static inline uint8_t rxOctet() INLINE;
static inline uint8_t rxOctet()
{
    register uint16_t rxBits = 0;
    // add extra nops to compensate for different shifts
    rxBits |= TRM433_READ_DATA_FAST() << 4;
    DELAY();
    NOP1();
    rxBits |= TRM433_READ_DATA_FAST() << 3;
    DELAY();
    NOP2();
    rxBits |= TRM433_READ_DATA_FAST() << 2;
    DELAY();
    NOP3();
    rxBits |= TRM433_READ_DATA_FAST() << 1;
    DELAY();
    NOP4();
    rxBits |= TRM433_READ_DATA_FAST();
    DELAY();
    NOP5();
    rxBits |= TRM433_READ_DATA_FAST() >> 1;
    DELAY();
    NOP3();
    rxBits |= TRM433_READ_DATA_FAST() >> 2;
    DELAY();
    NOP2();
    rxBits |= TRM433_READ_DATA_FAST() >> 3;
    NOP5();
    DELAY_SMALL();
    return rxBits;
}

static bool detectPreamble()
{
    rssi = adcReadFast();

    return rssi > rssiThreshold;
}

static inline bool currentTestIsOver()
{
    // return true if MAX_PACKET_BURST_TIME seconds has passed since last packet was received
    return (uint16_t) (TBR - firstTestPacketRxTime) > MAX_PACKET_BURST_TIME * 4096;
}

static inline bool currentTestIsUnderWay()
{
    return firstTestPacketRxTime != 0;
}

void rxData()
{
    for (;;) {
        // XXX: this checking happens in every iteration, it should be relatively short
        // compared to preamblete length
        if (currentTestIsUnderWay() && currentTestIsOver()) {
            // next test detected, calculate, print & clear statistics
            printTestStats();
            lastPacketRxTime = TBR;
            firstTestPacketRxTime = 0;
        }

        DISABLE_INTS();
        if (!detectPreamble()) {
            ENABLE_INTS();
            // PRINTF("no pkt, rssi=%d\n", rssi);
            // mdelay(1000);
            continue;
        }

        // -- read whole packet, as much as the buffer allows
        register uint16_t rxOctetNum = 0;
        for (; rxOctetNum < sizeof(rxBuffer); ++rxOctetNum) {
            rxBuffer[rxOctetNum] = rxOctet();
        }

        ENABLE_INTS();

        lastPacketRxTime = TBR;
        if (!currentTestIsUnderWay()) firstTestPacketRxTime = TBR;

        // -- now process the data in the buffer
        processPacket();
    }
}

//
// receive and process the packet
//
void packetRx(Packet_t *pck)
{
    static uint32_t lastRxTimestamp;

    if (pck->version != PACKET_FORMAT_VERSION) {
        PRINTF("bad version: %u\n", pck->version);
        return;
    }

    // if it's a duplicate, ignore it
    if (pck->timestamp == lastRxTimestamp) return;

    // toggleGreenLed();

//    PRINTF("got packet: originator=%u, #pck=%u, t=%u, act=%u, gps=(%lu %lu), time=%lu\n",
//            pck->originator,
//            pck->pckInTest,
//            pck->temperature,
//            pck->activity,
//            pck->gpsLat,
//            pck->gpsLon,
//            pck->timestamp);

    // PRINTF("got packet: #test %u, counter %lu, rssi %u\n",
    //         pck->testNr,
    //         pck->timestamp,
    //         rssi);

    lastRxTimestamp = pck->timestamp;

    updateTestStats(pck, rssi);
}
