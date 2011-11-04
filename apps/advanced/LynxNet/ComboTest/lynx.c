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
#include <hil/sleep.h>
#include <hil/fstream.h>
#include <hil/extflash.h>
#include <lib/random.h>
#include <string.h>
#include <lib/nmea/nmea.h>
#include <lib/nmea/nmea_stream.h>
#include <hil/humidity.h>

// whether to log data to flash
#define DO_FLASH_LOGGING 0

// whether to send data to radio
#define DO_RADIO_TX 1

// how many times to repeat radio sending
#define DO_RADIO_TX_REPEAT 1

// how long should be the period between sensor reading, in seconds
#define MAIN_LOOP_LENGTH 3

#define ORIGINATOR_ID 1 // the local identifier

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
            PRINTF("packet sent ok!\n");
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


// ------------------------------------
// flash writing stuff
// ------------------------------------

#define DATA_MAGIC_NUMBER  0xbaadf00dul

// all data types
enum {
    DB_STRING = 1, // just a string (not including zero character)
    DB_PACKET,     // a complete radio packet
    DB_ACCEL_GYRO, // only accel and gyro data
};

// data record superstructure
typedef struct Datablock {
    uint32_t magic;     // magic number, used to indentify start of datablock
    uint8_t dataType;   // one of the predefined data types
    uint8_t __padding1; // should be zero
    uint16_t blockLen;  // size of the block, including header
    // uint32_t timestamp; // time when this recond was made
    uint8_t data[];     // the data (variable size)
} Datablock_t PACKED;


bool flashWritePacket(Packet_t *p) {
    uint8_t buffer[100];
    Datablock_t *db;

    // write metainfo
    db = (Datablock_t *) buffer;
    db->magic = DATA_MAGIC_NUMBER;
    db->dataType = DB_PACKET;
    db->__padding1 = 0;
    db->blockLen = sizeof(Datablock_t) + sizeof(Packet_t);

    // write the packet
    memcpy(db->data, p, sizeof(*p));

    return flashStreamWrite(buffer, db->blockLen);
}

//-------------------------------------------
// GPS
//-------------------------------------------
static GPSFix_t fix;

enum {
    // get GPS fix every hour
    GPS_SAMPLE_PERIOD = 60ull * 60ull * 1000ull,
    // search for satellites for 60s
    GPS_RETRY_TIMEOUT = 60ull * 1000ull
};

#define CHECK_CMD(cmd) \
    if (nmeaBufState[NMEA_CMD_##cmd] == BS_READY) { \
        nmeaBufState[NMEA_CMD_##cmd] = BS_PROCESSING; \
        parseRMC(nmeaBuf[NMEA_CMD_##cmd] + 1, \
            MAX_NMEA_CMD_SIZE - 1, &fix); \
        nmeaBufState[NMEA_CMD_##cmd] = BS_EMPTY; \
        waitCmd++; \
    }

uint_t gpsFixOk(GPSFix_t *fix) {
    return fix->d.year >= 10 // year >= 2010
        && fix->d.mon > 0
        && fix->d.day > 0
        && fix->q.fix != FT_NO_FIX;
}

// try to get GPS fix for some period of time. Parse GGA, GSA & RMC commands
// return 1 on success, 0 otherwise
uint_t tryGPSFix(uint32_t tryEnd) {
    uint32_t thisTime = getRealTime();
    // turn on RX interrupt
    USARTEnableRX(0);

    // wait the whole sequence of commands one after another
    static uint_t waitCmd = NMEA_CMD_GGA;
    uint_t fixOk = 0;

    // invalidate fix;
    memset(&fix, 0, sizeof(fix));

    // check cmd buffers, wait for data to arrive
    while (thisTime < tryEnd) {
        switch (waitCmd) {
        case NMEA_CMD_GGA:
            CHECK_CMD(GGA);
            break;
        case NMEA_CMD_GSA:
            CHECK_CMD(GSA);
            break;
        case NMEA_CMD_RMC:
            CHECK_CMD(RMC);
            break;
        }

        thisTime = getRealTime();

        if (waitCmd == NMEA_CMD_COUNT) {
            // all commands parsed, check fix quality
            if (gpsFixOk(&fix)) {
                fixOk = 1;
                break;
            }
        }

        busyWait(10000); // play some rock
    }
    USARTDisableRX(0);

    return fixOk;
}

void sampleGPS() {
    // read GPS periodically
    static uint32_t lastGPSfix = 0;
    const uint32_t thisTime = getRealTime();

    if (thisTime - lastGPSfix >= GPS_SAMPLE_PERIOD) {
        if (tryGPSFix(thisTime + GPS_RETRY_TIMEOUT)) {
            // save time of last successful fix
            lastGPSfix = thisTime;
        }
    }
}


//-------------------------------------------
// Sensors
//-------------------------------------------
static uint16_t light;
static uint16_t temp;
static uint16_t humidity;

void readSensors() {
    light = adcRead(ADC_LIGHT);
    temp = readHTemperature(); // read temperature from humidity sensor
    humidity = readHumidity();
    // TODO - read activity
}


//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    PRINT_INIT(127);

    PRINTF("initializing\n");

    // initialize the external flash
    extFlashWake();

    // initialize TRM radio
    TRM433_INIT();
    // prepare TRM radio for sending
    TRM433_TX_MODE();
    TRM433_CLEAR_DATA();

    // initialize our special timer mode
    initTimer();

    // initialize RNG
    randomInit();

    // init UART0 for communication with GPS
    if (USARTInit(0, 115200, 0)) redLedOn();
    USARTSetReceiveHandle(0, nmeaCharRecv);


    PRINTF("init done\n");

    for (;;) {
        uint32_t endTime = getRealTime() + MAIN_LOOP_LENGTH * 1000;
        Packet_t pck;

        sampleGPS();
        readSensors();

        pck.version = PACKET_FORMAT_VERSION;
        pck.originator = ORIGINATOR_ID;
        pck.light = light;
        pck.temperature = temp;
        pck.humidity = humidity;
        pck.activity = 13; // TODO
        pck.gpsLat = fix.lat;
        pck.gpsLon = fix.lon;
        // alternative: use real timestamp or use sequence number
        // pck.timestamp = getRealTime();
        static uint32_t seqnum;
        pck.timestamp = seqnum++;

#if DO_FLASH_LOGGING
        // log the packet to external flash
        PRINTF("write to flash...\n");
        flashWritePacket(&pck);
#endif

#if DO_RADIO_TX
        // send to radio
        uint16_t i;
        for (i = 0; i < DO_RADIO_TX_REPEAT; ++i) {
            PRINTF("send to radio...\n");
            // delay some time, with some jitter
            mdelay(200 + randomRand() % 256);
            // place it in tx buffer
            radioTxPacket(&pck);
            // wait until transmission is completed
            while (packetToSend);
        }
#endif

        // sleep for some time
        // TODO: turn off the radio and really sleep
        PRINTF("sleep...\n");
        while (timeAfter(endTime, getRealTime()));
    }
}
