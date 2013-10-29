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

#include "threads.h"
#include <timing.h>
#include <kernel/alarms_internal.h>
#include <radio.h> // XXX
#include <lib/dprint.h>

// --------------------------------------------- alarm / time accounting timer

static volatile bool wasWraparound;

ALARM_TIMER_INTERRUPT0()
{
    // Advance jiffies (MansOS time counter) while counter register <= counter
    // Spurios interrupts may happen when the alarm timer is restarted after stopping!
    while (!timeAfter16(ALARM_TIMER_REGISTER, ALARM_TIMER_READ_STOPPED())) {
        jiffies += JIFFY_TIMER_MS;
        ALARM_TIMER_REGISTER += PLATFORM_ALARM_TIMER_PERIOD;
    }

#if USE_RADIO && (RADIO_CHIP==RADIO_CHIP_MRF24J40)
    // TODO: fix radio interrupts and remove this code!
    if (mrf24j40PollForPacket) mrf24j40PollForPacket();
#endif

    bool doYield = false;

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
            doYield = true;
        }
    } else if (wasWraparound) {
        // forced preemption in user context: possibly schedule a new thread
        doYield = true;
    }

    wasWraparound = false;

    // If TAR still > TACCR0 at this point, we are in trouble:
    // the interrupt will not be generated until the next wraparound (2 seconds).
    // So avoid it at all costs.
    while (!timeAfter16(ALARM_TIMER_REGISTER, ALARM_TIMER_READ_STOPPED() + 10)) {
        jiffies += JIFFY_TIMER_MS;
        ALARM_TIMER_REGISTER += PLATFORM_ALARM_TIMER_PERIOD;
    }

    ALARM_INTERRUPT_CLEAR();

    if (doYield) {
        yield();
    }
}


ALARM_TIMER_INTERRUPT1()
{
    switch (TIMER_INTERRUPT_VECTOR) {
#if PLATFORM_HAS_CORRECTION_TIMER
    case CORRECTION_TIMER_EXPIRED:
        if (CORRECTION_TIMER_REGISTER <= PLATFORM_ALARM_TIMER_PERIOD) {
            wasWraparound = true;
        }

        //
        // On MSP430 platforms binary ACLK oscillator usually is used.
        // It has constant rate 32768 Hz (the ACLK_SPEED define)
        // When ACLK ticks are converted to milliseconds, rounding error is introduced.
        // When TIMER_INTERRUPT_HZ = 1000, there are 32 ACLK ticks per millisecond;
        // The clock error is (32768 / 32) - 1000 = 1024 - 1000 = 24 milliseconds.
        // We improve the precision by applying a fix 24/3 = 8 times per second.
        //
        while (!timeAfter16(CORRECTION_TIMER_REGISTER, ALARM_TIMER_READ_STOPPED() + 10)) {
            // if (CORRECTION_TIMER_REGISTER <= PLATFORM_TIME_CORRECTION_PERIOD) {
            //     PRINTF("@");
            // }
            CORRECTION_TIMER_REGISTER += PLATFORM_TIME_CORRECTION_PERIOD;
            jiffies -= 3;
        }
        CORRECTION_INTERRUPT_CLEAR();
        break;
#endif // PLATFORM_HAS_CORRECTION_TIMER

    case SLEEP_TIMER_EXPIRED:
        // PRINTF("*");
        // exit low power mode
        EXIT_SLEEP_MODE();
        SLEEP_INTERRUPT_CLEAR();
        break;
    }
}
