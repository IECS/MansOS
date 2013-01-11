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

#include <platform.h>
#if USE_THREADS
#include "threads/threads.h"
#endif
#include <lib/energy.h>
#include <timers.h>
#include <print.h>

#ifndef CUSTOM_TIMER_INTERRUPT_HANDLERS

static volatile bool prematureWakeup;

//
// sleep timer interrupt for the case when threads are not used
//
SLEEP_TIMER_INTERRUPT()
{
    if (!SLEEP_TIMER_EXPIRED()) return;

    // we waited till the end, so its not premature
    prematureWakeup = false;

    // exit low power mode
    EXIT_SLEEP_MODE();
}

#endif // !CUSTOM_TIMER_INTERRUPT_HANDLERS

//
// Include msleep() function only on platforms where timer B is available.
//
#if defined TBCTL || defined TBCTL_ || defined TOIE1

void doMsleep(uint16_t milliseconds)
{
    // It is pointless to sleep for 0 time;
    // but do not mess the scheduler and allow this.
    if (!milliseconds) return;

    // PRINTF("doMsleep, ms=%u\n", milliseconds);

    DISABLE_INTS();
    // change energy accounting mode
    energyConsumerOff(ENERGY_CONSUMER_MCU);
    energyConsumerOn(ENERGY_CONSUMER_LPM);
    // setup sleep timer
    SLEEP_TIMER_SET(milliseconds);
    // start timer B
    SLEEP_TIMER_START();
    // stop timer A
    DISABLE_ALARM_INTERRUPT();
    // its premature unless we have got sleep timer interrupt
    prematureWakeup = true;
    // enter low power mode 3
    ENTER_SLEEP_MODE();

    // zzz... sleep... zzz...

    // after wakeup: adjust jiffies
    uint16_t tbr = SLEEP_TIMER_VALUE();
    uint16_t tccr = SLEEP_TIMER_EXPIRY_TIME();
    uint32_t ms = sleepCyclesToMs(prematureWakeup ? tbr : tccr);
    // PRINTF("wakeup from doMsleep, prematureWakeup=%d!\n", (int) prematureWakeup);
    // PRINTF("tbr=%u, tccr=%u, ms=%lu\n", tbr, tccr, ms);
    incRealtime(ms);

    // change energy accounting mode back to active
    energyConsumerOff(ENERGY_CONSUMER_LPM);
    energyConsumerOn(ENERGY_CONSUMER_MCU);

    // restart alarm interrupts
    RESET_ALARM_TIMER();
    ENABLE_ALARM_INTERRUPT();

    // sleep timer should not automatically restart
    SLEEP_TIMER_STOP();

#if USE_THREADS
    // wake up the current thread
    threadWakeup(currentThread->index, THREAD_RUNNING);

    if (processFlags.value
            && (currentThread->index != KERNEL_THREAD_INDEX
            || currentThread->state == THREAD_SLEEPING)) {
        // wake up the kernel thread to process all events that have happened
        threadWakeup(KERNEL_THREAD_INDEX, THREAD_READY);
        // XXX: do not call yield() here to avoid recursion.
        // rather, assume that all user code is well behaved.
        // (i.e. has no excessively long interrupt handlers!)
        // yield();
    }
#endif
}

#endif // CUSTOM_TIMER_INTERRUPT_HANDLERS not defined
