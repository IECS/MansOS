/*
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

#include "stdmansos.h"
#include "average.h"
#include "random.h"
#include "stdev.h"
#include "algo.h"

void appMain(void) {
    uint8_t coeffs[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // Init with coefficients
    Average_t avg1 = avgInitWithCoeffs(10, coeffs);
    // Init with window 5
    Average_t avg2 = avgInit(10);
    // Init with infinite window
    Average_t avg3 = avgInit(0);

    while (true) {
        uint16_t temp = randomNumber();
        addAverage(&avg1, &temp);
        addAverage(&avg2, &temp);
        addAverage(&avg3, &temp);

        PRINTF("%u %u %u %u %u\n", temp,getAverageValue(&avg1),
                getAverageValue(&avg2), getAverageValue(&avg3),
                intSqrt(temp));
        // change the default LED status
        ledToggle();
        // wait for 1000 milliseconds
        mdelay(1000);
    }
}
