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

#ifndef MANSOS_NMEA_TYPES_H
#define MANSOS_NMEA_TYPES_H

#include <kernel/stdtypes.h>
#include <kernel/defines.h>

enum {
    // constant used to specify south latitude or west longitude (see examples)
    SOUTH_WEST = 0x8000
};

typedef struct {
    uint8_t deg; // degrees
    uint8_t min;  // minutes
    uint16_t mindec; // decimal parts of minutes, up to 4 digits.
                     // 15th bit used to specify whether it is N/E or S/W pos
} GPSPos_t;

#define POS_IS_SOUTH(md) (md & SOUTH_WEST)
#define POS_IS_WEST(md) (md & SOUTH_WEST)
#define POS_IS_NORTH(md) !POS_IS_SOUTH(md)
#define POS_IS_EAST(md) !POS_IS_WEST(md)

// examples:
// *) 56 58.6597'N => deg = 56, min = 58, mindec = 6597
// *) 56 58.6597'S => deg = 56, min = 58, mindec = 6597 + 0x8000 = 39365
// *) 24 11.7257'E => deg = 24, min = 11, mindec = 7257
// *) 24 11.7257'W => deg = 24, min = 11, mindec = 7257 + 0x800 = 40025

typedef struct {
    volatile uint8_t
    fix:2, // 3D/2D Fix
    dop:3,
    satcnt:3;
} PACKED GPSFixQuality_t;

// fix:
// 0 - no fix
// 1 = 2D fix
// 2 = 3D fix
// 3 - reserved
enum {
    FT_NO_FIX = 0,
    FT_2D_FIX,
    FT_3D_FIX
};

// dop describes horizontal dilution of position (DOP):
// (http://en.wikipedia.org/wiki/Dilution_of_precision_%28GPS%29)
// 0 = Ideal (DOP = 1)
// 1 = Excellent (DOP = [1..2))
// 2 = Good (DOP = [2..5))
// 3 = Moderate (DOP = [5..10))
// 4 = Fair (DOP = [10..20))
// 5 = Poor (DOP >= 20)
enum {
    DOP_IDEAL = 1,
    DOP_EXCELLENT,
    DOP_GOOD,
    DOP_MODERATE,
    DOP_FAIR,
    DOP_POOR
};

// satcnt stores satelite count:
// 0: 0
// 1: 1
// 2: 2
// 3: 3
// 4: 4
// 5: 5-6
// 6: 7-8
// 7: >8
enum {
    SAT_COUNT_0 = 0,
    SAT_COUNT_1 = 1,
    SAT_COUNT_2 = 2,
    SAT_COUNT_3 = 3,
    SAT_COUNT_4 = 4,
    SAT_COUNT_5_6 = 5,
    SAT_COUNT_7_8 = 6,
    SAT_COUNT_OVER_8 = 7
};

// date with offset 2010-01-01
typedef struct {
    volatile uint16_t
    day:5,   // day: 1-31
    mon:4,   // month: 1-12
    year:7;  // year: 2000-2127

} PACKED GPSDate_t;

typedef struct {
    // date of fix
    GPSDate_t d;
    // time of fix: h:m:s
    uint8_t h;
    uint8_t m;
    uint8_t s;
    // fix quality
    GPSFixQuality_t q;
    // latitude, longitude
    GPSPos_t lat;
    GPSPos_t lon;
} PACKED GPSFix_t;

#endif /* NMEA_TYPES_H_ */
