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
    static volatile bool isWraparound;

    if (ALARM_TIMER_WRAPAROUND()) {
        //
        // On MSP430 platforms binary ACLK oscillator usually is used.
        // It has constant rate 32768 Hz (the ACLK_SPEED define)
        // When ACLK ticks are converted to milliseconds, rounding error is introduced.
        // When TIMER_INTERRUPT_HZ = 1000, there are 33 ACLK ticks per millisecond.
        // When TIMER_INTERRUPT_HZ = 100, there are 32.8 ACLK ticks per milliseconds.
        // The clock errors are 2000 - (65536 / 33) = 14.(06) milliseconds
        // and 1.95 milliseconds per wraparound period respectively.
        // We assume its precisely 14 milliseconds and 2 milliseconds respectively
        // and improve the precision by applying the fix once per every wraparound.
        // It is improved 232 and 40 times respectively.
        // The final imprecision at 1000 Hz is 30.3 ppm, or 109.09 ms drift per hour.
        //
        jiffies += IMPRECISION_PER_WRAPAROUND;
#if TIMER_INTERRUPT_HZ == 1000
        // Fix even that small imprecision by adding 2 milliseconds
        // once every 33 wraparound periods. This precision time accounting
        // is supported only with 1000 Hz interrupt frequency!
        static uint8_t numWraparoundTimes;
        numWraparoundTimes++;
        if (numWraparoundTimes == 33) {
            jiffies += 2;
            numWraparoundTimes = 0;
        }
#endif
        isWraparound = true;
        ALARM_TIMER_RESET_WRAPAROUND();
        return;
    }

    if (!ALARM_TIMER_EXPIRED()) return;

    // advance the CCR
    SET_NEXT_ALARM_TIMER(PLATFORM_ALARM_TIMER_PERIOD);

    jiffies += JIFFY_TIMER_MS;

#if USE_RADIO && (RADIO_CHIP==RADIO_CHIP_MRF24J40)
    // TODO: fix radio interrupts and remove this code!
    if (mrf24j40PollForPacket) mrf24j40PollForPacket();
#endif

    bool wasWraparound = isWraparound;
    isWraparound = false;

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
    } else if (wasWraparound) {
        // forced preemption in user context: possibly schedule a new thread
        yield();
    }
}
