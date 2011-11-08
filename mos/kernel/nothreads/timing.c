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

#include <kernel/alarms_system.h>
#include <platform.h>
#include <hil/radio.h> // XXX

//----------------------------------------------------------
// internal variables
//----------------------------------------------------------

volatile uint32_t jiffies; // real time counter, in ms

#ifndef CUSTOM_TIMER_INTERRUPT_HANDLERS

// alarm timer interrupt handler
ALARM_TIMER_INTERRUPT()
{
    if (!ALARM_TIMER_EXPIRED()) return;

    // reset the counter
    RESET_ALARM_TIMER();

    jiffies += JIFFY_MS_COUNT;

    //
    // Clock error (software, due to rounding) is 32/32768 seconds per second,
    // or 0.99609375 milliseconds per each 1.02 seconds
    // We assume it's precisely 1 millisecond per each 1.02 seconds and fix that error here.
    // The precision is improved 255 times. (0.99609375 compared to 1-0.99609375=0.00390625)
    //
    bool fixed = false;
    if (!((jiffies / 10) % 102)) {  // XXX: division...
        // fix them
        ++jiffies;
        fixed = true;
    }

#ifdef USE_ALARMS
    if (hasAnyReadyAlarms(jiffies)) {
        alarmsProcess();
    }
        // if (getNextAlarm) {
        //     Alarm_t *head = getNextAlarm();
        //     //HACK! the head shouldn't ever have 0 msecs here.
        //     if (head && head->msecs == 0) {
        //         fireAlarm();
        //     } else if (head && (--(head->msecs) == 0)) {
        //         fireAlarm();
        //     }
        // }
#endif // USE_ALARMS

    // TODO: remove this code!
#if USE_RADIO && (RADIO_CHIP==RADIO_CHIP_MRF24J40)
    if (mrf24j40PollForPacket) mrf24j40PollForPacket();
#endif
}

//
// sleep timer interrupt for the case when threads are not used
//
SLEEP_TIMER_INTERRUPT()
{
    if (!SLEEP_TIMER_EXPIRED()) return;

    // restart alarm interrupts
    ENABLE_ALARM_INTERRUPT();

    // sleep timer should not automatically restart
    SLEEP_TIMER_STOP();

    // exit low power mode
    EXIT_SLEEP_MODE();
}

#endif // !CUSTOM_TIMER_INTERRUPT_HANDLERS

#ifndef PLATFORM_PC

void msleep(uint16_t milliseconds)
{
    // setup sleep timer
    SLEEP_TIMER_SET(milliseconds);
    // start timer B
    SLEEP_TIMER_START();
    // stop timer A
    DISABLE_ALARM_INTERRUPT();
    // enter low power mode 3
    ENTER_SLEEP_MODE();
}

uint16_t sleep(uint16_t seconds)
{
    // 
    // Maximal supported sleeping time is 15984 msec.
    // XXX: we do not account for the time that was spent
    // in the loop and in function calls.
    // 
    while (seconds > PLATFORM_MAX_SLEEP_SECONDS) {
        seconds -= PLATFORM_MAX_SLEEP_SECONDS;
        msleep(PLATFORM_MAX_SLEEP_MS);
    }
    msleep(seconds * 1000);

    return 0; // keep this function compatible with sleep() on PC
}

#endif // !PLATFORM_PC
