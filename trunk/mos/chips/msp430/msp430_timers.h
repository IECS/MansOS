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

#ifndef _MSP430_TIMERS_H_
#define _MSP430_TIMERS_H_

#include <kernel/defines.h>

#ifndef TASSEL_ACLK
#define TASSEL_ACLK   TASSEL_1
#define TASSEL_SMCLK  TASSEL_2
#define TBSSEL_ACLK   TBSSEL_1
#define TBSSEL_SMCLK  TBSSEL_2

#define MC_UPTO_CCR0  MC_1
#define MC_CONT       MC_2
#define MC_UPDOWN     MC_3

#define ID_DIV1       ID_0
#define ID_DIV2       ID_1
#define ID_DIV4       ID_2
#define ID_DIV8       ID_3

#define SSEL_0        0x00
#define SSEL_1        0x10
#define SSEL_2        0x20
#define SSEL_3        0x30

#define SSEL_ACLK     SSEL_1
#define SSEL_SMCLK    SSEL_2
#endif // TASSEL_ACLK

#ifndef ACLK_SPEED
#define ACLK_SPEED 32768
#endif

// use ACLK by default
#define JIFFY_CLOCK_USE_ACLK 1
#define SLEEP_CLOCK_USE_ACLK 1

#define JIFFY_CLOCK_DIVIDER 1
#define SLEEP_CLOCK_DIVIDER 8

#if JIFFY_CLOCK_USE_ACLK
// use ACLK as source for the jiffy clock
#define JIFFY_CLOCK_SPEED ACLK_SPEED
#else
// use SMCLK as source for the jiffy clock
#define JIFFY_CLOCK_SPEED CPU_HZ
#endif

#if SLEEP_CLOCK_USE_ACLK
// use ACLK as source for the sleep clock
#define SLEEP_CLOCK_SPEED ACLK_SPEED
#else
// use SMCLK as source for the sleep clock
#define SLEEP_CLOCK_SPEED CPU_HZ
#endif

enum {
    JIFFY_TIMER_MS = 1000 / TIMER_INTERRUPT_HZ, // jiffy counter signals every 10 milliseconds
    // round up, because 7>5 and 6>5
    PLATFORM_ALARM_TIMER_PERIOD = ACLK_SPEED / TIMER_INTERRUPT_HZ + 1,

    PLATFORM_MIN_SLEEP_MS = 10, // min sleep amount = 10ms
    PLATFORM_MAX_SLEEP_MS = 0xffff / (SLEEP_CLOCK_SPEED / 1000 / SLEEP_CLOCK_DIVIDER + 1),
    PLATFORM_MAX_SLEEP_SECONDS = PLATFORM_MAX_SLEEP_MS / 1000,

    // clock cycles in every sleep ms: significant digits and decimal part
    SLEEP_CYCLES = (SLEEP_CLOCK_SPEED / 1000 / SLEEP_CLOCK_DIVIDER),
    SLEEP_CYCLES_DEC = (SLEEP_CLOCK_SPEED / SLEEP_CLOCK_DIVIDER) % 1000,

    TICKS_IN_MS = ACLK_SPEED / 1000 + 1,
};

#define TIMER_TICKS_TO_MS(ticks) ((ticks) / TICKS_IN_MS)

//===========================================================
// Macros
//===========================================================

#if defined(__msp430x54xA)
#define TACTL TA0CTL
#define TACCTL0 TA0CCTL0
#define TACCTL1 TA0CCTL1
#define TACCTL2 TA0CCTL2
#define TACCTL3 TA0CCTL3
#define TACCTL5 TA0CCTL5
#define TAR TA0R
#define TACCR0 TA0CCR0
#define TACCR1 TA0CCR1
#define TACCR2 TA0CCR2
#define TACCR3 TA0CCR3
#define TAIV TA0IV
#endif

// count up to CCR0 continuously
//#define msp430StartTimerA() TACTL |= MC_UPTO_CCR0
#define msp430StartTimerA() TACTL |= MC_CONT
#define msp430StopTimerA() TACTL &= ~(MC_3)

