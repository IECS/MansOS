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

#if 0
#define RPRINTF PRINTF
#define rdebugHexdump debugHexdump
#else
#define RPRINTF(...) // nothing
#define rdebugHexdump(...) // nothing
#endif

void packetRx(Packet_t *);

// this should also account for preamble + frame delimiter size
uint8_t rxBuffer[MAX_FRAME_SIZE * SAMPLES_PER_BIT / 8];

uint8_t rxDataBuffer[MAX_PACKET_SIZE];
uint16_t rxDataBitNum;
uint16_t rxDataZeroBits;


enum {
    RSSI_START_THRESHOLD = 2700,
    RSSI_MIN_THRESHOLD = 1500,
    RSSI_STEP = 50,
};

static uint16_t rssi = 0;
uint16_t rssiThreshold = RSSI_START_THRESHOLD;
static uint16_t lastPacketRxTime;
uint16_t pckReceived;
static uint16_t numTestsWithoutPackets;


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

static inline void rxDataReset()
{
    rxDataBitNum = 0;
    rxDataZeroBits = 0;
}

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

    if (length != sizeof(Frame_t)) {
        RPRINTF("bad length: %u\n", length);
        return false;
    }
    
    Frame_t *frame = (Frame_t *) rxDataBuffer;
    if (crc16((uint8_t *) frame, sizeof(frame->data)) != frame->crc) {
        RPRINTF("bad checksum\n");
        return false;
    }

    packetRx(&frame->data);
    return true;
}

static inline void decodeOctet(uint8_t octet, uint8_t byteArray[8])
{
    byteArray[0] = (octet & (1 << 0)) ? 1 : 0;
    byteArray[1] = (octet & (1 << 1)) ? 1 : 0;
    byteArray[2] = (octet & (1 << 2)) ? 1 : 0;
    byteArray[3] = (octet & (1 << 3)) ? 1 : 0;
    byteArray[4] = (octet & (1 << 4)) ? 1 : 0;
    byteArray[5] = (octet & (1 << 5)) ? 1 : 0;
    byteArray[6] = (octet & (1 << 6)) ? 1 : 0;
    byteArray[7] = (octet & (1 << 7)) ? 1 : 0;
}

enum { BYTE_ARRAY_SIZE = 32 };

// XXX: this could be optimized
static inline uint8_t detectBit(uint8_t *array, uint16_t arrayPos)
{
    uint16_t numOnes = 0;
    uint16_t i;
    for (i = 0; i < SAMPLES_PER_BIT; ++i) {
        numOnes += array[(arrayPos + i) % BYTE_ARRAY_SIZE];
    }
    return numOnes >= SAMPLES_PER_BIT / 2 + 1;
}

