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

#include <assert.h>
#include "nmea.h"

int main() {
    const char buf1[] = "063645.000,5658.6597,N,02411.7264,E,1,3,1.00,125.5,M,23.5,M,,";
    const char buf2[] = "235959.187,5658.6597,S,02411.7264,W,1,5,1.40,125.5,M,23.5,M,,";
    const char buf3[] = "0.0,5658.6597,N,02411.7264,E,1,12,1.04,125.5,M,23.5,M,,";
    const char buf4[] = "0.0,5658.6597,N,02411.7264,E,1,12,2.00,125.5,M,23.5,M,,";
    const char buf5[] = "0.0,5658.6597,N,02411.7264,E,1,12,2.5,125.5,M,23.5,M,,";
    const char buf6[] = "0.0,5658.6597,N,02411.7264,E,1,12,5.04,125.5,M,23.5,M,,";
    const char buf7[] = "0.0,5658.6597,N,02411.7264,E,1,12,13.42,125.5,M,23.5,M,,";
    const char buf8[] = "0.0,5658.6597,N,02411.7264,E,1,12,138.42,125.5,M,23.5,M,,";

    const char buf8[] = "A,1,,,,,,,,,,,,,,,";


    GPSFix_t fix;
    parseGGA(buf1, sizeof(buf1) - 1, &fix);
    assert(fix.h == 6);
    assert(fix.m == 36);
    assert(fix.s == 45);
    assert(fix.lat.deg == 56);
    assert(fix.lat.min == 58);
    assert(fix.lat.mindec == 6597);
    assert(fix.lon.deg == 24);
    assert(fix.lon.min == 11);
    assert(fix.lon.mindec == 7264);
    assert(fix.q.satcnt == 3);
    assert(fix.q.dop == DOP_IDEAL);

    parseGGA(buf2, sizeof(buf2) - 1, &fix);
    assert(fix.h == 23);
    assert(fix.m == 59);
    assert(fix.s == 59);
    assert(fix.lat.deg == 56);
    assert(fix.lat.min == 58);
    assert(fix.lat.mindec == 6597 + 0x8000);
    assert(fix.lon.deg == 24);
    assert(fix.lon.min == 11);
    assert(fix.lon.mindec == 7264 + 0x8000);
    assert(fix.q.satcnt == 5);
    assert(fix.q.dop == DOP_EXCELLENT);

    parseGGA(buf3, sizeof(buf3) - 1, &fix);
    assert(fix.h == 0);
    assert(fix.m == 0);
    assert(fix.s == 0);
    assert(fix.q.satcnt == 12);
    assert(fix.q.dop == DOP_EXCELLENT);

    parseGGA(buf4, sizeof(buf4) - 1, &fix);
    assert(fix.q.dop == DOP_GOOD);

    parseGGA(buf5, sizeof(buf5) - 1, &fix);
    assert(fix.q.dop == DOP_GOOD);

    parseGGA(buf6, sizeof(buf6) - 1, &fix);
    assert(fix.q.dop == DOP_MODERATE);

    parseGGA(buf7, sizeof(buf7) - 1, &fix);
    assert(fix.q.dop == DOP_FAIR);

    parseGGA(buf8, sizeof(buf8) - 1, &fix);
    assert(fix.q.dop == DOP_POOR);

    return 0;
}
