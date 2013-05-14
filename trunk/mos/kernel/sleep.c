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

#include <platform.h>
#if USE_THREADS
#include "threads/threads.h"
#endif
#include <lib/energy.h>
#include "sleep_internal.h"
#include <print.h>
#include <leds.h>
#include <radio.h>

#ifndef CUSTOM_TIMER_INTERRUPT_HANDLERS

static volatile uint16_t timeWentToSleep;
static volatile uint16_t ticksInSleepMode;
static volatile uint16_t jiffiesInSleepMode;

static inline void sleepTimerSet(uint16_t ms)
{
    if (ms > PLATFORM_MAX_SLEEP_MS) {
        ms = PLATFORM_MAX_SLEEP_MS;
    } else if (ms < PLATFORM_MIN_SLEEP_MS) {
        ms = PLATFORM_MIN_SLEEP_MS;
    }

    uint16_t sleepTicks = convertMsToSleepTimer(ms);

    // wait for end of tick and read the new value
    uint16_t ticksNow = SLEEP_TIMER_READ();
    timeWentToSleep = ticksNow;

    SLEEP_TIMER_REGISTER = ticksNow + sleepTicks;
}

void doMsleep(uint16_t milliseconds)
{
    if (milliseconds) {
        // PRINTF("doMsleep, ms=%u\n", milliseconds);

        DISABLE_INTS();
        // change energy accounting mode
        energyConsumerOff(ENERGY_CONSUMER_MCU);
        energyConsumerOn(ENERGY_CONSUMER_LPM);

        // setup sleep timer
        sleepTimerSet(milliseconds);
        // stop timer A
        ALARM_TIMER_STOP();
        // start timer B
        SLEEP_TIMER_START();
        // enter low power mode
        ENTER_SLEEP_MODE();

        // zzz... sleep... zzz...

        // after wakeup: determine for how long we actually slept
        // (unexpected wakeups are possible because of interrupts)
        uint16_t ticksSlept = SLEEP_TIMER_READ() - timeWentToSleep;

        // sleep timer should not automatically restart
        SLEEP_TIMER_STOP();

        // advance alarm and correction timers too
        ALARM_TIMER_REGISTER += ticksSlept;
#if PLATFORM_HAS_CORRECTION_TIMER
        CORRECTION_TIMER_REGISTER += ticksSlept;
#endif
        ticksInSleepMode += ticksSlept;
        // this is calibrated for 32 ticks ~= 1 jiffy
        jiffies += ticksInSleepMode / 32;
        jiffiesInSleepMode += ticksInSleepMode / 32;
        ticksInSleepMode %= 32;
        // on every 128 jiffies the time correction is 3 jiffies,
        // because 4000 ticks = 125 and 4096 = 128 uncorrected jiffies
        // but should be 4096 ticks = 125 corrected jiffies
        while (jiffiesInSleepMode >= 128) {
            // PRINTF("@");
            jiffiesInSleepMode -= 128;
            jiffies -= 3;
        }

        // change energy accounting mode back to active
        energyConsumerOff(ENERGY_CONSUMER_LPM);
        energyConsumerOn(ENERGY_CONSUMER_MCU);

        // fix jiffies taking into account the time spent for local processing
        while (!timeAfter16(ALARM_TIMER_REGISTER, ALARM_TIMER_READ_STOPPED())) {
            // PRINTF("*");
            jiffies += JIFFY_TIMER_MS;
            ALARM_TIMER_REGISTER += PLATFORM_ALARM_TIMER_PERIOD;
        }
#if PLATFORM_TIME_CORRECTION_PERIOD
        while (!timeAfter16(CORRECTION_TIMER_REGISTER, CORRECTION_TIMER_READ_STOPPED())) {
            // PRINTF("#");
            CORRECTION_TIMER_REGISTER += PLATFORM_TIME_CORRECTION_PERIOD;
            jiffies -= 3;
        }
#endif
        // restart timer A (count from the place where it left)
        ALARM_TIMER_START();
    } else {
        // Zero sleep time requested. Allow, because this way
        // scheduler code becomes simpler, but don't even try going to sleep.
    }

#if USE_THREADS
    // wake up the current thread
    threadWakeup(currentThread->index, THREAD_RUNNING);

    if (processFlags.value) {
        // wake up the kernel thread to process all events that have happened
        threadWakeup(KERNEL_THREAD_INDEX, THREAD_READY);
        // Do not call yield() here to avoid recursion!
        // rather, assume that all user code is well-behaved.
        // (i.e. has no excessively long interrupt handlers)
    }
#endif
}

#endif // CUSTOM_TIMER_INTERRUPT_HANDLERS not defined
