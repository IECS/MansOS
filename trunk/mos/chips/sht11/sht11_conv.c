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

#include <kernel/stdtypes.h>

//-------------------------------------------
// SHT11 Humidity sensor conversion formulas
//-------------------------------------------

// RH = C1 + C2 * MH + C3 * MH * MH
// RH - relative humidity
// MH - measured humidity value

// Humidity conversion following the datasheet:
// RW = -2.0468 + 0.0367 * MH - 0.0000015955 * MH * MH

// We convert it to:
//       -20468 + 367 * MH - 0.015955 * MH * MH
// RH =  --------------------------------------
//                       10000


//                           15955 * MH * MH
//       -20468 + 367 * MH - ---------------
//                               1000000
// RH =  --------------------------------------
//                       10000

// Temperature conversion following the datasheet:
// TD = D1 + D2 * MT
// TD  -Temperature in degrees
// MT - measured temperature value

// TD = -39.4 + 0.04 * MT

// We convert it to:
//       -3940 + 4 * MT
// TD =  --------------
//            100


#define SHT11_VER 3

// constants for conversion
#if SHT11_VER == 3

// Old revision 3.0 of the sensor
// values copied from Contiki
#define SHT11_C1 -40000ll
#define SHT11_C2 405ll
#define SHT11_C3 -28000ll
// temperature @ 2.5V VCC
#define SHT11_D1 -3960ll
#define SHT11_D2 1ll

#else

// constants from the datasheet, version 4.3, May 2010
#define SHT11_C1 -20468ll
#define SHT11_C2 367ll
#define SHT11_C3 -15955ll
// temperature @ 2.5V VCC
#define SHT11_D1 -3940ll
#define SHT11_D2 4ll

#endif


uint16_t sht11_raw2rel_hum(const uint16_t raw) {
    int32_t res = SHT11_C3;
    int32_t temp = raw;
    temp *= temp; // temp = MH * MH
    // to fit in 32 bits
    temp /= 1000; // temp = MH * MH / 1000
    res *= temp; // res = C3 * MH * MH / 1000
    res = SHT11_C1 + res / 1000ll; // res = C1 + (C3 * MH * MH) / 1000000
    temp = raw;
    temp *= SHT11_C2; // temp = C2 * MH
    res += temp;  // res = C1 + C2 * MH + (C3 * MH * MH) / 1000000
    res /= 10000ll;
    return (uint16_t) res;
}

uint16_t sht11_raw2deg_temp(const uint16_t raw) {
    int32_t res = raw;
    res *= SHT11_D2; // res = D2 * MT
    res += SHT11_D1; // res = D1 + D2 * MT
    res /= 100; // res = (D1 + D2 * MT) / 100
    return (uint16_t) res;
}
