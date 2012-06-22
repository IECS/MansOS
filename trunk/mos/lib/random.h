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

#ifndef MANSOS_RANDOM_H
#define MANSOS_RANDOM_H

#include <kernel/stdtypes.h>

//===========================================================
// Data types and constants
//===========================================================

#undef RANDOM_MAX
#define RANDOM_MAX 65535u

//===========================================================
// Procedures
//===========================================================

//
// Initialize the random number with some random or unique bits from hardware
//
void randomInit(void);

//
// Seed the random number generator with a specific integer
//
void randomSeed(uint16_t seed);

//
// Get a random integer between 0 and RANDOM_MAX (including)
//
uint16_t randomNumber(void);

//
// Get a random integer between 0 and limit
//
static uint16_t randomNumberBounded(uint16_t limit)
{
    if (limit == 0) return 0;
    return randomNumber() % limit;
}

//
// Get a random integer in a specific range
//
static uint16_t randomInRange(uint16_t low, uint16_t high)
{
    if (low >= high) return low;
    return randomNumber() % (high - low) + low;
}

#endif
