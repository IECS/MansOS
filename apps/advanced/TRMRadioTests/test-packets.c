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

#define SENDER 0

//
// Possible packet encodings:
// * plain (with bit stuffing)
// * Hamming encoded (with bit stuffing)
// * Manchester encoded
// * double encoded (first Hamming codes, then Manchester encoding)
//
// Bit stuffing means that after each four "0" bits "1" bit is sent,
// and ignored by the receiver
//
enum {
    ENCODING_NONE,
    ENCODING_HAMMING,
    ENCODING_MANCHESTER,
    ENCODING_BOTH,
};

#define USED_ENCODING  ENCODING_NONE
//#define USED_ENCODING  ENCODING_HAMMING

enum {
    MIN_SAMPLES_PER_BIT = 6,
    SAMPLES_PER_BIT = 8,
    MAX_SAMPLES_PER_BIT = 10,

    // MAX_PACKET_SIZE = 128,
    MAX_PACKET_SIZE = 32, // in bytes

    MAX_SUCCESSIVE_ZERO_BITS = 4,
};

//
// Length of single bit in usec;
// if taking one sample is ~20 CPU cycles long,
// then 40usec should allow to take sample 8 times per bit.
//
//#define BIT_LEN_USEC 40     // 25000 bps
#define BIT_LEN_USEC 100  // 10000 bps
//#define BIT_LEN_USEC 1000
//#define BIT_LEN_USEC 10000

// preamble length in usec
//#define PREAMBLE_LENGTH_USEC 4000
//#define PREAMBLE_LENGTH_USEC 2000
//#define PREAMBLE_LENGTH_USEC 1000

// preamble size in bits
//#define PREAMBLE_SIZE (PREAMBLE_LENGTH_USEC / BIT_LEN_USEC)

#define PREAMBLE_SIZE 20

// gap (that follows after preamble) size in bits
//#define GAP_SIZE 6

// frame delimiter size in bits
#define FRAME_DELIM_SIZE 6 // 010101

// pause size in bits
#define PAUSE_SIZE 100


const uint8_t hammingEncodeTable[] = {
    0x0,
    0x87,
    0x99,
    0x1e,
    0xaa,
    0x2d,
    0x33,
    0xb4,
    0x4b,
    0xcc,
    0xd2,
    0x55,
    0xe1,
    0x66,
    0x78,
    0xff
};

const uint8_t hammingDecodeTable[] = {
    0x00, 0x00, 0x00, 0xff, 0x00, 0xff, 0xff, 0x01,
    0x00, 0xff, 0xff, 0x08, 0xff, 0x05, 0x03, 0xff,
    0x00, 0xff, 0xff, 0x06, 0xff, 0x0b, 0x03, 0xff,
    0xff, 0x02, 0x03, 0xff, 0x03, 0xff, 0x03, 0x03,
    0x00, 0xff, 0xff, 0x06, 0xff, 0x05, 0x0d, 0xff,
    0xff, 0x05, 0x04, 0xff, 0x05, 0x05, 0xff, 0x05,
    0xff, 0x06, 0x06, 0x06, 0x07, 0xff, 0xff, 0x06,
    0x0e, 0xff, 0xff, 0x06, 0xff, 0x05, 0x03, 0xff,
    0x00, 0xff, 0xff, 0x08, 0xff, 0x0b, 0x0d, 0xff,
    0xff, 0x08, 0x08, 0x08, 0x09, 0xff, 0xff, 0x08,
    0xff, 0x0b, 0x0a, 0xff, 0x0b, 0x0b, 0xff, 0x0b,
    0x0e, 0xff, 0xff, 0x08, 0xff, 0x0b, 0x03, 0xff,
    0xff, 0x0c, 0x0d, 0xff, 0x0d, 0xff, 0x0d, 0x0d,
    0x0e, 0xff, 0xff, 0x08, 0xff, 0x05, 0x0d, 0xff,
    0x0e, 0xff, 0xff, 0x06, 0xff, 0x0b, 0x0d, 0xff,
    0x0e, 0x0e, 0x0e, 0xff, 0x0e, 0xff, 0xff, 0x0f,
    0x00, 0xff, 0xff, 0x01, 0xff, 0x01, 0x01, 0x01,
    0xff, 0x02, 0x04, 0xff, 0x09, 0xff, 0xff, 0x01,
    0xff, 0x02, 0x0a, 0xff, 0x07, 0xff, 0xff, 0x01,
    0x02, 0x02, 0xff, 0x02, 0xff, 0x02, 0x03, 0xff,
    0xff, 0x0c, 0x04, 0xff, 0x07, 0xff, 0xff, 0x01,
    0x04, 0xff, 0x04, 0x04, 0xff, 0x05, 0x04, 0xff,
    0x07, 0xff, 0xff, 0x06, 0x07, 0x07, 0x07, 0xff,
    0xff, 0x02, 0x04, 0xff, 0x07, 0xff, 0xff, 0x0f,
    0xff, 0x0c, 0x0a, 0xff, 0x09, 0xff, 0xff, 0x01,
    0x09, 0xff, 0xff, 0x08, 0x09, 0x09, 0x09, 0xff,
    0x0a, 0xff, 0x0a, 0x0a, 0xff, 0x0b, 0x0a, 0xff,
    0xff, 0x02, 0x0a, 0xff, 0x09, 0xff, 0xff, 0x0f,
    0x0c, 0x0c, 0xff, 0x0c, 0xff, 0x0c, 0x0d, 0xff,
    0xff, 0x0c, 0x04, 0xff, 0x09, 0xff, 0xff, 0x0f,
    0xff, 0x0c, 0x0a, 0xff, 0x07, 0xff, 0xff, 0x0f,
    0x0e, 0xff, 0xff, 0x0f, 0xff, 0x0f, 0x0f, 0x0f
};

