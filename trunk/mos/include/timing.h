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

#ifndef MANSOS_TIMING_H
#define MANSOS_TIMING_H

/// \file
/// Time accounting API
///
/// If USE_LONG_LIFETIME=n (the default), most of 64-bit functions
/// return the same value as their 32-but counterparts.
///

#include <platform.h>

///
/// MansOS "timer ticks"
///
/// This value is in platform-independent milliseconds.
/// It can be either 32-bit or 64-bit number, depending on configuration settings.
/// Specify USE_LONG_LIFETIME=y to use 64-bit jiffies (less efficient code!)
///
/// Note: in some contexts the value is not valid!
/// For example, in interrupt handler running while the system is in a low-power mode,
/// 'jiffies' will correspond to the time *before* the system went to sleep.
///
/// Note: the value is not extremely precise!
/// In long-term MansOS keeps jiffies syncronized with the high-accuracy HW clock crystal,
/// but relative error up to 48 milliseconds may be present.
/// If higher precision is needed (e.g. for TDMA protocol), use HW timer ticks directly!
///
extern volatile ticks_t jiffies;

///
/// Get ticks elapsed since system start.
///
/// The return value can be 32-bit or 64-bit number, depending on configuration settings.
/// Specify USE_LONG_LIFETIME=y to use 64-bit jiffies (less efficient code!)
///
static inline ticks_t getJiffies(void)
{
    return ATOMIC_READ(jiffies);
}

///
/// Get milliseconds elapsed since system start as 32-bit value
///
static inline uint32_t getTimeMs(void) {
    return (uint32_t) getJiffies();
}

///
/// Get milliseconds elapsed since system start as 64-bit value
///
/// Same as getTimeMs() unless USE_LONG_LIFETIME=y is used.
///
static inline uint64_t getTimeMs64(void) {
    return getJiffies();
}

///
/// Get seconds elapsed since system start
///
/// Only 32-bit version is provided,
/// because 0xffffffff seconds correspond to approximately 136 years.
///
static inline uint32_t getTimeSec(void) {
    return getJiffies() / 1000ul;
}

///
/// Convert alarm (jiffy) timer ticks to milliseconds
///
/// Note: on msp430 a conversion error is introduced due to rounding!
///
static inline uint16_t convertAlarmTimerToMs(uint16_t ticks)
{
    return ticks * 1000ul / (ACLK_SPEED / JIFFY_CLOCK_DIVIDER);
}

///
/// Convert milliseconds to alarm (jiffy) timer ticks
///
/// Note: on msp430 a conversion error is introduced due to rouding!
///
static inline uint16_t convertMsToAlarmTimer(uint16_t ms)
{
    return ms * ALARM_CYCLES
        + (uint16_t) ((uint32_t) ms * ALARM_CYCLES_DEC / 1000ul);
}

///
/// Convert sleep timer ticks to milliseconds
///
/// Note: on msp430 a conversion error is introduced due to rounding!
///
static inline uint16_t convertSleepTimerToMs(uint16_t ticks)
{
    return ticks * 1000ul / (SLEEP_CLOCK_SPEED / SLEEP_CLOCK_DIVIDER);
}

///
/// Convert milliseconds to sleep timer ticks
///
/// Note: on msp430 a conversion error is introduced due to rouding!
///
static inline uint16_t convertMsToSleepTimer(uint16_t ms)
{
    return ms * SLEEP_CYCLES
        + (uint16_t) ((uint32_t) ms * SLEEP_CYCLES_DEC / 1000ul);
}

//
// Synchronized time functions: return time that is synchronized
// with network root's clock using routing/timesync protocol.
//
#if !USE_ROLE_BASE_STATION && USE_NET

//! Time difference with network root node's clock in milliseconds
extern int64_t rootClockDeltaMs;
//! Get the network-wide synchronized time in milliseconds as 32-bit value
static inline uint32_t getSyncTimeMs(void) {
    return (uint32_t) (getTimeMs() + rootClockDeltaMs);
}
//! Get the network-wide synchronized time in milliseconds as 64-bit value
static inline uint64_t getSyncTimeMs64(void) {
    return getTimeMs64() + rootClockDeltaMs;
}
//! Get the network-wide synchronized time in seconds as 32-bit value
static inline uint32_t getSyncTimeSec(void) {
    return (uint32_t) (getTimeSec() + rootClockDeltaMs / 1000);
}

#else

#define getSyncTimeMs()    getTimeMs()
#define getSyncTimeMs64()  getTimeMs64()
#define getSyncTimeSec()   getTimeSec()

#endif

#endif
