/*
 * Copyright (c) 2013 the MansOS team. All rights reserved.
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

///
/// ISL1219 Real-Time Clock chip
///

#ifndef MANSOS_ISL1219_H
#define MANSOS_ISL1219_H

#include <defines.h>

// I2C address (1101111x)
#define ISL1219_ADDRESS    0x6F

//
// ISL1219 register section addresses
//
#define ISL1219_REGISTER_RTC_SECTION       0x0  // from 0 to 6
#define ISL1219_REGISTER_STATUS_SECTION    0x7  // from 7 to 11
#define ISL1219_REGISTER_ALARM_SECTION     0xC  // from 12 to 17
#define ISL1219_REGISTER_USER_SECTION      0x12 // from 18 to 19
#define ISL1219_REGISTER_TIMESTAMP_SECTION 0x14 // from 20 to 25

// write-enable flag on statu register
#define ISL1219_WRITE_FLAG                 0x10


// Initialize the ISL1219 RTC chip
void isl1219Init(void);

struct ISL1219_Clock_s {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t date;
    uint8_t month;
    uint8_t year;
} PACKED; 

typedef struct ISL1219_Clock_s ISL1219_Clock_t;

bool isl1219Write(const ISL1219_Clock_t *clock);

bool isl1219Read(ISL1219_Clock_t *result);

#endif
