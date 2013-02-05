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

#ifndef CUSTOM_TIMER_INTERRUPT_HANDLERS

//
// Include sleep functionality only on platforms where timer B is available.
//
#if PLATFORM_HAS_TIMERB

volatile uint16_t timeWentToSleep;
volatile uint16_t millisecondsInSleepMode;

//
// sleep timer interrupt handler
//
SLEEP_TIMER_INTERRUPT()
{
    if (SLEEP_TIMER_WRAPAROUND()) {
        //
        // Fix jiffies for greater good/precision!
        //
        uint16_t milliseconds = convertSleepTimerToMs(-timeWentToSleep);
        jiffies += milliseconds;
        millisecondsInSleepMode += milliseconds;
        // Now we know we have slept for exactly 16000 milliseconds;
        // use the value of how much we *think* we have slept to calc correction.
        int16_t correction = 16000 - millisecondsInSleepMode;
        // PRINTF("sleep timer correction = %d ms\n", correction);
        jiffies += correction;

        timeWentToSleep = 0;
        millisecondsInSleepMode = 0;
        SLEEP_TIMER_RESET_WRAPAROUND();

        if (!(SLEEP_TIMER_EXPIRED())) return;
    }

    // exit low power mode
    EXIT_SLEEP_MODE();
}

static inline void sleepTimerSet(uint16_t ms)
{
    if (ms > PLATFORM_MAX_SLEEP_MS) {
        ms = PLATFORM_MAX_SLEEP_MS;
    } else if (ms < PLATFORM_MIN_SLEEP_MS) {
        ms = PLATFORM_MIN_SLEEP_MS;
    }

    uint16_t tbr = SLEEP_TIMER_READ_STOPPED();
    timeWentToSleep = tbr;

    SLEEP_TIMER_SET(tbr + convertMsToSleepTimer(ms));
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
        // start timer B
        SLEEP_TIMER_START();
        // stop timer A
        ALARM_TIMER_STOP();
        // enter low power mode
        ENTER_SLEEP_MODE();

        // zzz... sleep... zzz...

        // after wakeup: determine for how long we actually slept
        // (unexpected wakeups are possible because of interrupts)
        milliseconds = convertSleepTimerToMs(
                SLEEP_TIMER_READ() - timeWentToSleep);
        // adjust jiffies accordingly
        jiffies += milliseconds;
        millisecondsInSleepMode += milliseconds;

        // change energy accounting mode back to active
        energyConsumerOff(ENERGY_CONSUMER_LPM);
        energyConsumerOn(ENERGY_CONSUMER_MCU);

        // restart timer A (count from the place where it left)
        ALARM_TIMER_START();

        // sleep timer should not automatically restart
        SLEEP_TIMER_STOP();
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

#endif // Timer B present

#endif // CUSTOM_TIMER_INTERRUPT_HANDLERS not defined
