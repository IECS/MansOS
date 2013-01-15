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

#ifndef _ATMEGA_TIMERS_H_
#define _ATMEGA_TIMERS_H_

#include <avr/wdt.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <kernel/defines.h>

#define atmegaWatchdogStop() wdt_disable()

// these must be defined as macros
#define JIFFY_CLOCK_DIVIDER 64
#define SLEEP_CLOCK_DIVIDER 1024

// TODO: test this
#define SLEEP_CLOCK_SPEED   (CPU_HZ / SLEEP_CLOCK_DIVIDER)

enum {
    JIFFY_TIMER_MS = 1000 / TIMER_INTERRUPT_HZ,

    // how many ticks per jiffy
    PLATFORM_ALARM_TIMER_PERIOD = JIFFY_TIMER_MS *
           (CPU_HZ / TIMER_INTERRUPT_HZ / JIFFY_CLOCK_DIVIDER),
    // in 1 jiffy = 1 ms case, 1 jiffy = 16000 / 64 = 250 clock cycles

    PLATFORM_MIN_SLEEP_MS = 1, // min sleep amount = 1ms
    PLATFORM_MAX_SLEEP_MS = 0xffff / (CPU_HZ / 1000 / SLEEP_CLOCK_DIVIDER + 1),
    PLATFORM_MAX_SLEEP_SECONDS = PLATFORM_MAX_SLEEP_MS / 1000,

    // clock cycles in every sleep ms: significant digits and decimal part
    SLEEP_CYCLES = (CPU_HZ / 1000 / SLEEP_CLOCK_DIVIDER),
    SLEEP_CYCLES_DEC = (CPU_HZ / SLEEP_CLOCK_DIVIDER) % 1000,

    TICKS_IN_MS = CPU_HZ / 1000 / JIFFY_CLOCK_DIVIDER,
};

// XXX: this is slow
#define TIMER_TICKS_TO_MS(ticks) ((ticks) / TICKS_IN_MS)

// bits for clock divider setup
#define TIMER0_DIV_1 (1 << CS00)
#define TIMER0_DIV_8 (1 << CS01)
#define TIMER0_DIV_64 (1 << CS01) | (1 << CS00)
#define TIMER0_DIV_256 (1 << CS02)
#define TIMER0_DIV_1024 (1 << CS02) | (1 << CS00)

#define TIMER1_DIV_1 (1 << CS10)
#define TIMER1_DIV_8 (1 << CS11)
#define TIMER1_DIV_64 (1 << CS11) | (1 << CS10)
#define TIMER1_DIV_256 (1 << CS12)
#define TIMER1_DIV_1024 (1 << CS12) | (1 << CS10)

#if JIFFY_CLOCK_DIVIDER == 1
#define TIMER0_DIVIDER_BITS TIMER0_DIV_1
#elif JIFFY_CLOCK_DIVIDER == 8
#define TIMER0_DIVIDER_BITS TIMER0_DIV_8
#elif JIFFY_CLOCK_DIVIDER == 64
#define TIMER0_DIVIDER_BITS TIMER0_DIV_64
#elif JIFFY_CLOCK_DIVIDER == 256
#define TIMER0_DIVIDER_BITS TIMER0_DIV_256
#elif JIFFY_CLOCK_DIVIDER == 1024
#define TIMER0_DIVIDER_BITS TIMER0_DIV_1024
#endif

#if SLEEP_CLOCK_DIVIDER == 1
#define TIMER1_DIVIDER_BITS TIMER1_DIV_1
#elif SLEEP_CLOCK_DIVIDER == 8
#define TIMER1_DIVIDER_BITS TIMER1_DIV_8
#elif SLEEP_CLOCK_DIVIDER == 64
#define TIMER1_DIVIDER_BITS TIMER1_DIV_64
#elif SLEEP_CLOCK_DIVIDER == 256
#define TIMER1_DIVIDER_BITS TIMER1_DIV_256
#elif SLEEP_CLOCK_DIVIDER == 1024
#define TIMER1_DIVIDER_BITS TIMER1_DIV_1024
#endif

