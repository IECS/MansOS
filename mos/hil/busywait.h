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

#ifndef MANSOS_BUSWYAIT_H
#define MANSOS_BUSWYAIT_H

#include <kernel/defines.h>
#include <timers.h>

static inline uint16_t timerTicksRead(void)
{
    uint16_t t1 = ALARM_TIMER_VALUE();
    uint16_t t2;
    do {
        t2 = ALARM_TIMER_VALUE();
    } while (t1 == t2);
    return t2;
}

// works with interrupts disabled, but max time is ~2 seconds
#define BUSYWAIT_UNTIL(cond, maxTime, ok)                               \
    do {                                                                \
        uint16_t endTime = timerTicksRead() + maxTime;                  \
        ok = false;                                                     \
        do {                                                            \
            if (cond) {                                                 \
                ok = true;                                              \
                break;                                                  \
            }                                                           \
        } while (timeAfter16(endTime, timerTicksRead()));               \
    } while (0)

// buswait for courtesy only: do not care about the result
#define BUSYWAIT_UNTIL_NORET(cond, maxTime)                             \
    do {                                                                \
        uint16_t endTime = timerTicksRead() + maxTime;                  \
        do {                                                            \
            if (cond) break;                                            \
        } while (timeAfter16(endTime, timerTicksRead()));               \
    } while (0)

// interrupts must be enabled!
#define BUSYWAIT_UNTIL_LONG(cond, maxTime, ok)                          \
    do {                                                                \
        uint32_t endTime = getRealTime() + maxTime;                     \
        ok = false;                                                     \
        do {                                                            \
            if (cond) {                                                 \
                ok = true;                                              \
                break;                                                  \
            }                                                           \
        } while (timeAfter32(endTime, getRealTime()));                  \
    } while (0)


#endif
