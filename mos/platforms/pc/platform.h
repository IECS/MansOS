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

#include "sem_hal.h"
#include "leds_hal.h"
#include "alarms_hal.h"
#include "adc_hal.h"

#include <arch/null_spi.h>

#include <digital.h>

#if USE_FATFS
//
// As stdio.h cannot be included: define some of
// frequently used function prototypes.
//
extern int sprintf(const char *str, const char *format, ...);
extern int snprintf(const char *str, size_t size, const char *format, ...);
#endif
extern void perror(const char *s);


// PC platform uses NO timers
#define DISABLE_INTS() // nothing
#define ENABLE_INTS()  // nothing

#define ALARM_TIMER_INTERRUPT() void alarmTimerInterrupt(void)
#define SLEEP_TIMER_INTERRUPT() void sleepTimerInterrupt(void)

#define ALARM_TIMER_START()
// #define ENABLE_ALARM_INTERRUPT()
// #define DISABLE_ALARM_INTERRUPT()
#define ALARM_TIMER_EXPIRED() (1)
#define ALARM_TIMER_READ() 0
#define SET_NEXT_ALARM_TIMER(value)

#define ALARM_TIMER_WRAPAROUND() false
#define ALARM_TIMER_RESET_WRAPAROUND()

#define SLEEP_TIMER_STOP()
#define SLEEP_TIMER_EXPIRED() (1)
#define SLEEP_TIMER_READ() 0

#define PLATFORM_CAN_SLEEP() (1)
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
    PLATFORM_MIN_SLEEP_MS = 1, // min sleep amount = 1ms
    PLATFORM_MAX_SLEEP_MS = 0xffff,
    PLATFORM_MAX_SLEEP_SECONDS = PLATFORM_MAX_SLEEP_MS / 1000,
    PLATFORM_ALARM_TIMER_PERIOD = 1,
};

#define TIMER_TICKS_TO_MS(ticks) ticks

#define JIFFY_TIMER_MS 1

#define SLEEP_CYCLES        1
#define SLEEP_CYCLES_DEC    0
#define SLEEP_CLOCK_SPEED   1
#define SLEEP_CLOCK_DIVIDER 1

#define ATOMIC_START(x) (x) = 0
#define ATOMIC_END(x)   ((void) x)

// LEDs: all present! Defined in pc/ledslist.h

// number of USARTs
#define SERIAL_COUNT 1
// use the only "USART" for PRINTF
#define PRINTF_SERIAL_ID 0

// SD card ID
#define SDCARD_SPI_ID 0

#endif