static inline uint8_t hammingEncode(uint8_t nibble)
{
    return hammingEncodeTable[nibble];
}

static inline bool hammingDecode(uint8_t byte, uint8_t *result)
{
    uint8_t b = hammingDecodeTable[byte];
    if (b == 0xff) return false;
    *result = b;
    return true;
}

static inline uint16_t hammingDecodeInplace(uint8_t *data, uint16_t length)
{
    uint16_t i, j;
    length /= 2;
    for (i = 0, j = 0; i < length; ++i, j += 2) {
        uint8_t b1, b2;
        if (!hammingDecode(data[j], &b1)) return 0;
        if (!hammingDecode(data[j + 1], &b2)) return 0;

        data[i] = b1 | (b2 << 4);
    }
    return length;
}

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

void txNextBit(void);

// the data that is going to be sent
uint8_t txBuffer[MAX_PACKET_SIZE];
uint16_t txBufferSize;
uint16_t numTxSuccessiveZeroBits;

void prepareToTx(uint8_t *data, uint16_t dataSize)
{
    uint16_t i, j;
    switch (USED_ENCODING) {
    case ENCODING_HAMMING:
    case ENCODING_BOTH:
        if (dataSize > MAX_PACKET_SIZE / 2) dataSize = MAX_PACKET_SIZE / 2;
        for (i = 0, j = 0; i < dataSize; ++i, j += 2) {
            txBuffer[j] = hammingEncode(data[i] & 0xf);
            txBuffer[j + 1] = hammingEncode(data[i] >> 4);
        }
        txBufferSize = dataSize * 2;
        break;
    default:
        if (dataSize > MAX_PACKET_SIZE) dataSize = MAX_PACKET_SIZE;
        memcpy(txBuffer, data, dataSize);
        txBufferSize = dataSize;
        break;
    }
}

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

static inline void startTimer()
{
    TBCTL |= MC_UPTO_CCR0;
}

static inline void stopTimer()
{
    TBCTL &= ~MC_UPTO_CCR0;
}

#define startTxBit(bit)  TRM433_WRITE_DATA_FAST(bit);


static inline uint16_t txDataBit(uint16_t bitNum)
{
    const uint8_t bit = txBuffer[bitNum / 8] & (1 << (bitNum & 0x7));

    switch (USED_ENCODING) {
    case ENCODING_NONE:
    case ENCODING_HAMMING:
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
        break;

    case ENCODING_MANCHESTER:
    case ENCODING_BOTH: {
        // do Manchester encoding on the run
        static uint8_t b;
        startTxBit(b ^ bit);
        if (b) ++bitNum;
        b = !b;
    }
        break;
    }

    return bitNum;
}

enum TxState {
    STATE_IDLE,
    STATE_PREAMBLE,
    STATE_DELIM,
    STATE_DATA,
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
            state = STATE_PREAMBLE;
            // redLedOn();
            startTxBit(1);
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
            state = STATE_IDLE;
        }
        break;
    }
}

