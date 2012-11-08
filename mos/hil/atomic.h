/**
 * Copyright (c) 2012 the MansOS team. All rights reserved.
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

#ifndef MANSOS_HIL_ATOMIC
#define MANSOS_HIL_ATOMIC

#include <kernel/defines.h>
#include <platform.h>

extern volatile ticks_t jiffies;

//
// More heavyweight versions of ATOMIC_START/ATOMIC_END macros
// that take in account timer ticks during interrupt phase.
// Note that interrupt handlers are called as soon as interrupts are enabled,
// so delays < 10ms will be handled by the default logic
//
#define ATOMIC_START_TIMESAVE(h) {                                    \
    const uint16_t _start_time = ALARM_TIMER_VALUE();                 \
    ATOMIC_START(h)

#define ATOMIC_END_TIMESAVE(h)                                        \
    if (h != 0) {                                                     \
        const uint16_t _time_diff = ALARM_TIMER_VALUE() - _start_time;\
        if (_time_diff > PLATFORM_ALARM_TIMER_PERIOD) {               \
            jiffies += TIMER_TICKS_TO_MS(                             \
                    _time_diff - PLATFORM_ALARM_TIMER_PERIOD);        \
        }                                                             \
    }                                                                 \
    ATOMIC_END(h);                                                    \
    }

#endif
