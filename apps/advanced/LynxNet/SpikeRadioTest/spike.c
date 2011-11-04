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
#include <hil/sleep.h>
#include <hil/fstream.h>
#include <hil/extflash.h>
#include <lib/random.h>
#include <string.h>
#include <lib/nmea/nmea.h>
#include <lib/nmea/nmea_stream.h>
#include <hil/humidity.h>

#define ORIGINATOR_ID 1 // the local identifier

#define pauseBetweenPackets() mdelay(100)


// ------------------------------------
// radio tx stuff 
// ------------------------------------

// true if there is a packet to send
volatile bool packetToSend;

// the data that is going to be sent
uint8_t txBuffer[MAX_PACKET_SIZE];
uint16_t txBufferSize;
uint16_t numTxSuccessiveZeroBits;

void txNextBit(void);

//
// timer A interrupt
//
interrupt (TIMERA1_VECTOR) timerA1Int()
{
    switch (TAIV) {
    case 2: // CCR1
        CCR1 += 4 * BIT_LEN_USEC;
        if (packetToSend) txNextBit();
        break;
    case 4: // CCR2 not used
        break;
    case 10: { // CCR0
        extern uint32_t realTime;
        // increment the 'real time' value
        ++realTime;
        CCR1 = 4 * BIT_LEN_USEC;
    }
        break;
    }
}

static inline void initTimer()
{
    TACTL = TACLR;
    TACCTL0 = CCIE;
    TACCTL1 = CCIE;
    TACCR0 = 4 * 1000; // ~once per ms
    TACCR1 = 4 * BIT_LEN_USEC;
    TACTL = TASSEL_SMCLK | TAIE | MC_UPTO_CCR0;

    // start it
    TACTL |= MC_UPTO_CCR0;
}

#define startTxBit(bit)  TRM433_WRITE_DATA_FAST(bit);

static inline uint16_t txDataBit(uint16_t bitNum)
{
    const uint8_t bit = txBuffer[bitNum / 8] & (1 << (bitNum & 0x7));

#if ENCODING_MANCHESTER

    // do Manchester encoding on the run
    static uint8_t b;
    startTxBit(b ^ (bit ? 1 : 0));
    bitNum += b;
    b ^= 1;

#else

    // do bit stuffing on the run
    if (bit) {
        numTxSuccessiveZeroBits = 0;
        startTxBit(1);
        ++bitNum;
    } else if (numTxSuccessiveZeroBits == MAX_SUCCESSIVE_ZERO_BITS) {
        numTxSuccessiveZeroBits = 0;
        startTxBit(1);
    } else {
        // XXX: if the data ends with 4+ zero bits, the ending "1" bit
        // is never transmitted!
        ++numTxSuccessiveZeroBits;
        startTxBit(0);
        ++bitNum;
    }

#endif

    return bitNum;
}

void prepareToTx(uint8_t *data, uint16_t dataSize)
{
#if ENCODING_HAMMING

    uint16_t i, j;
    if (dataSize > MAX_PACKET_SIZE / 2) dataSize = MAX_PACKET_SIZE / 2;
    for (i = 0, j = 0; i < dataSize; ++i, j += 2) {
        txBuffer[j] = hammingEncode(data[i] & 0xf);
        txBuffer[j + 1] = hammingEncode(data[i] >> 4);
    }
    txBufferSize = dataSize * 2;

#else

    if (dataSize > MAX_PACKET_SIZE) dataSize = MAX_PACKET_SIZE;
    memcpy(txBuffer, data, dataSize);
    txBufferSize = dataSize;

#endif
}

enum TxState {
    STATE_IDLE,
    STATE_PREAMBLE,
    STATE_DELIM,
    STATE_DATA,
    STATE_EPILOGUE,
};

void txNextBit()
{
    static enum TxState state = STATE_IDLE;
    static uint16_t bitNum; // the number of bit being sent

    switch (state) {
    case STATE_IDLE:
        startTxBit(0);
        ++bitNum;
        if (bitNum >= PAUSE_SIZE) {
            bitNum = 0;
            // if there is data to tx...
            if (txBufferSize) {
                state = STATE_PREAMBLE;
                // redLedOn();
                startTxBit(1);
            }
        }
        break;

    case STATE_PREAMBLE:
        startTxBit(1);
        ++bitNum;
        if (bitNum >= PREAMBLE_SIZE) {
            bitNum = 0;
            state = STATE_DELIM;
        }
        break;

    case STATE_DELIM:
        startTxBit(bitNum & 0x1);
        ++bitNum;
        if (bitNum >= FRAME_DELIM_SIZE) {
            bitNum = 0;
            state = STATE_DATA;
        }
        break;

    case STATE_DATA:
        bitNum = txDataBit(bitNum);
        if (bitNum >= txBufferSize * 8) {
            numTxSuccessiveZeroBits = 0;
            // redLedOff();
            bitNum = 0;
            state = STATE_EPILOGUE;
        }
        break;

    case STATE_EPILOGUE:
        ++bitNum;
        if (bitNum <= EPILOGUE_SIZE) {
            startTxBit(1);            
        } else {
            bitNum = 0;
            state = STATE_IDLE;
            startTxBit(0);
            txBufferSize = 0;
            packetToSend = false;
        }
        break;
    }
}

bool radioTxPacket(Packet_t *p) {
    Frame_t frame;

    // if there already is a packet...
    if (packetToSend) return false;

    memcpy(&frame.data, p, sizeof(frame.data));

    frame.crc = crc16((uint8_t *) &frame, sizeof(frame.data));
    prepareToTx((uint8_t *) &frame, sizeof(frame));

    packetToSend = true;
    return true; // success
}


//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    // initialize TRM radio
    TRM433_INIT();
    // prepare TRM radio for sending
    TRM433_TX_MODE();
    TRM433_CLEAR_DATA();
    TRM433_ON();

    // initialize our special timer mode
    initTimer();

    Packet_t pck;
    pck.version = PACKET_FORMAT_VERSION;
    pck.originator = ORIGINATOR_ID;
    pck.testNr = 0;
    pck.light = 0x3456;
    pck.temperature = 0x1234;
    pck.humidity = 0x4731;

    uint32_t seqnum = 1;

    for (;;) {
        uint32_t endTime = getRealTime() + MAX_TEST_TIME * 1000;

        pck.testNr++;
        uint16_t i;
        for (i = 0; i < PACKETS_IN_TEST; ++i) {
            pck.timestamp = seqnum++;

            // send to radio
            radioTxPacket(&pck);
            // wait until transmission is completed
            while (packetToSend);

            toggleRedLed();

            pauseBetweenPackets();
        }

        while (timeAfter(endTime, getRealTime()));
    }
}
