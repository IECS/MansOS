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

#include <mansos.h>
#include <dprint.h>
#include <leds.h>
#include <udelay.h>
#include <string.h>
#include <hil/timers.h>
#include "main.h"
#include "assert.h"

// operation modes (of this program)
#define MODE_INIT  0
#define MODE_WRITE 1
#define MODE_READ  2
#define MODE_READ_WRITE  3

// which mode to use?
//#define PROGRAM_MODE MODE_INIT
//#define PROGRAM_MODE MODE_WRITE
#define PROGRAM_MODE MODE_READ_WRITE

#define DATA_MAGIC_NUMBER  0xbaadf00dul

//#define NUM_RECORDS   1
#define NUM_RECORDS   5

uint16_t localNetworkAddress = 0x1234;

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
    uint32_t timestamp; // time when this recond was made
    uint8_t data[];     // the data (variable size)
} Datablock_t PACKED;

// this is sent via radio (timestamp is not included, though it's also sent)
typedef struct Packet {
    // originator's ID
    uint16_t originator;
    // sensor data
    uint16_t light;
    uint16_t temperature;
    uint16_t activity;
    // GPS data
    uint32_t gpsLat;
    uint32_t gpsLon;
} Packet_t PACKED;

typedef struct AccelGyroData {
    uint16_t acc_x;
    uint16_t acc_y;
    uint16_t acc_z;
    uint16_t gyro_x;
    uint16_t gyro_y;
    uint16_t __padding;
} AccelGyroData_t PACKED;


bool writeData() {
    uint8_t buffer[100];

    Datablock_t *db;
    Packet_t *pck;
    AccelGyroData_t *agd;
    uint8_t *s;

    uint16_t i;

    toggleGreenLed();

    for (i = 0; i < NUM_RECORDS; ++i) {
    // -- a packet
    db = (Datablock_t *) buffer;
    db->magic = DATA_MAGIC_NUMBER;
    db->dataType = DB_PACKET;
    db->__padding1 = 0;
    db->blockLen = sizeof(Datablock_t) + sizeof(Packet_t);
    db->timestamp = getRealTime();

    pck = (Packet_t *)db->data;
    pck->originator = localNetworkAddress;
    pck->light = 123;
    pck->temperature = 20;
    pck->activity = 13;
    pck->gpsLat = 111111;
    pck->gpsLon = 222222;

    if (!flashStreamWrite(buffer, db->blockLen)) return false;

    // -- accel and gyro data
    db = (Datablock_t *) buffer;
    db->magic = DATA_MAGIC_NUMBER;
    db->dataType = DB_ACCEL_GYRO;
    db->__padding1 = 0;
    db->blockLen = sizeof(Datablock_t) + sizeof(AccelGyroData_t);
    db->timestamp = getRealTime();

    agd = (AccelGyroData_t *)db->data;
    agd->acc_x = 1;
    agd->acc_y = 2;
    agd->acc_z = 3;
    agd->gyro_x = 45;
    agd->gyro_y = 90;
    agd->__padding = 0;

    if (!flashStreamWrite(buffer, db->blockLen)) return false;

    // -- just a debug string
#define TEST_STRING  "hello world"
    db = (Datablock_t *) buffer;
    db->magic = DATA_MAGIC_NUMBER;
    db->dataType = DB_STRING;
    db->__padding1 = 0;
    db->blockLen = sizeof(Datablock_t) + sizeof(TEST_STRING) - 1;
    db->timestamp = getRealTime();

    s = (uint8_t *)db->data;
    memcpy(s, TEST_STRING, sizeof(TEST_STRING) - 1);
    if (!flashStreamWrite(buffer, db->blockLen)) return false;

    }

    flashStreamFlush();

    PRINTF("all data written!\n");

    toggleGreenLed();
    return true;
}

bool verifyData() {
    static char buffer[100];
    uint16_t dataLen;

    Datablock_t db;
    Packet_t *pck;
    AccelGyroData_t *agd;

    toggleBlueLed();

    for (;;) {
        dataLen = sizeof(Datablock_t);
        if (!flashStreamRead(buffer, &dataLen)) return false;

        if (dataLen < sizeof(Datablock_t)) return false;
        memcpy(&db, buffer, sizeof(Datablock_t));

        if (db.magic != DATA_MAGIC_NUMBER) {
            PRINTF("got something, but the magic is bad\n");
            // bad record, try restart reading from a new block
            flashStreamSeekToNewBlock();
        }

        PRINTF("got data block with type %d, len %u\n", db.dataType, db.blockLen);

        dataLen = db.blockLen - sizeof(Datablock_t);
        if (!flashStreamRead(buffer, &dataLen)) return false;
        if (dataLen < db.blockLen - sizeof(Datablock_t)) return false;
 
        switch (db.dataType) {
        case DB_PACKET:
            pck = (Packet_t *) buffer;
            PRINTF("got packet: originator=%u, light=%u, t=%u, act=%u, gps=(%lu %lu)\n",
                    pck->originator,
                    pck->light,
                    pck->temperature,
                    pck->activity,
                    pck->gpsLat,
                    pck->gpsLon);
            break;
        case DB_ACCEL_GYRO:
            agd = (AccelGyroData_t *) buffer;
            PRINTF("got agd: (%u, %u, %u) (%u %u)\n",
                    agd->acc_x,
                    agd->acc_y,
                    agd->acc_z,
                    agd->gyro_x,
                    agd->gyro_y);
            break;
        case DB_STRING:
            if (dataLen >= sizeof(buffer)) dataLen = sizeof(buffer) - 1;
            buffer[dataLen] = '\0';
            PRINTF("got string: \"%s\"\n", buffer);
            break;
        default:
            PRINTF("got unsupported type %u\n", db.dataType);
            break;
        }

        blueLedOff();
    }
    return true;
}

void appMain(void)
{
    PRINT_INIT(128);
    busyWait(500000);

    PRINTF("start working with flash\n");

    extFlashWake();

    redLedOn();

#if PROGRAM_MODE == MODE_INIT
    flashStreamClear();
    flashStreamVerifyChecksums();
#endif

#if (PROGRAM_MODE & MODE_WRITE)
    flashStreamClear();
    writeData();
    flashStreamVerifyChecksums();
#endif

#if (PROGRAM_MODE & MODE_READ)
    flashStreamVerifyChecksums();
    verifyData();
#endif

    redLedOff();

    for (;;) {
        busyWait(200000);
        PRINTF(".\n");
        redLedOn();
        busyWait(10000);
        redLedOff();
    }
}
