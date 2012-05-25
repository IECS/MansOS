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

#include "stdev.h"

// Initialize Stdev_t
Stdev_t stdevInit(uint8_t window) {
    Stdev_t result;
    // Disallowed, because can't hold all values in memory.
    if (window == 0) {
        result.average = avgInit(DEFAULT_SIZE);
    } else {
        result.average = avgInit(window);
    }
    return result;
};

void addStdev(Stdev_t *stdev, uint16_t *val) {
    addAverage(&stdev->average, val);
};

uint16_t getStdevValue(Stdev_t *stdev) {
    // If getter() is used we can calculate this only on demand
    uint16_t average = getAverageValue(&stdev->average);
    uint8_t temp;
    uint32_t sum = 0;
    for (temp = 0; temp < stdev->average.count; temp++) {
        int32_t dif;
        if (stdev->average.history[temp] > average) {
            dif = (int32_t)(stdev->average.history[temp] - average);
        } else {
            dif = (int32_t)(average - stdev->average.history[temp]);
        }
        // Dividing here helps against overflow if dif > 2^16, because
        // 2^16^2 = 2^32 - too large
        sum += (dif / stdev->average.count) * dif;
#ifdef DEBUG
        PRINTF("Adding %lu to stdev sum\n", dif * dif);
#endif //DEBUG
    }
    return (stdev->value = intSqrt(sum));
};
