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

#ifndef MANSOS_AVERAGE_H
#define MANSOS_AVERAGE_H

#include <kernel/defines.h>

// For 2^32
#define BUFFERING_START_TRESHOLD 2147483648U // 2^31
#define BUFFERING_STOP_TRESHOLD 4294901760U // 2^32 - 2^16

struct Average_s {
    uint16_t value;
    uint32_t sum;
    uint32_t count;
    uint32_t bufSum;
    uint32_t bufCount;
    uint8_t window;
    uint16_t *history;
    uint8_t *coefficients;
    uint8_t oldestValue;
    bool haveCoefficients;
};

typedef struct Average_s Average_t;

Average_t avgInit(uint8_t);

Average_t avgInitWithCoeffs(uint8_t, uint8_t *);

void addAverage(Average_t*, uint16_t*);

uint16_t getAverageValue(Average_t *avg);

#endif
