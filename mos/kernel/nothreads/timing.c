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
    if (!((jiffies / 10) % 102)) {  // XXX: division...
        // fix them
        ++jiffies;
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

#endif // !CUSTOM_TIMER_INTERRUPT_HANDLERS