#define atmegaStartTimer0() TCCR0B |= TIMER0_DIVIDER_BITS
#define atmegaStopTimer0() TCCR0B &= ~(TIMER0_DIVIDER_BITS)

#define atmegaStartTimer1() TCCR1B |= TIMER1_DIVIDER_BITS
#define atmegaStopTimer1() TCCR1B &= ~(TIMER1_DIVIDER_BITS)

#define atmegaTimersInit() \
    atmegaInitTimer0(); \
    atmegaInitTimer1();

#define atmegaInitTimer0() \
    /* disable timer power saving */ \
    power_timer0_enable(); \
    /* CTC mode, compare with OCRxA */ \
    /* no clock source at the moment - timer not running */ \
    TCCR0A = (1 << WGM01); \
    /* set time slice to jiffy (1ms) */ \
    OCR0A = PLATFORM_ALARM_TIMER_PERIOD - 1; \
    /* Enable Compare-A interrupt */ \
    TIMSK0 = (1 << OCIE0A); \
    /* reset counter */ \
    TCNT0 = 0;


#define atmegaInitTimer1() \
    /* disable timer power saving */ \
    power_timer1_enable(); \
    /* CTC mode, compare with OCRxA */ \
    /* no clock source at the moment - timer not running */ \
    TCCR1A = 0; \
    TCCR1B = (1 << WGM12); \
    /* do not set ocr yet */ \
    /* Enable Compare-A interrupt */ \
    TIMSK1 = (1 << OCIE1A); \
    /* reset counter */ \
    TCNT1 = 0;

#define ALARM_TIMER_INTERRUPT0() ISR(TIMER0_COMPA_vect)
#define ALARM_TIMER_INTERRUPT1() ISR(TIMER0_COMPB_vect)

#define ALARM_TIMER_INIT() atmegaInitTimer0()
#define ALARM_TIMER_START() atmegaStartTimer0()
#define ALARM_TIMER_STOP() atmegaStopTimer0()
#define SLEEP_TIMER_INIT() atmegaInitTimer1()
#define SLEEP_TIMER_START() atmegaStartTimer1()
#define SLEEP_TIMER_STOP() atmegaStopTimer1()
#define ALARM_TIMER_READ_STOPPED() (TCNT0)
#define ALARM_TIMER_WRAPAROUND() false
#define ALARM_TIMER_RESET_WRAPAROUND()

#define NEXT_ALARM_TIMER(value) OCR0A
#define SET_NEXT_ALARM_TIMER(value) OCR0A += value

// this expands to ALARM_TIMER_READ
ACTIVE_TIMER_READ(ALARM, TCNT0)

#define SLEEP_TIMER_SET(value) OCR1A = (value)

#define SLEEP_TIMER_WRAPAROUND() false
#define SLEEP_TIMER_RESET_WRAPAROUND()

// this expands to SLEEP_TIMER_READ
ACTIVE_TIMER_READ(SLEEP, TCNT1)
#define SLEEP_TIMER_READ_STOPPED(ms) (TCNT1)

// perform sleep/idle
// FIXME: why is power-save mode not working?
#define ENTER_SLEEP_MODE()                      \
    set_sleep_mode(SLEEP_MODE_IDLE);            \
    sleep_enable();                             \
    sei();                                      \
    sleep_mode();                               \
    sleep_disable();

// TODO: enable/disable all pull-ups?

#define PLATFORM_CAN_SLEEP() (1)
// TODO: return 0, when platform cannot go into sleep mode (for example,
//     UART/SPI transmission is in progress

// no action needed to exit idle/sleep mode
#define EXIT_SLEEP_MODE()

// in order to wake from sleep mode the timer must be async
// don't know, what that means, it was in Mantis :)
// but we use different timer setting, so it should do without these routines
#define WAIT_FOR_ASYNC_UPDATE() \
      /* while ((ASSR & (1 << OCR0UB)) || (ASSR & (1 << TCN0UB))); */

// no need to check explicitly
#define ALARM_TIMER_EXPIRED() (1)

// assume that timer compare interrupt occurs only on compare match
#define SLEEP_TIMER_EXPIRED() (1)
#define SLEEP_TIMER_INTERRUPT() SIGNAL(TIMER1_COMPA_vect)


#endif