void sendData() {
    TRM433_TX_MODE();
    TRM433_CLEAR_DATA();

    PRINT("sendData...\n");

    initTimer();
    startTimer();
}

#else
// receiver

// TODO: should also account for preamble size!
uint8_t rxBuffer[MAX_SAMPLES_PER_BIT * MAX_PACKET_SIZE];

uint8_t rxDataBuffer[MAX_PACKET_SIZE];
uint16_t rxDataBitNum;
uint16_t rxDataZeroBits;
uint8_t rxBitFirstHalf;

static inline void rxDataReset()
{
    rxDataBitNum = 0;
    rxDataZeroBits = 0;
    rxBitFirstHalf = false;
}

// returns true on end of packet
static inline bool rxDataBit(uint8_t bit)
{
    if (USED_ENCODING == ENCODING_MANCHESTER
            || USED_ENCODING == ENCODING_BOTH) {
        rxBitFirstHalf = !rxBitFirstHalf;
        // ignore the second part of the bits
        if (!rxBitFirstHalf) {
            if (bit) rxDataZeroBits = 0;
            else ++rxDataZeroBits;
            return false;
        }
    }
    if (bit) {
        if (USED_ENCODING == ENCODING_NONE
                || USED_ENCODING == ENCODING_HAMMING) {
            if (rxDataZeroBits >= MAX_SUCCESSIVE_ZERO_BITS) {
                // ignore it
                rxDataZeroBits = 0;
                return false;
            }
        }
        rxDataZeroBits = 0;
        rxDataBuffer[rxDataBitNum / 8] |= (1 << (rxDataBitNum % 8));
    } else {
        const uint16_t endingZeroBits = 
                (USED_ENCODING == ENCODING_NONE
                        || USED_ENCODING == ENCODING_HAMMING) ?
                5 : 3;
        if (++rxDataZeroBits >= endingZeroBits) return true;
        rxDataBuffer[rxDataBitNum / 8] &= ~(1 << (rxDataBitNum % 8));

    }
    ++rxDataBitNum;
    return false;
}

static inline void rxDataProcess()
{
    // cut it down to byte boundary
    uint16_t length = rxDataBitNum / 8;

    PRINTF("a packet sucessfully received!\n");
    debugHexdump(rxDataBuffer, length);

    {
        rxDataBuffer[length] = 0;
        PRINTF("as string: %s\n", (char *)rxDataBuffer);
    }

    if (USED_ENCODING == ENCODING_HAMMING
            || USED_ENCODING == ENCODING_BOTH) {
        length = hammingDecodeInplace(rxDataBuffer, length);
        if (length) {
            PRINTF("packet after decoding: \n");
            debugHexdump(rxDataBuffer, length);
        } else {
            PRINTF("Hamming decoding failed\n");
        }
    }
}

static inline uint8_t countBits(uint8_t octet)
{
    return (octet & (1 << 0))
            + ((octet & (1 << 1)) >> 1)
            + ((octet & (1 << 2)) >> 2)
            + ((octet & (1 << 3)) >> 3)
            + ((octet & (1 << 4)) >> 4)
            + ((octet & (1 << 5)) >> 5)
            + ((octet & (1 << 6)) >> 6)
            + ((octet & (1 << 7)) >> 7);
}

