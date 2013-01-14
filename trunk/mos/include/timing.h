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

#ifndef MANSOS_TIMING_H
#define MANSOS_TIMING_H

#include <platform.h>

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
// Get milliseconds elapsed since system start
//
static inline uint32_t getTimeMs(void) {
    return (uint32_t) jiffies2ms(jiffies);
}

static inline uint64_t getTimeMs64(void) {
    return jiffies2ms(jiffies);
}


//
// Get seconds elapsed since system start
// 32-bit value is OK: ~136 year system lifetime sounds long enough ;)
//
static inline uint32_t getTimeSec(void) {
    return jiffies2ms(jiffies) / 1000;
}

//
// Internal use: convert from milliseconds to sleep clock ticks
//
static inline uint16_t msToSleepCycles(uint16_t ms)
{
    return ms * SLEEP_CYCLES
        + (uint16_t) ((uint32_t) ms * (uint32_t) SLEEP_CYCLES_DEC / 1000ul);
}

//
// Internal use: convert from sleep clock ticks to milliseconds
//
static inline uint16_t sleepCyclesToMs(uint16_t ocr)
{
    return ocr * 1000ul / (SLEEP_CLOCK_SPEED / SLEEP_CLOCK_DIVIDER);
}


//
// Internal use only: this functions does the actual sleeping
//
void doMsleep(uint16_t milliseconds);


//
// Synchronized time functions: return time that is synchronized
// (fixed) root clock from the time synchronization protocol.
//
#if !USE_ROLE_BASE_STATION && USE_NET

extern int64_t rootClockDeltaMs;
#define getSyncTimeMs()   ((uint32_t)(getTimeMs() + rootClockDeltaMs))
#define getSyncTimeMs64() (getTimeMs() + rootClockDeltaMs)
#define getSyncTimeSec()  ((uint32_t)(getTimeSec() + rootClockDeltaMs / 1000))

#else

#define getSyncTimeMs()  getTimeMs()
#define getSyncTimeSec() getTimeSec()

#endif


#endif
