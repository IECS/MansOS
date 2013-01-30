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

#ifndef TIMERS_HAL_H
#define TIMERS_HAL_H

// PC platform uses NO interrupts
#define ALARM_TIMER_INTERRUPT0() void alarmTimerInterrupt0(void)
#define ALARM_TIMER_INTERRUPT1() void alarmTimerInterrupt1(void)
#define SLEEP_TIMER_INTERRUPT() void sleepTimerInterrupt(void)

#define ALARM_TIMER_START()
#define ALARM_TIMER_EXPIRED() (1)
#define ALARM_TIMER_READ_STOPPED() 0
#define SET_NEXT_ALARM_TIMER(value)
#define NEXT_ALARM_TIMER() 0

#define ALARM_TIMER_WRAPAROUND() false
#define ALARM_TIMER_RESET_WRAPAROUND()

#define SLEEP_TIMER_STOP()
#define SLEEP_TIMER_EXPIRED() (1)
#define SLEEP_TIMER_READ() 0

#define ENTER_SLEEP_MODE()
#define EXIT_SLEEP_MODE()

// semaphore used for SLEEP implementation
extern sem_t sleepSem;
#define SLEEP() mos_sem_wait(&sleepSem)
#define EXIT_SLEEP() mos_sem_post(&sleepSem)

enum {
    PLATFORM_MIN_SLEEP_MS = 1, // min sleep amount = 1ms
    PLATFORM_MAX_SLEEP_MS = 0xffff,
    PLATFORM_MAX_SLEEP_SECONDS = PLATFORM_MAX_SLEEP_MS / 1000,
    PLATFORM_ALARM_TIMER_PERIOD = 1,
};

#define TIMER_TICKS_TO_MS(ticks) ticks

#define JIFFY_TIMER_MS 1

#define ACLK_SPEED 1
#define ALARM_CYCLES        1
#define ALARM_CYCLES_DEC    0
#define JIFFY_CLOCK_SPEED   1
#define JIFFY_CLOCK_DIVIDER 1

#define SLEEP_CYCLES        1
#define SLEEP_CYCLES_DEC    0
#define SLEEP_CLOCK_SPEED   1
#define SLEEP_CLOCK_DIVIDER 1

#endif