uint16_t countAllBits()
{
    uint16_t i, result = 0;
    for (i = 0; i < sizeof(rxBuffer); ++i) {
        result += countBits(rxBuffer[i]);
    }
    return result;
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

// how many samples should be 1 to detect bit "1"?
const uint8_t BIT_SAMPLE_COUNT[MAX_SAMPLES_PER_BIT + 1] =
{
    0, // 0
    1, // 1
    2, // 2
    2, // 3
    3, // 4
    3, // 5
    4, // 6
    4, // 7
    5, // 8
    5, // 9
    6 // 10
};

enum { BYTE_ARRAY_SIZE = 32 };

// XXX: this could be optimized
static inline uint8_t detectBit(uint8_t *array, uint16_t arrayPos,
        uint16_t samplesPerBit)
{
    uint16_t numOnes = 0;
    uint16_t i;
    for (i = 0; i < samplesPerBit; ++i) {
        numOnes += array[(arrayPos + i) % BYTE_ARRAY_SIZE];
    }
    return numOnes >= BIT_SAMPLE_COUNT[samplesPerBit];
}

static inline bool detectDelimiterBit(uint8_t *array, uint16_t *arrayPos,
        uint16_t *runningBitLength, uint16_t delimiterBitNum) 
{
    const uint8_t expectedBit = delimiterBitNum & 0x1;

    uint16_t startIndex = *arrayPos;
    uint16_t startIndexM = (startIndex - 1) % BYTE_ARRAY_SIZE;
    uint16_t startIndexP = (startIndex + 1) % BYTE_ARRAY_SIZE;

    uint16_t endIndex = (startIndexM + *runningBitLength) % BYTE_ARRAY_SIZE;
    uint16_t endIndexM = (endIndex - 1) % BYTE_ARRAY_SIZE;
    uint16_t endIndexP = (endIndex + 1) % BYTE_ARRAY_SIZE;

    PRINT("DDB\n");

    // adjust frame position
    if (array[endIndex] != expectedBit
            && array[startIndexM] == expectedBit) {
        PRINT("DDlmB: move frame back\n");
        startIndex = startIndexM;
        endIndexP = endIndex;
        endIndex = endIndexM;
    }
    else if (array[startIndex] != expectedBit
            && array[endIndexP] == expectedBit) {
        PRINT("DDlmB: move frame forward\n");
        startIndex = startIndexP;
        endIndex = endIndexP;
        endIndexP = (endIndexP + 1) % BYTE_ARRAY_SIZE;
    }

    // adjust frame length (this has no effect for the last bit)
    if (array[endIndexP] == expectedBit) {
        if (*runningBitLength < MAX_SAMPLES_PER_BIT) {
            PRINT("DDlmB: make frame longer\n");
            (*runningBitLength)++;
        }
    }
    else if (array[endIndex] != expectedBit) {
        if (*runningBitLength > MIN_SAMPLES_PER_BIT) {
            PRINT("DDlmB: make frame shorter\n");
            (*runningBitLength)--;
        }
    }

    if (detectBit(array, startIndex, *runningBitLength) != expectedBit) {
        // serious trouble: delimiter sequence should be rx'ed without errors
        // XXX: this makes the sequence extra sensitive to errors;
        // a better approach mught be to make those bits in delim seq. larger,
        // but that would make processing more complex.
        PRINTF("detectDelimiterBit: bit %u is wrong!\n", delimiterBitNum);
        return false;
    }
    *arrayPos = startIndex;
    return true;

}

static inline uint8_t detectDataBit(uint8_t *array, uint16_t *arrayPos,
        uint16_t runningBitLength) 
{
    uint16_t startIndex = *arrayPos;
    uint8_t result = detectBit(array, startIndex, runningBitLength);

    uint16_t endIndex = (startIndex + runningBitLength - 1) % BYTE_ARRAY_SIZE;
    if (array[endIndex] != result) {
        startIndex = (startIndex - 1) % BYTE_ARRAY_SIZE;
        if (array[startIndex] == result) {
            // move it back
            *arrayPos = startIndex;
            PRINT("DDtB: move frame back\n");
        }
    } else {
        endIndex = (endIndex + 1) % BYTE_ARRAY_SIZE;
        if (array[endIndex] == result) {
            if (array[startIndex] != result) {
// FIXME: looks like a bug here
                *arrayPos = (startIndex + 1) % BYTE_ARRAY_SIZE;
                PRINT("DDtB: move frame forward\n");
            }
        }
    }

    return result;
}

#define ENOUGH_SPACE(readPos, writePos)                                 \
    ((writePos > readPos) ?                                             \
            (writePos >= readPos + MAX_SAMPLES_PER_BIT) :               \
            (writePos + BYTE_ARRAY_SIZE >= readPos + MAX_SAMPLES_PER_BIT))

//
// Take the packet recieved and convert it from samples to bits.
// End of packet is signaled by n zero bits 
// (count depends on whether Manch. Enc. is used)
// The precise ending is determined by the end of the last octet.
//
void processPacket()
{
    // at this moment the rx buffer contains delimiter sequence,
    // the data packet, and some zero samples in the end.
    PRINTF("%lu: a packet received\n", getRealTime());
    debugHexdump(rxBuffer, sizeof(rxBuffer));

    // start with default model parameters
    uint16_t runningBitLength = SAMPLES_PER_BIT;
    enum {
        STATE_PREAMBLE,
        STATE_DELIM,
        STATE_DATA,
    } state = STATE_PREAMBLE;
    uint16_t delimiterBit = 0;
    uint16_t delimiterStartPos = 0;

    // decode 4 octets we are currently processing in this array
    uint8_t byteArray[BYTE_ARRAY_SIZE];
    uint16_t writePos, readPos;

    uint8_t *pbuf = rxBuffer;
    const uint8_t *pend = pbuf + sizeof(rxBuffer);
    decodeOctet(*pbuf++, byteArray);
    decodeOctet(*pbuf++, byteArray + 8);
    decodeOctet(*pbuf++, byteArray + 16);
    decodeOctet(*pbuf++, byteArray + 24);

    // gap sequence must precede everything else
    if (detectBit(byteArray, 0, SAMPLES_PER_BIT) != 1
            || detectBit(byteArray, SAMPLES_PER_BIT, SAMPLES_PER_BIT) != 1) {
        PRINTF("the packet does not start with preamble!\n");
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
                if (byteArray[readPos] != 0
                        || detectBit(byteArray, readPos, SAMPLES_PER_BIT) != 0) {
                    // nothing good here yet.
                    break;
                }
                state = STATE_DELIM;
                // fallthrough
            case STATE_DELIM:
                if (!detectDelimiterBit(byteArray, &readPos,
                                &runningBitLength, delimiterBit)) {
                    // delimiter broken, cannot proceed
                    return;
                }
                ++delimiterBit;
                if (delimiterBit == 2) {
                    delimiterStartPos = readPos;
                }
                else if (delimiterBit == FRAME_DELIM_SIZE) {
                    uint16_t delimSeqLen;
                    delimSeqLen = ((uint16_t) (readPos - delimiterStartPos))
                            % BYTE_ARRAY_SIZE;
                    if (delimSeqLen < 4 * MIN_SAMPLES_PER_BIT) {
                        delimSeqLen += BYTE_ARRAY_SIZE;
                    }
                    delimSeqLen += 2; // to round it correctly
                    PRINTF("processPacket: final adjust bit size, t=%u, size=%u\n",
                            delimSeqLen, delimSeqLen / 4);
                    runningBitLength = delimSeqLen / 4;

                    PRINT("processPacket: process data\n");
                    state = STATE_DATA;
                    rxDataReset();
                }

                break;

            case STATE_DATA: {
                // TODO: limit data bit count?
                const uint8_t dataBit = detectDataBit(byteArray, &readPos,
                        runningBitLength);
                if (rxDataBit(dataBit)) {
                    rxDataProcess();
                    return;
                }
            }
                break;
            }

            readPos = (readPos + runningBitLength) % sizeof(byteArray);
        }
    }

    PRINT("processPacket: done.\n");
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
    uint16_t numOn = 0;
    if (rxOctet() == 0xff) ++numOn;
    if (rxOctet() == 0xff) ++numOn;
    if (rxOctet() == 0xff) ++numOn;
    if (rxOctet() == 0xff) ++numOn;
    if (rxOctet() == 0xff) ++numOn;
    if (rxOctet() == 0xff) ++numOn;
    if (rxOctet() == 0xff) ++numOn;
    if (rxOctet() == 0xff) ++numOn;
    if (rxOctet() == 0xff) ++numOn;
    if (rxOctet() == 0xff) ++numOn;
    return numOn >= 8;
}

void rxData()
{
    for (;;) {
        if (!detectPreamble()) continue;

        // -- read whole packet, as much as the buffer allows
        DISABLE_INTS(); // XXX: some time may get lost
        register uint16_t rxOctetNum = 0;
        for (; rxOctetNum < sizeof(rxBuffer); ++rxOctetNum) {
            rxBuffer[rxOctetNum] = rxOctet();
        }
        ENABLE_INTS();

        // -- now process the data in the buffer
        processPacket();

        // -- pause for a random time
        mdelay(1000 + (randomRand() % 512));
    }
}

void recvData()
{
    randomInit();

    TRM433_RX_MODE();
    TRM433_ON();

    mdelay(500);

    rxData();
}

#endif


//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    PRINT_INIT(127);

    TRM433_INIT();
#if SENDER
//    uint8_t data[] = {0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55};
//    uint8_t data[] = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};
    uint8_t data[] = {'h', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd'};
    prepareToTx(data, sizeof(data));
    sendData();
#else
    recvData();
#endif
}
