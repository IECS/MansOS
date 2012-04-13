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

#ifndef _PLATFORM_PC_H_
#define _PLATFORM_PC_H_

#include <unistd.h>
#include "sem_hal.h"
#include "leds_hal.h"
#include "alarms_hal.h"
#include "adc_hal.h"

#include <hil/gpio.h>

// PC platform uses NO timers
#define DISABLE_INTS() // nothing
#define ENABLE_INTS()  // nothing

#define ALARM_TIMER_INTERRUPT() void alarmTimerInterrupt(void)
#define SLEEP_TIMER_INTERRUPT() void sleepTimerInterrupt(void)

#define ALARM_TIMER_START()
#define ENABLE_ALARM_INTERRUPT()
#define DISABLE_ALARM_INTERRUPT()
#define ALARM_TIMER_EXPIRED() (1)
#define RESET_ALARM_TIMER()

#define SLEEP_TIMER_STOP()
#define SLEEP_TIMER_EXPIRED() (1)

#define ENTER_SLEEP_MODE()
#define EXIT_SLEEP_MODE()

// semaphore used for SLEEP implementation
extern sem_t sleepSem;
#define SLEEP() mos_sem_wait(&sleepSem)
#define EXIT_SLEEP() mos_sem_post(&sleepSem)

void initPlatform(void);

#ifndef PRINT_BUFFER_SIZE
#define PRINT_BUFFER_SIZE 127
#endif

enum {
    PLATFORM_MIN_SLEEP_MS = 10, // min sleep amount = 10ms
    PLATFORM_MAX_SLEEP_MS = 0xffff,
    PLATFORM_MAX_SLEEP_SECONDS = PLATFORM_MAX_SLEEP_MS / 1000,
};

#define JIFFY_TIMER_MS 1

#define SLEEP_CYCLES 0
#define SLEEP_CYCLES_DEC 0

#define ATOMIC_START(x) ((void) x)
#define ATOMIC_END(x)   ((void) x)

#define atomic(op)  op
#define atomic_read(x, ret)  atomic(ret = x)
#define atomic_write(x, y)  atomic(x = y)
#define atomic_inc(x, c)  atomic(x += c)
#define atomic_dec(x, c)  atomic(x -= c)

// LEDs: all present! Defined in pc/ledslist.h

#endif
