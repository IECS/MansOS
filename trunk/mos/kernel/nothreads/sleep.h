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

#include <kernel/alarms_system.h>
#include <kernel/timing.h>

#include <lib/dprint.h>

//
// Put current thread to sleep for a specific time
//
static inline void msleep(uint16_t ms)
{
    const uint32_t sleepEnd = (uint32_t) getJiffies() + ms;
    // PRINTF("msleep %u, end=%lu\n", ms, sleepEnd);

    bool allTimeSpent;
    do {
        // how much to sleep?
        uint32_t now = (uint32_t) getJiffies();
        int16_t msToSleep = sleepEnd - now;
        // if time has passed, quit now
        if (msToSleep <= 0) break;

        // calculate time to sleep: minumum of 'ms' and time to next alarm
        Alarm_t *first = SLIST_FIRST(&alarmListHead);
        if (first && timeAfter32(sleepEnd, first->jiffies)) {
            // PRINTF("alarms, dont sleep to end!\n");
            msToSleep = first->jiffies - now;
            // make sure no outstanding alarms are present
            if (msToSleep <= 0) {
                // PRINTF("process and restart\n");
                // take care of expired alarms
                alarmsProcess();
                // restart the loop
                continue;
            }
            allTimeSpent = false;
        } else {
            // PRINTF("sleep to end\n");
            allTimeSpent = true;
        }
        // do the real sleep
        doMsleep(msToSleep);
        //check for exit conditions
    } while (!allTimeSpent);
}
