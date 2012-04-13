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

#ifndef _NRF24_TIMERS_H_
#define _NRF24_TIMERS_H_


#define nrf24WatchdogStop() wdt_disable()

// these must be defined as macros
#define JIFFY_CLOCK_DIVIDER 64
#define SLEEP_CLOCK_DIVIDER 1024

#define CPU_SPEED (CPU_MHZ * 1000000ul)

enum {
    JIFFY_TIMER_MS = 1, // jiffy counter signals every 1 millisecond
    // how many ticks per jiffy
    JIFFY_CYCLES = JIFFY_TIMER_MS * (CPU_SPEED / 1000 / JIFFY_CLOCK_DIVIDER),
    // in this case, 1 jiffy = 16000 / 64 = 250 clock cycles

    MIN_SLEEP_MS = 10, // min sleep amount = 10ms
    MAX_SLEEP_MS = 0xffff / (CPU_SPEED / 1000 / SLEEP_CLOCK_DIVIDER + 1),
    MAX_SLEEP_SECONDS = MAX_SLEEP_MS / 1000,

    // clock cycles in every sleep ms: significant digits and decimal part
    SLEEP_CYCLES = (CPU_SPEED / 1000 / SLEEP_CLOCK_DIVIDER),
    SLEEP_CYCLES_DEC = (CPU_SPEED / SLEEP_CLOCK_DIVIDER) % 1000
};

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

#define nrf24StartTimer0() TCCR0B |= TIMER0_DIVIDER_BITS
#define nrf24StopTimer0() TCCR0B &= ~(TIMER0_DIVIDER_BITS)

#define nrf24StartTimer1() TCCR1B |= TIMER1_DIVIDER_BITS
#define nrf24StopTimer1() TCCR1B &= ~(TIMER1_DIVIDER_BITS)

#define nrf24TimersInit() \
    nrf24InitTimer0(); \
    nrf24InitTimer1();

#define nrf24InitTimer0() \
    /* disable timer power saving */ \
    power_timer0_enable(); \
    /* CTC mode, compare with OCRxA */ \
    /* no clock source at the moment - timer not running */ \
    TCCR0A = (1 << WGM01); \
    /* set time slice to jiffy (1ms) */ \
    OCR0A = JIFFY_CYCLES - 1; \
    /* Enable Compare-A interrupt */ \
    TIMSK0 = (1 << OCIE0A); \
    /* reset counter */ \
    TCNT0 = 0;


#define nrf24InitTimer1() \
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

#define ALARM_TIMER_INTERRUPT() ISR(TIMER0_COMPA_vect)

#define ALARM_TIMER_START() nrf24StartTimer0(); blueLedOn();
#define ALARM_TIMER_STOP() nrf24StopTimer0()
#define SLEEP_TIMER_START() nrf24StartTimer1(); greenLedOn();
#define SLEEP_TIMER_STOP() nrf24StopTimer1()

#define ALARM_TIMER_VALUE (TCNT0)
#define RESET_ALARM_TIMER() (TCNT0 = 0)
#define SET_ALARM_OCR_VALUE(value) OCR0A  = value
#define SET_SLEEP_TIMER_VALUE(time) TCNT1 = time
#define SET_SLEEP_OCR_VALUE(value) OCR1A  = value

#define DISABLE_ALARM_INTERRUPT() TIMSK0 &= ~(1 << OCIE0A)
#define ENABLE_ALARM_INTERRUPT() TIMSK0 |= (1 << OCIE0A)

#define DISABLE_SLEEP_INTERRUPT() TIMSK1 &= ~(1 << OCIE1A)
#define ENABLE_SLEEP_INTERRUPT() TIMSK1 |= (1 << OCIE1A)

// perform sleep/idle
// FIXME: why is power-save mode not working?
#define ENTER_SLEEP_MODE() \
    set_sleep_mode(SLEEP_MODE_IDLE); \
    sleep_mode();

#define ENTER_IDLE_MODE() \
    set_sleep_mode(SLEEP_MODE_IDLE); \
    sleep_mode();

// no action needed to exit idle/sleep mode
#define EXIT_IDLE_MODE()
#define EXIT_SLEEP_MODE()
#define EXIT_DEEP_SLEEP()

// in order to wake from sleep mode the timer must be async
// don't know, what that means, it was in Mantis :)
// but we use different timer setting, so it should do without these routines
#define WAIT_FOR_ASYNC_UPDATE() \
      /* while ((ASSR & (1 << OCR0UB)) || (ASSR & (1 << TCN0UB))); */


#define PRE_KERNEL_SLEEP() MCUCR |= (1 << PUD); // disable all pull-ups
#define POST_KERNEL_SLEEP() MCUCR &= ~(1 << PUD); // re-enable the pull-ups

// no need to check explicitly
#define ALARM_TIMER_EXPIRED() (1)

#define platformTurnAlarmsOff() DISABLE_ALARM_INTERRUPT()
#define platformTurnAlarmsOn() ENABLE_ALARM_INTERRUPT()

#define platformTimersInit() nrf24TimersInit()
// reschedule happens automatically
#define platformAlarmsReset()
#define platformAlarmTimerInit() nrf24InitTimer0()
#define platformAlarmTimerStart() nrf24StartTimer0()
#define platformAlarmTimerStop() nrf24StopTimer0()

// sleep timer not defined
#define platformSleepTimerInit() nrf24InitTimer1()
#define platformSleepTimerStart() nrf24StartTimer1()
#define platformSleepTimerStop() nrf24StopTimer1()
void nrf24Timer1Set(uint16_t ms);
#define platformSleepTimerSet(ms) nrf24Timer1Set(ms)
// asuume, that timer compare interrupt occurs only on compare match
#define SLEEP_TIMER_EXPIRED() (1)
#define SLEEP_TIMER_INTERRUPT() SIGNAL(TIMER1_COMPA_vect)


#endif  // _NRF24_TIMERS_h_