static inline uint8_t detectMeBit(uint8_t *array, uint16_t arrayPos, bool *over)
{
    uint16_t numOnes1 = 0, numOnes2 = 0;
    uint16_t i = (arrayPos + 1) % BYTE_ARRAY_SIZE;
    numOnes1 += array[i++];
    i %= BYTE_ARRAY_SIZE;
    numOnes1 += array[i++];
    i %= BYTE_ARRAY_SIZE;
    numOnes1 += array[i++];
    i %= BYTE_ARRAY_SIZE;
    numOnes1 += array[i++];
    i %= BYTE_ARRAY_SIZE;
    numOnes1 += array[i++];
    i %= BYTE_ARRAY_SIZE;
    numOnes1 += array[i];
    i = (arrayPos + SAMPLES_PER_BIT + 1) % BYTE_ARRAY_SIZE;
    numOnes2 += array[i++];
    i %= BYTE_ARRAY_SIZE;
    numOnes2 += array[i++];
    i %= BYTE_ARRAY_SIZE;
    numOnes2 += array[i++];
    i %= BYTE_ARRAY_SIZE;
    numOnes2 += array[i++];
    i %= BYTE_ARRAY_SIZE;
    numOnes2 += array[i++];
    i %= BYTE_ARRAY_SIZE;
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

static bool detectDelimiterBit(uint8_t *array, uint16_t *arrayPos,
        uint16_t delimiterBitNum) 
{
    const uint8_t expectedBit = delimiterBitNum & 0x1;

    // now detect the edge
    uint8_t edge[SAMPLES_PER_BIT * 2];
    uint16_t i, idx;
    idx = (uint16_t)(*arrayPos - SAMPLES_PER_BIT) % BYTE_ARRAY_SIZE;
    for (i = 0; i < sizeof(edge); ++i) {
        idx = (idx + 1) % BYTE_ARRAY_SIZE;
        edge[i] = array[idx];
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
    RPRINTF("detectDelimiterBit: bestPos=%u, bestScore=%u\n", bestPos, bestScore);
    rdebugHexdump(edge, sizeof(edge));

    *arrayPos = (uint16_t)(*arrayPos + bestPos - 4) % BYTE_ARRAY_SIZE;
    return true;
}

static uint8_t detectDataBit(uint8_t *array, uint16_t *arrayPos, bool *over) 
{
    //TODO: copy to "edge" first
    uint8_t result = detectMeBit(array, *arrayPos, over);
    if (*over) {
        RPRINTF("detectDataBit: over!\n");
        return 0;
    }

    // now detect the edge
    uint8_t edge[SAMPLES_PER_BIT * 2];
    uint16_t i, idx;
    // idx = (*arrayPos + (uint16_t)(SAMPLES_PER_BIT - 4)) % BYTE_ARRAY_SIZE;
    idx = *arrayPos;
    for (i = 0; i < sizeof(edge); ++i) {
        idx = (idx + 1) % BYTE_ARRAY_SIZE;
        edge[i] = array[idx];
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

    *arrayPos = (*arrayPos + (uint16_t)(2 * SAMPLES_PER_BIT + bestPos - 4))
            % BYTE_ARRAY_SIZE;
    return result;
}

#define MAX_SAMPLES() (SAMPLES_PER_BIT * 2 + 4)
    
#define ENOUGH_SPACE(readPos, writePos)                                 \
    ((writePos > readPos) ?                                             \
            (writePos >= readPos + MAX_SAMPLES()) :                     \
            (writePos + BYTE_ARRAY_SIZE >= readPos + MAX_SAMPLES()))

//
// Take the packet received and convert it from samples to bits.
// End of packet is signaled by n zero bits 
// (count depends on whether Manch. Enc. is used)
// The precise ending is determined by the end of the last octet.
//
void processPacket()
{
    // at this moment the rx buffer contains delimiter sequence,
    // the data packet, and some zero samples in the end.
    RPRINTF("%lu: a potential packet received\n", getRealTime());
    rdebugHexdump(rxBuffer, 256);

    //PRINTF("p?\n");

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
    decodeOctet(*pbuf++, byteArray);
    decodeOctet(*pbuf++, byteArray + 8);
    decodeOctet(*pbuf++, byteArray + 16);
    decodeOctet(*pbuf++, byteArray + 24);

    // gap sequence must precede everything else
    if (detectBit(byteArray, 0) != 1
            || detectBit(byteArray, SAMPLES_PER_BIT) != 1) {
        RPRINTF("the packet does not start with preamble!\n");
        return;
    }

    writePos = 0; // (this much has been decoded) mod 32
    readPos = 16; // this much has been processed

    for (; pbuf != pend; ++pbuf) {
        decodeOctet(*pbuf, byteArray + writePos);
        writePos = (writePos + 8) % sizeof(byteArray);

        // there may be 0, 1, or 2 inner 'while' loops per 'for' loop
        while (ENOUGH_SPACE(readPos, writePos)) {
            switch (state) {
            case STATE_PREAMBLE:
                if (detectBit(byteArray, readPos) != 0) {
                    // nothing good here yet.
                    readPos = (readPos + 2) % sizeof(byteArray);
                    break;
                }
                state = STATE_DELIM;
                // fallthrough
            case STATE_DELIM:
                if (!detectDelimiterBit(byteArray, &readPos, delimiterBit)) {
                    // delimiter broken, cannot proceed
                    return;
                }
                ++delimiterBit;
                if (delimiterBit == FRAME_DELIM_SIZE) {
                    // RPRINTF("processPacket: process data\n");
                    state = STATE_DATA;
                    rxDataReset();
                }
                readPos = (readPos + SAMPLES_PER_BIT) % sizeof(byteArray);
                break;

            case STATE_DATA: {
                bool over;
                const uint8_t bit = detectDataBit(
                        byteArray, &readPos, &over);
                if (over || rxDataBit(bit)) {
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
    }
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
    // return true if MAX_TEST_TIME seconds has passed since last packet was received
    return (uint16_t) (TBR - lastPacketRxTime) > MAX_PACKET_BURST_TIME * 4096;
}

void rxData()
{
    for (;;) {
        // XXX: this checking happens in every iteration, it should be relatively short
        // compared to preamblete length
        if (currentTestIsOver()) {
            if (pckReceived) {
                // next test detected, calculate, print & clear statistics
                showTestStats();
                clearTestStats();
                lastPacketRxTime = TBR;
                numTestsWithoutPackets = 0;
            } else {
                if (++numTestsWithoutPackets >= 2) {
                    numTestsWithoutPackets = 0;
                    // TODO: change RSSI threshold?
                }
            }
        }

        DISABLE_INTS();
        if (!detectPreamble()) {
            ENABLE_INTS();
            continue;
        }

        // -- read whole packet, as much as the buffer allows
        register uint16_t rxOctetNum = 0;
        for (; rxOctetNum < sizeof(rxBuffer); ++rxOctetNum) {
            rxBuffer[rxOctetNum] = rxOctet();
        }

        ENABLE_INTS();

        lastPacketRxTime = TBR;

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

    toggleGreenLed();

//    PRINTF("got packet: originator=%u, #pck=%u, t=%u, act=%u, gps=(%lu %lu), time=%lu\n",
//            pck->originator,
//            pck->pckInTest,
//            pck->temperature,
//            pck->activity,
//            pck->gpsLat,
//            pck->gpsLon,
//            pck->timestamp);
//     PRINTF("got packet: #test=%u, counter=%lu, rssi = %u\n",
//             pck->testNr,
//             pck->timestamp,
//             rssi);
    lastRxTimestamp = pck->timestamp;

    updateTestStats(pck, rssi);
}

void userButtonStateChanged() {
    // decrease rssi threshold
    if (rssiThreshold > RSSI_MIN_THRESHOLD) {
        PRINTF("dec rssi threshold\n");
        rssiThreshold -= RSSI_STEP;
    }
}

//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    PRINT_INIT(127);

    initTimerB();

    TRM433_INIT();

    adcSetChannel(TRM433_RSSI_CH);

    TRM433_RX_MODE();
    TRM433_ON();

    userButtonEnable(userButtonStateChanged);

    PRINTF("Init done\n");

    rxData();
}
