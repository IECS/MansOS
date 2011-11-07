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

#include <platform.h>
#include "../timing.h"

static inline uint16_t msToSleepCycles(uint16_t ms) {
    return ms * SLEEP_CYCLES
        + (uint16_t) ((uint32_t) ms * (uint32_t) SLEEP_CYCLES_DEC / 1000ull);
}

void atmegaTimer1Set(uint16_t ms)
{
    if (ms > PLATFORM_MAX_SLEEP_MS) {
        ms = PLATFORM_MAX_SLEEP_MS;
    } else if (ms < PLATFORM_MIN_SLEEP_MS) {
        ms = PLATFORM_MIN_SLEEP_MS;
    }

    uint16_t ocr = msToSleepCycles(ms);
    jiffies += ms2jiffies(ms);
    OCR1A = TCNT1 + ocr;
}
