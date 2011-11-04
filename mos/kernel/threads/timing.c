/**
 * Copyright (c) 2008-2010 Leo Selavo and the contributors. All rights reserved.
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
#include "alarms.h"
#include "platform_hpl.h"
#include <hil/radio.h> // XXX

volatile uint32_t jiffies;

// --------------------------------------------- alarm / time accounting timer

ALARM_TIMER_INTERRUPT()
{
    // Check the interrupt vector. Value 10 means timer overflow.
    if (!ALARM_TIMER_EXPIRED()) return;

    // reset the counter
    ALARM_TIMER_VALUE = 0;
    // SET_NEXT_ALARM_TIMER(PLATFORM_ALARM_TIMER_PERIOD);

    jiffies += JIFFY_MS_COUNT;

    //
    // Clock error (software, due to rounding) is 32/32768 seconds per second,
    // or 0.99609375 milliseconds per each 1.02 seconds
    // We assume it's precisely 1 millisecond per each 1.02 seconds and fix that error here.
    // The precision is improved 255 times. (0.99609375 against 1-0.99609375=0.00390625)
    //
    bool fixed = false;
    if (!((jiffies / 10) % 102)) {  // XXX: division...
        // fix them
        ++jiffies;
        fixed = true;
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
        scheduleProcessAlarms();
    }
    if (processFlags.value) {
        if (currentThread->index != KERNEL_THREAD_INDEX) {
            // wake up the kernel thread to process all events that have happened
            threadWakeup(KERNEL_THREAD_INDEX, THREAD_READY);
            yield();
        }
    } else if (fixed) {
        // forced preemption in user context: possibly schedule a new thread
        yield();
    }
}

// --------------------------------------------- sleep timer

void doMsleep(uint16_t milliseconds)
{
    // setup sleep timer
    SLEEP_TIMER_SET(milliseconds);
    // start timer B
    SLEEP_TIMER_START();
    // disable alarm interrupt generation
    DISABLE_ALARM_INTERRUPT();
    // enter low power mode 3 and make sure interrupts are enabled
    ENTER_SLEEP_MODE();
}

SLEEP_TIMER_INTERRUPT()
{
    if (!SLEEP_TIMER_EXPIRED()) return;

    // restart alarm interrupts
    ENABLE_ALARM_INTERRUPT();

    // sleep timer should not automatically restart
    SLEEP_TIMER_STOP();

    // wake up the current thread
    threadWakeup(currentThread->index, THREAD_RUNNING);

    // exit low power mode
    EXIT_SLEEP_MODE();
}
