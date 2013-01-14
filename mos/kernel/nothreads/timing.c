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
#include <platform.h>
#include <timing.h>

#if USE_PROTOTHREADS
#include <etimer.h>
#endif

#ifndef CUSTOM_TIMER_INTERRUPT_HANDLERS

// alarm timer interrupt handler
ALARM_TIMER_INTERRUPT()
{
    if (ALARM_TIMER_WRAPAROUND()) {
        //
        // On MSP430 platforms binary ACLK oscillator usually is used.
        // It has constant rate 32768 Hz (the ACLK_SPEED define)
        // When ACLK ticks are converted to milliseconds, rounding error is introduced.
        // When TIMER_INTERRUPT_HZ = 1000, there are 32 ACLK ticks per millisecond;
        // when TIMER_INTERRUPT_HZ = 100, there are 32.7 ACLK ticks per millisecond.
        // The clock errors are (65536 / 32) - 2000 = 48 milliseconds
        // and 4.159 or approximately 4 milliseconds respectively.
        // We improve the precision by applying the fix once per every wraparound.
        //
        jiffies += IMPRECISION_PER_WRAPAROUND;

        ALARM_TIMER_RESET_WRAPAROUND();
        return;
    }

    if (!ALARM_TIMER_EXPIRED()) return;

    // advance the CCR
    SET_NEXT_ALARM_TIMER(PLATFORM_ALARM_TIMER_PERIOD);

    jiffies += JIFFY_TIMER_MS;

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
}

#endif // !CUSTOM_TIMER_INTERRUPT_HANDLERS
