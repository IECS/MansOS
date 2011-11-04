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

/*
 * sense.h
 *
 *  Created on: May 24, 2009
 *      Author: girts
 */

#ifndef SENSE_H_
#define SENSE_H_

#include <stdint.h>

//-----------------
// constants
//-----------------
enum {
    RETRIES = 3, // how many values to read in one step, MUST be < 16

    READ_PERIOD_MS = 2 * 1000,
    SEND_PERIOD_MS = 6 * 1000,

//    READ_PERIOD_MS = 20 * 1000ul, // 20 seconds
//    SEND_PERIOD_MS = 60 * 1000ul, // 1 minute

    // pause between sending two consecutive packets
    // XXX: 100 ms IS TOO LOW!!!
    PACKET_PAUSE = 200,

    // pause between measuring two consecutive samples (all ADC 
    // channels red without pause between them)
    SAMPLE_PAUSE = 100,

    MAX_RECORDS = 10, // max records stored in buffer
    SAD_STAMP = 0x2357 // first bytes of message (identificator) 
};

//-------------------------------------------
// types
//-------------------------------------------

typedef struct {
    uint16_t light[RETRIES];
    uint16_t psaLight[RETRIES];
    uint16_t voltage;
} __attribute__((packed)) Measurement_t;

typedef struct {
    uint16_t id;
    uint16_t address;
    Measurement_t data;
    uint16_t crc;
} __attribute__((packed)) SadPacket_t ;

#ifdef PLATFOR_PC
typedef struct {
    time_t timestamp;
    uint16_t address;
    Measurement_t data;
} __attribute__((packed)) SadFilePacket_t ;
#endif

//-------------------------------------------
// functions
//-------------------------------------------

unsigned short crc16_data(const unsigned char *data, int datalen,
                          unsigned short acc);

#endif /* SENSE_H_ */
