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

#ifndef COMBOTEST_COMMON_H
#define COMBOTEST_COMMON_H

#include "mansos.h"
#include "leds.h"
#include "udelay.h"
#include "trm433.h"
#include "crc.h"
#include "hamming.h"
#include "timers.h"
#include "nmea_types.h"

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
#define ENCODING_MANCHESTER 1
#define ENCODING_HAMMING 0

//
// Length of single bit in usec;
// if taking one sample is ~20 CPU cycles long,
// then 40usec should allow to take sample 8 times per bit.
//
//#define BIT_LEN_USEC 40    // 25000 bps
#define BIT_LEN_USEC 100   // 10000 bps
//#define BIT_LEN_USEC 1000
//#define BIT_LEN_USEC 10000

enum {
    // sample count per bit
    SAMPLES_PER_BIT = 8,

    // sizes of packet parts, in bits
    PREAMBLE_SIZE = 10,
    FRAME_DELIM_SIZE = 6, // 010101
    EPILOGUE_SIZE = 2,
    PAUSE_SIZE = 1000,

    // decoded packet, in bytes
    MAX_PACKET_SIZE = 32 * (ENCODING_HAMMING ? 2 : 1),

    // received, undecoded data size, in bits
    MAX_FRAME_SIZE = MAX_PACKET_SIZE * 8
                     * (ENCODING_MANCHESTER ? 2 : 1)
                     + FRAME_DELIM_SIZE + PREAMBLE_SIZE + EPILOGUE_SIZE,

    // for bit stuffing
    MAX_SUCCESSIVE_ZERO_BITS = 4,
};

// -- data structures

#define PACKET_FORMAT_VERSION  0x1

// the data packet (the size currently is 22 bytes)
typedef struct Packet {
    uint8_t version;
    // originator's ID
    uint8_t originator;

    uint16_t testNr; // number of the test, incrementing
    // sensor data - to represent real packets. data is actually not used
    uint16_t light;
    uint16_t temperature;
    uint16_t humidity;
    // GPS data
    GPSPos_t gpsLat;
    GPSPos_t gpsLon;
    // time, when first sent
    uint32_t timestamp; // counter stored here
} Packet_t PACKED;

// this is sent via radio
typedef struct Frame {
    Packet_t data;
    // checksum
    uint16_t crc;
} Frame_t PACKED;

// typedef struct AccelGyroData {
//     uint16_t acc_x;
//     uint16_t acc_y;
//     uint16_t acc_z;
//     uint16_t gyro_x;
//     uint16_t gyro_y;
//     uint16_t __padding;
// } AccelGyroData_t PACKED;

#endif // COMBOTEST_COMMON_H
