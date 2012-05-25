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

#include "average.h"

// Initialize Average_t
Average_t avgInit(uint8_t window) {
    Average_t result;
    uint8_t temp;
    result.sum = result.count = result.bufSum = result.bufCount = 0;
    result.window = window;
    result.haveCoefficients = false;
    if (window) {
        result.history = malloc(sizeof(uint16_t) * window);
        // Why implicit declaration?
        //memset(result.history, 0, sizeof(uint16_t) * window);
        for (temp = 0; temp < window; temp++) {
            result.history[temp] = 0;
        }
        result.oldestValue = 0;
    }
    return (result);
};

// Initialize Average_t with coefficients, window = len(coefs)
Average_t avgInitWithCoeffs(uint8_t window, uint8_t *coefs){
    Average_t result = avgInit(window);
    uint8_t temp;
    if (window) {
        result.coefficients = (uint8_t*)malloc(sizeof(uint8_t) * window);
        // Why implicit declaration?
        //memcpy(result.coefficients, coefs, sizeof(uint8_t) * window);
        for (temp = 0; temp < window; temp++){
               result.coefficients[temp] = coefs[temp];
               result.count += coefs[temp];
           }
        result.haveCoefficients = true;
    }
    return (result);
};

void addAverage(Average_t *avg, uint16_t *val) {
    // Continuous average
    if (avg->window == 0) {
        // Check for overflowing
        if (avg->sum >= BUFFERING_START_TRESHOLD && avg->bufSum == 0) {
            avg->bufSum = avg->sum;
            avg->bufCount = avg->count;
#ifdef DEBUG
            PRINTF("Starting average buffer\n");
#endif //DEBUG
        }
        else if (avg->sum >= BUFFERING_STOP_TRESHOLD) {
            avg->sum -= avg->bufSum;
            avg->count -= avg->bufCount;
            avg->bufSum = avg->bufCount = 0;
#ifdef DEBUG
            PRINTF("Flushing from average buffer\n");
#endif //DEBUG
        }
        // Add next value
        avg->sum += *val;
        avg->count++;
    } else {
        uint16_t temp = avg->history[avg->oldestValue];
        // Save new value and increase counter
        avg->history[avg->oldestValue++] = *val;
        if (avg->oldestValue == avg->window) {
            avg->oldestValue = 0;
        }
        // Moving average with coefficients
        if (avg->haveCoefficients) {
            avg->sum = 0;
            // history[oldest]*coeff[0] + history[oldest + 1]*coeff[1] + ...
            for (temp = 0; temp < avg->window; temp++) {
                avg->sum += (uint32_t)avg->history[(avg->oldestValue + temp)
                                 % avg->window] * avg->coefficients[temp];
            }
        // Moving average
		} else {
            // Change sum, cant use *val - temp, because it yields incorrect
            // result if temp > *val
            avg->sum += *val;
            avg->sum -= temp;
            // count = max(count, oldestVal + 1)
            avg->count += (uint8_t)(avg->count < avg->window);
        }
    }
};

uint16_t getAverageValue(Average_t *avg) {
    // If getter() is used we can calculate this only on demand
    return (avg->value = avg->sum / avg->count);
};
