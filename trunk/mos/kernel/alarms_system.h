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

//
// Alarm implementation (system-only API)
//

#ifndef MANSOS_ALARMS_SYSTEM_H
#define MANSOS_ALARMS_SYSTEM_H

// include user API
#include <hil/alarms.h>

typedef SLIST_HEAD(head, Alarm_s) AlarmList_t;

// the global list with all alarms
extern AlarmList_t alarmListHead;

// initiaze alarms
void initAlarms(void);

// called from kernel context (when threads are enabled)
// or from interrupt context (when threads are not enabled)
void alarmsProcess(void);

#if USE_EXP_THREADS
// called from interrupt context
static inline void scheduleProcessAlarms(uint32_t now)
{
    // if there are no alarms, return
    if (SLIST_EMPTY(&alarmListHead)) return;
    // take the fist alarm and compare its time with the current time
    if (timeAfter32(SLIST_FIRST(&alarmListHead)->jiffies, now) == false) {
        processFlags.bits.alarmsProcess = true;
    }
}

// used for kernel to determine how long to put kernel thread to sleep
static inline uint32_t getNextAlarmTime(void)
{
    Alarm_t *first = SLIST_FIRST(&alarmListHead);
    return first ? first->jiffies : getJiffies() + MAX_KERNEL_SLEEP_TIME;
}

#else // not using threads

// called from interrupt context
static inline bool hasAnyReadyAlarms(uint32_t now) {
    // if there are no alarms, return false
    if (SLIST_EMPTY(&alarmListHead)) return false;
    // take the fist alarm and compare its time with the current time
    return !timeAfter32(SLIST_FIRST(&alarmListHead)->jiffies, now);
}

#endif // USE_EXP_THREADS

#endif
