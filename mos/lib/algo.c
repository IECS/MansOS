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

#include "algo.h"

// Calculate square root, rounded down, exception:
// intSqrt(8) = 3
// Accuracy +-1.
// Highest possible inaccuracy when:
// intSqrt(x^2 - 1) = sqrt(x - 1)
uint16_t intSqrt(uint32_t val)
{
    // Rough estimation of result
    // can be calculated using formulas:
    // d = digit count
    // x0 = 2 * 10^n and n = (d - 1)/2, if d is odd
    // x0 = 6 * 10^n and n = (d - 2)/2, if d is even
    // but since possible values ain't too much, we can hard code them
    uint32_t prev = (val < 10 ? 2 :
        (val < 100 ? 6 :
        (val < 1000 ? 20 :
        (val < 10000 ? 60 :
        (val < 100000 ? 200 :
        (val < 1000000 ? 600 :
        (val < 10000000 ? 2000 :
        (val < 100000000 ? 6000 :
        (val < 1000000000 ? 20000 :
       60000)))))))));
    uint32_t cur = prev, next;
#ifdef DEBUG
    PRINTF("Starting sqrt(%lu) = %lu\n", val, cur);
    uint8_t iter = 1;
#endif // DEBUG
    // Babylonian method for square root calculation
    while(cur)
    {
        // Calculating next estimation
        next = (cur + val / cur) / 2;
        // Stop when next value equals current value.
        // But since we are using integers there might be situation, when next
        // and current value differs by 1 and they switch values each iteration
        // and never are equal, so we need to check equality with last two values
        // and take current value not next, because it yields correct result.
        if (next == cur || next == prev)
        {
#ifdef DEBUG
            PRINTF("    With %d iterations found sqrt(%lu) = %lu\n", iter, val, cur);
#endif // DEBUG
            return cur;
        }

        prev = cur;
        cur = next;
#ifdef DEBUG
        iter++;
        PRINTF("    Itering sqrt(%lu) = %lu\n", val, cur);
#endif // DEBUG
    }
    return 0;
}
