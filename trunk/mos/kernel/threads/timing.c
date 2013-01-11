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

#include "threads.h"
#include <kernel/alarms_system.h>
#include <platform.h>
#include <radio.h> // XXX

// --------------------------------------------- alarm / time accounting timer

ALARM_TIMER_INTERRUPT()
{
    // advance the CCR
    SET_NEXT_ALARM_TIMER(PLATFORM_ALARM_TIMER_PERIOD);

    jiffies += JIFFY_TIMER_MS;

    //
    // Clock error (software, due to rounding) is 32/32768 seconds per second,
    // or 0.99609375 milliseconds per each 1.02 seconds
    // We assume it's precisely 1 millisecond per each 1.02 seconds and fix that error here.
    // The precision is improved 255 times: 0.99609375 compared to 1 - 0.99609375 = 0.00390625
    //
    bool fixed = false;
    static ticks_t lastFixedJiffies;
    if (jiffies - lastFixedJiffies > 1020) {
        // fix them
        ++jiffies;
        fixed = true;
        lastFixedJiffies += 1020;
    }

// #ifdef DEBUG_THREADS
//     if (!(jiffies & 127)) {
//         checkThreadLockups();
//     }
// #endif

#if USE_RADIO && (RADIO_CHIP==RADIO_CHIP_MRF24J40)
    // TODO: fix radio interrupts and remove this code!
    if (mrf24j40PollForPacket) mrf24j40PollForPacket();
#endif

    // check if any alarm has expired and mark it for processing
    if (!processFlags.bits.alarmsProcess) {
        // ..but avoid calling it more than once! (in case the call takes >1 jiffy)
        scheduleProcessAlarms(jiffies);
    }
    if (processFlags.value) {
        if (currentThread->index != KERNEL_THREAD_INDEX
                || currentThread->state == THREAD_SLEEPING) {
            // wake up the kernel thread to process all events that have happened
            threadWakeup(KERNEL_THREAD_INDEX, THREAD_READY);
            yield();
        }
    } else if (fixed) {
        // forced preemption in user context: possibly schedule a new thread
        yield();
    }
}
