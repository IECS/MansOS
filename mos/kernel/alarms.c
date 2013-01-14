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

#if USE_THREADS
#include "threads/threads.h"
#endif
#include "alarms_system.h"
#include <timing.h>
#include <print.h>

// the global list with all alarms
AlarmList_t alarmListHead;

void initAlarms(void)
{
    SLIST_INIT(&alarmListHead);

    ALARM_TIMER_START();
}

void alarmsProcess(void)
{
    //
    // Note that this code works in the way that neither locking the list
    // nor disabling interrupts is required
    //
    uint32_t now = (uint32_t) getJiffies();

    // PRINTF("processAlarms: jiffies=%lu\n", now);

    Alarm_t **a = &SLIST_FIRST(&alarmListHead);
    while (*a) {
        Alarm_t *ap = *a;

        // PRINTF("processAlarms: ap=%p, ap->jiffies=%lu\n", ap, ap->jiffies);

        if (timeAfter32(ap->jiffies, now)) {
            break;
        }

        // save next alarm for later reference
        Alarm_t **next = &SLIST_NEXT(ap, chain);
        // remove the alarm from the list
        *a = *next;
        // call the callback
        ap->callback(ap->data);
        // move to the next
        a = next;
    }
}

void alarmSchedule(Alarm_t *alarm, uint32_t milliseconds)
{
    // PRINTF("alarmSchedule %p, ms=%lu\n", alarm, milliseconds);
    alarm->jiffies = (uint32_t)getJiffies() + ms2jiffies(milliseconds);

    // locking is required, because both kernel and user threads can be using this function
    Handle_t h;
    ATOMIC_START(h);

    // unschedule the alarm, if it was already scheduled
    SLIST_REMOVE_SAFE(&alarmListHead, alarm, Alarm_s, chain);

    // insert it in appropriate position
    Alarm_t **prev = &SLIST_FIRST(&alarmListHead);
    Alarm_t *a = *prev;
    while (a && timeAfter32(alarm->jiffies, a->jiffies)) {
        prev = &SLIST_NEXT(a, chain);
        a = *prev;
    }
    SLIST_INSERT(prev, alarm, chain);

#if USE_THREADS
    // always reschedule alarm processing in case some alarm was added
    // that might need to be processed before end of current kernel sleep time
    processFlags.bits.alarmsProcess = true;
    // and make sure the kernel thread is awake and ready to deal with it
    threadWakeup(KERNEL_THREAD_INDEX, THREAD_READY);
#endif

    ATOMIC_END(h);
}

void alarmRemove(Alarm_t *alarm)
{
    Handle_t h;
    ATOMIC_START(h);
    SLIST_REMOVE_SAFE(&alarmListHead, alarm, Alarm_s, chain);
    ATOMIC_END(h);
}

uint32_t getAlarmTime(Alarm_t *alarm)
{
    return (uint32_t)(alarm->jiffies - getJiffies());
}
