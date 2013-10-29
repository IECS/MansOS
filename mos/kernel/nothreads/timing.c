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

#include <kernel/alarms_internal.h>
#include <platform.h>
#include <print.h>
#include <timing.h>

#if USE_PROTOTHREADS
#include <etimer.h>
#endif

#ifndef CUSTOM_TIMER_INTERRUPT_HANDLERS

// alarm timer interrupt handler
ALARM_TIMER_INTERRUPT0()
{
    // Advance jiffies (MansOS time counter) while counter register <= counter
    // Spurios interrupts may happen when the alarm timer is restarted after stopping!
    while (!timeAfter16(ALARM_TIMER_REGISTER, ALARM_TIMER_READ_STOPPED())) {
        jiffies += JIFFY_TIMER_MS;
        ALARM_TIMER_REGISTER += PLATFORM_ALARM_TIMER_PERIOD;
    }

#ifdef USE_ALARMS
    if (hasAnyReadyAlarms(jiffies)) {
        alarmsProcess();
    }
#endif
#ifdef USE_PROTOTHREADS
    if (etimer_pending() && !etimer_polled()
            && !timeAfter32(jiffies, etimer_next_expiration_time())) {
         etimer_request_poll();
         EXIT_SLEEP_MODE();
    }
#endif

    // If TAR still > TACCR0 at this point, we are in trouble:
    // the interrupt will not be generated until the next wraparound (2 seconds).
    // So avoid it at all costs.
    while (!timeAfter16(ALARM_TIMER_REGISTER, ALARM_TIMER_READ_STOPPED() + 10)) {
        jiffies += JIFFY_TIMER_MS;
        ALARM_TIMER_REGISTER += PLATFORM_ALARM_TIMER_PERIOD;
    }

    ALARM_INTERRUPT_CLEAR();
}

ALARM_TIMER_INTERRUPT1()
{
    switch (TIMER_INTERRUPT_VECTOR) {
#if PLATFORM_HAS_CORRECTION_TIMER
    case CORRECTION_TIMER_EXPIRED:
        //
        // On MSP430 platforms binary ACLK oscillator usually is used.
        // It has constant rate 32768 Hz (the ACLK_SPEED define)
        // When ACLK ticks are converted to milliseconds, rounding error is introduced.
        // When TIMER_INTERRUPT_HZ = 1000, there are 32 ACLK ticks per millisecond;
        // The clock error is (32768 / 32) - 1000 = 1024 - 1000 = 24 milliseconds.
        // We improve the precision by applying a fix 24/3 = 8 times per second.
        //
        while (!timeAfter16(CORRECTION_TIMER_REGISTER, ALARM_TIMER_READ_STOPPED() + 10)) {
            CORRECTION_TIMER_REGISTER += PLATFORM_TIME_CORRECTION_PERIOD;
            jiffies -= 3;
        }
        CORRECTION_INTERRUPT_CLEAR();
        break;
#endif // PLATFORM_HAS_CORRECTION_TIMER

    case SLEEP_TIMER_EXPIRED:
        // exit low power mode
        EXIT_SLEEP_MODE();
        SLEEP_INTERRUPT_CLEAR();
        break;
    }
}

#endif // !CUSTOM_TIMER_INTERRUPT_HANDLERS
