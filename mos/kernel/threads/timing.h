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

#ifndef MANSOS_EXPTHREADS_TIMING_H
#define MANSOS_EXPTHREADS_TIMING_H

#include <kernel/defines.h>

extern volatile ticks_t jiffies;

static inline ticks_t getJiffies(void) INLINE;
static inline ticks_t getJiffies(void)
{
    return jiffies;
}

static inline ticks_t jiffies2ms(ticks_t jiffies) INLINE;
static inline ticks_t jiffies2ms(ticks_t jiffies)
{
    return jiffies;
}

static inline ticks_t ms2jiffies(ticks_t ms) INLINE;
static inline ticks_t ms2jiffies(ticks_t ms)
{
    return ms;
}

//
// For backwards compatibility
//
static inline ticks_t getRealTime(void) {
    return jiffies2ms(jiffies);
}

//
// Get seconds elapsed since system start
// 32-bit value is OK: ~136 year system lifetime sounds long enough ;)
//
static inline uint32_t getUptime(void) {
    return jiffies2ms(jiffies) / 1000;
}

#endif
