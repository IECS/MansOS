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

#include <kernel/defines.h>
#include <platform.h>
#include <hil/alarm.h>
#include <hil/radio.h> // XXX

//----------------------------------------------------------
// internal variables
//----------------------------------------------------------

volatile uint32_t realTime; // real time counter, in ms

//----------------------------------------------------------
// function implementations
//----------------------------------------------------------
uint32_t getRealTime() {
    uint32_t ret;
    atomic_read(realTime, ret);
    return ret;
}

void incRealtime(uint32_t inc)
{
    atomic_inc(realTime, inc);
}

#ifndef CUSTOM_TIMER_INTERRUPT_HANDLERS

// alarm timer interrupt handler
ALARM_TIMER_INTERRUPT()
{
    if (ALARM_TIMER_EXPIRED())
    {
        // increment the 'real time' value
        ++realTime;

#ifdef USE_ALARMS
        if (getNextAlarm) {
            Alarm_t *head = getNextAlarm();
            //HACK! the head shouldn't ever have 0 msecs here.
            if (head && head->msecs == 0) {
                fireAlarm();
            } else if (head && (--(head->msecs) == 0)) {
                fireAlarm();
            }
        }
#endif // USE_ALARMS

        // TODO: remove this code!
#if USE_RADIO && (RADIO_CHIP==RADIO_CHIP_MRF24J40)
        if ((realTime & 7) == 0) {
            if (mrf24j40PollForPacket) mrf24j40PollForPacket();
        }
#endif
    }
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

