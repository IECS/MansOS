/**
 * Copyright (c) 2008-2012 the MansOS team. All rights reserved.
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

#include "nmea.h"

// internal functions
static uint_t parseGPSTime(const uint8_t **buf, uint_t *len, GPSFix_t *fix);
static uint_t parseGPSDate(const uint8_t **buf, uint_t *len, GPSFix_t *fix);
static uint_t parseGPSu32(const uint8_t **buf, uint_t *len, uint32_t *u32,
        uint16_t *dec);
static uint8_t parseGPSGetChar(const uint8_t **buf, uint_t *len);
static uint_t parseGPSPos(const uint8_t **buf, uint_t *len, GPSPos_t *pos);
static uint8_t parseGPSChar(const uint8_t **buf, uint_t *len);
static uint_t parseGPSSatCnt(const uint8_t **buf, uint_t *len, GPSFix_t *fix);
static uint_t parseGPSDOP(const uint8_t **buf, uint_t *len, GPSFix_t *fix);
static uint_t parseGPSFixQual(const uint8_t **buf, uint_t *len, GPSFix_t *fix);

// to avoiod multiplication use
#define mul10(x) ((x << 3) + (x << 1))

//-----------------------------
// public functions
//-----------------------------

// parse GPGGA command starting with timestamp (right after GPGGA,)
// example of cmd in the buffer:
// 063645.000,5658.6597,N,02411.7264,E,1,3,1.40,125.5,M,23.5,M,,
uint_t parseGGA(const void *buf, uint_t len, GPSFix_t *fix)
{
    const uint8_t *p = (uint8_t *) buf;
    uint_t res;
    if ((res = parseGPSTime(&p, &len, fix))) return res;
    if ((res = parseGPSPos(&p, &len, &fix->lat))) return res;
    if ((res = parseGPSPos(&p, &len, &fix->lon))) return res;
    if (parseGPSChar(&p, &len) == 0) return -1; // discard fix type
    if ((res = parseGPSSatCnt(&p, &len, fix))) return res;
    if ((res = parseGPSDOP(&p, &len, fix))) return res;
    return 0;
}

// parse GPGSA command starting with auto-selection field (right after GPGSA,)
// only Fix type (2D/3D) is checked!
// example of cmd in the buffer:
// A,3,14,20,17,31,19,,,,,,,,3.37,1.20,3.15
uint_t parseGSA(const void *buf, uint_t len, GPSFix_t *fix)
{
    const uint8_t *p = (uint8_t *) buf;
    uint_t res;
    if (parseGPSChar(&p, &len) == 0) return -1; // discard auto-selection
    if ((res = parseGPSFixQual(&p, &len, fix))) return res;
    return 0;
}

// parse GPRMC command starting with time stamp (right after GPRMC,)
// example of cmd in the buffer:
// 055810.68,A,5623.9911,N,02415.2237,E,46.8,179.0,170210,07,E
// return 0 on success, error otherwise
// RMC contains:
// * time
// * date
// * latitude, longitude
uint_t parseRMC(const void *buf, uint_t len, GPSFix_t *fix) {
    const uint8_t *p = (uint8_t *) buf;
    uint_t res;
    if ((res = parseGPSTime(&p, &len, fix))) return res;

    // status != active => fix not taken
    if (parseGPSChar(&p, &len) != 'A') return -1;

    if ((res = parseGPSPos(&p, &len, &fix->lat))) return res;
    if ((res = parseGPSPos(&p, &len, &fix->lon))) return res;

    // skip speed and angle
    uint32_t dummy;
    if ((res = parseGPSu32(&p, &len, &dummy, 0))) return res;
    if ((res = parseGPSu32(&p, &len, &dummy, 0))) return res;

    if ((res = parseGPSDate(&p, &len, fix))) return res;

    return 0;
}

//-----------------------------
// internal functions
//-----------------------------

// time in format hhmmss.ddd, where ddd is second decimals (discarded)
// example: 063645.000
// example: 085051.418
uint_t parseGPSTime(const uint8_t **buf, uint_t *len, GPSFix_t *fix)
{
    // time and decimal part
    uint32_t t;
    uint16_t dec; // not used

    uint_t res = parseGPSu32(buf, len, &t, &dec);
    if (res) return res;
    fix->s = t % 100;
    t /= 100;
    fix->m = t % 100;
    fix->h = t / 100;
    return 0;
}

// date in format ddmmyy, convert it to days since 2010-jan-01
// example: 160710 : July 16th, 2010
uint_t parseGPSDate(const uint8_t **buf, uint_t *len, GPSFix_t *fix)
{
    uint32_t d;

    uint_t res = parseGPSu32(buf, len, &d, 0);
    if (res) return res;

    fix->d.year = d % 100;
    d /= 100;
    fix->d.mon = d % 100;
    fix->d.day = d / 100;

    return 0;
}

// position in format ddmm.xxxx,D, where
// dd = degrees,
// mm - minutes,
// xxxx - decimal part of minutes
// D- direction (N/S/E/W)
// example: 5658.6597,N
uint_t parseGPSPos(const uint8_t **buf, uint_t *len, GPSPos_t *pos)
{
    uint32_t p;
    uint_t res = parseGPSu32(buf, len, &p, &pos->mindec);
    if (res) return res;
    if (p != -1u) {
        pos->min = p % 100;
        pos->deg = p / 100;
        // parse degree direction: N/S E/W
        uint8_t ch = parseGPSChar(buf, len);
        if (ch == 'S' || ch == 'W') pos->mindec |= SOUTH_WEST;
    }
    return 0;
}

uint_t parseGPSSatCnt(const uint8_t **buf, uint_t *len, GPSFix_t *fix)
{
    uint32_t satcnt;
    parseGPSu32(buf, len, &satcnt, 0);
    if (satcnt != -1u) {
        if (satcnt > 8) {
            fix->q.satcnt = SAT_COUNT_OVER_8;
        } else if (satcnt > 6) {
            fix->q.satcnt = SAT_COUNT_7_8;
        } else if (satcnt > 4) {
            fix->q.satcnt = SAT_COUNT_5_6;
        } else {
            fix->q.satcnt = satcnt;
        }
        return 0;
    } else {
        return -1;
    }
}

uint_t parseGPSFixQual(const uint8_t **buf, uint_t *len, GPSFix_t *fix)
{
    uint32_t fixq;
    parseGPSu32(buf, len, &fixq, 0);
    if (fixq != -1u) {
        if (fixq == 3) {
            fix->q.fix = FT_3D_FIX;
        } else if (fixq == 2) {
            fix->q.fix = FT_2D_FIX;
        } else {
            fix->q.fix = FT_NO_FIX;
        }
        return 0;
    } else {
        return -1;
    }
}


uint_t parseGPSDOP(const uint8_t **buf, uint_t *len, GPSFix_t *fix)
{
    uint32_t dop;
    uint16_t dopdec;
    parseGPSu32(buf, len, &dop, &dopdec);
    if (dop != -1u) {
        if (dop == 1) {
            if (dopdec == 0) {
                // 1.0
                fix->q.dop = DOP_IDEAL;
            } else {
                // 1.x
                fix->q.dop = DOP_EXCELLENT;
            }
        } else if (dop == 0) {
            // no fix
            fix->q.dop = DOP_POOR;
            fix->q.fix = FT_NO_FIX;
        } else if (dop < 5) {
            // 2..5
            fix->q.dop = DOP_GOOD;
        } else if (dop < 10) {
            // 5..10
            fix->q.dop = DOP_MODERATE;
        } else if (dop < 20) {
            // 10..20
            fix->q.dop = DOP_FAIR;
        } else {
            // >= 20
            fix->q.dop = DOP_POOR;
        }
        return 0;
    } else {
        fix->q.dop = -1;
        return -1;
    }
}

// parse 32-bit uint and its decimal part after colon
// stop parsing at comma
uint_t parseGPSu32(const uint8_t **buf, uint_t *len, uint32_t *u32, uint16_t *dec)
{
    uint_t mainPart = 1; // parsing main part
    *u32 = 0;
    if (dec) *dec = 0;
    uint8_t ch = 0;
    while ((ch = parseGPSGetChar(buf, len)) != 0) {
        if (ch >= '0' && ch <= '9') {
            if (mainPart) {
                // add digit to u32
                *u32 = mul10(*u32) + (ch - '0');
            } else {
                // add digit to decimal part
                if (dec) *dec = mul10(*dec) + (ch - '0');
            }
        } else if (ch == '.') {
            if (mainPart) {
                // starting to read decimal part
                mainPart = 0;
            } else {
                // error
                *u32 = -1u;
                return -1;
            }
        } else if (ch == ',') {
            // done
            break;
        }
    }
    return 0;
}

// parse char and discard comma after it
uint8_t parseGPSChar(const uint8_t **buf, uint_t *len)
{
    uint8_t ch = parseGPSGetChar(buf, len); // read char
    parseGPSGetChar(buf, len); // discard comma
    return ch;
}

// get character from the buffer, increment buf pointer, decrement len
// when len == 0, return 0
uint8_t parseGPSGetChar(const uint8_t **buf, uint_t *len)
{
    if (*len == 0) return 0;
    --(*len);
    const uint8_t ch = **buf;
    ++(*buf);
    return ch;
}