#define msp430StartTimerB() TBCTL |= MC_UPTO_CCR0
#define msp430StopTimerB() TBCTL &= ~(MC_3)

// TODO - make register values dynamic (use constants defined above)
#define msp430InitTimerA() \
    /* begin reset/init */ \
    TACTL = TACLR; \
    /* source: 32768Hz ACLK, DIV = 1, interrupt disabled */ \
    TACTL = TASSEL_ACLK | ID_DIV1;  \
    /* clear all other registers */ \
    TAR = 0; \
    TACCTL0 = TACCTL1 = 0; \
    TACCR0 = TACCR1 = 0; \
    /* we want the timer to work every single HZ */ \
    TACCR0 = PLATFORM_ALARM_TIMER_PERIOD; \

#define msp430InitTimerB() \
    /*    TBR = 0; */ \
    /* TBCTL */ \
    /* .TBCLGRP = 0; each TBCL group latched independently */ \
    /* .CNTL = 0; 16-bit counter */ \
    /* .TBSSEL = 1; source ACLK */ \
    /* .ID = 0; input divisor of 1 */ \
    /* .MC = 0; initially disabled */ \
    /* .TBCLR = 0; reset timer B */ \
    /* .TBIE = 1; enable timer B interrupts */ \
    /* TBCTL = TBSSEL0 | TBIE; */ \
    TBCTL = TBCLR; \
    /* src = ACLK, DIV = 8, INT = enabled, 16bit */ \
    TBCTL = TBSSEL_ACLK | ID_DIV8 | TBIE | CNTL_0; \

extern void msp430TimerBSet(uint16_t ms);

// Stop watchdog timer
#define msp430WatchdogStop() WDTCTL = WDTPW + WDTHOLD

// not necessary for msp430
#define PRE_KERNEL_SLEEP()
#define POST_KERNEL_SLEEP()
#define WAIT_FOR_ASYNC_UPDATE()

// Alarm timer
#define ALARM_TIMER_INIT() msp430InitTimerA()
#define ALARM_TIMER_START() msp430StartTimerA()
#define ALARM_TIMER_STOP() msp430StopTimerA()
#define ALARM_TIMER_EXPIRED() (TAIV == 10)
#define ALARM_TIMER_VALUE() (TAR)
#define RESET_ALARM_TIMER() (TAR = 0)
#define SET_NEXT_ALARM_TIMER(value) TACCR0 += value

// TimerA interrupt enable/disable
#define ENABLE_ALARM_INTERRUPT()  TACCTL0 |= CCIE
#define DISABLE_ALARM_INTERRUPT() TACCTL0 &= ~(CCIE)

// Sleep timer
#define SLEEP_TIMER_INIT() msp430InitTimerB()
#define SLEEP_TIMER_START() msp430StartTimerB()
#define SLEEP_TIMER_STOP() msp430StopTimerB()
// #define SET_SLEEP_TIMER_VALUE(time) TBR = time
// #define SET_SLEEP_OCR_VALUE(value) TBCCR0  = value
// make sure the interrupt was triggered
// because of a capture
#define SLEEP_TIMER_EXPIRED() (TBIV == 14)

#define SLEEP_TIMER_SET(ms) msp430TimerBSet(ms)

// TimerB interrupt enable/disable
#define ENABLE_SLEEP_INTERRUPT() TBCTL |= TBIE
#define DISABLE_SLEEP_INTERRUPT() TBCTL &= ~(TBIE)

#ifdef TIMERA0_VECTOR
#define ALARM_TIMER_INTERRUPT() ISR(TIMERA0, alarmTimerInterrupt)
#else // for MSP430F5438
#define ALARM_TIMER_INTERRUPT() ISR(TIMER0_A0, alarmTimerInterrupt)
#endif
#define SLEEP_TIMER_INTERRUPT() ISR(TIMERB1, sleepTimerInterrupt)

#define platformTurnAlarmsOff() DISABLE_ALARM_INTERRUPT()
#define platformTurnAlarmsOn() ENABLE_ALARM_INTERRUPT()

#define platformTimersInit() msp430InitClocks()

//===========================================================
//===========================================================

#endif
