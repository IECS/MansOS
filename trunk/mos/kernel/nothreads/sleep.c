/*
 * Copyright (c) 2008-2013 the MansOS team. All rights reserved.
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

#include <kernel/alarms_internal.h>
#include <kernel/sleep_internal.h>
#include <timing.h>

#include <print.h>

//
// Put the system to sleep for a specific time;
// version for event-based kernel.
//
void msleep(uint16_t ms)
{
    const uint32_t sleepEnd = (uint32_t) getJiffies() + ms;
    // PRINTF("msleep %u, end=%lu\n", ms, sleepEnd);

    bool allTimeSpent;
    for (;;) {
        // how much to sleep?
        uint32_t now = (uint32_t) getJiffies();
        int16_t msToSleep = sleepEnd - now;
        // if time has passed, quit now
        if (msToSleep <= 0) break;

        // calculate time to sleep: minumum of 'ms' and time to next alarm
        Handle_t handle;
        ATOMIC_START(handle);
        Alarm_t *first = SLIST_FIRST(&alarmListHead);
        if (first && timeAfter32(sleepEnd, first->jiffies)) {
            msToSleep = first->jiffies - now;
            // PRINTF("alarms, dont sleep to end!, msToSleep=%u\n", msToSleep);
            // do the alarm processing with enabled interrupts - it can take long!
            // make sure no outstanding alarms are present
            if (msToSleep <= 0) {
                // PRINTF("process and restart\n");
                ATOMIC_END(handle);
                // take care of expired alarms
                alarmsProcess();
                // restart the loop
                continue;
            }
            allTimeSpent = false;
        } else if (msToSleep > PLATFORM_MAX_SLEEP_MS) {
            // maximum value allowed by platform
            msToSleep = PLATFORM_MAX_SLEEP_MS;
        } else {
            // PRINTF("sleep to end\n");
            allTimeSpent = true;
        }

        // do the real sleeping
        doMsleep(msToSleep);

        ATOMIC_END(handle);
#if 1
        // check for exit conditions
        if (allTimeSpent) break;
#else
        // use to achieve "wake-on-interrupt" like behavior,
        // given that the there is LPM_EXIT in interrupt handlers
        break;
#endif
    }
}
