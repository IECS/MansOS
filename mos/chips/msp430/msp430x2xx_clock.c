/*
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

//
// msp430x2xx series clock calibration
//

#include "msp430_clock.h"
#include "msp430_timers.h"
#include "msp430_int.h"
#include <delay.h>

// DCO calibration approach is borrowed from TOS1.x
enum
{
    TARGET_DCO_DELTA = CPU_HZ / ACLK_SPEED,
};

static uint16_t test_calib_busywait_delta(uint16_t calib)
{
    uint16_t prev = 0;
    uint16_t curr = 0;

    // set DCO calibration
    BCSCTL1 = (BCSCTL1 & ~0x07) | (calib >> 8);
    DCOCTL = calib & 0xff;

    // wait for clock to stabilize
    udelay(100);

    TBCCTL6 &= ~CCIFG;
    while (!(TBCCTL6 & CCIFG));
    prev = TBCCR6;

    TBCCTL6 &= ~CCIFG;

    // wait for next capture and calculate difference
    while (!(TBCCTL6 & CCIFG));
    curr = TBCCR6;

    return curr - prev;
}

//-----------------------------------------------------------
//      Calibrate the DCO and the clock(s)
//-----------------------------------------------------------
static void msp430CalibrateDCO(void)
{
    uint16_t calib;
    uint16_t step;

    // Select SMCLK clock in UP mode, and capture on ACLK for TBCCR6
    TBCTL = TBSSEL_SMCLK | MC1;
    TBCCTL6 = CCIS0 + CM0 + CAP;

    BCSCTL1 = 0x8D;

    // Binary search for DCO calibration
    for (calib = 0xfff, step = 0x800; step != 0; step >>= 1)
    {
        // if the step is not past the target, commit it
        if (test_calib_busywait_delta(calib & ~step) >= TARGET_DCO_DELTA) {
            calib &= ~step;
        }
    }

    // Set DCO calibration
    BCSCTL1 = (BCSCTL1 & ~0x07) | (calib >> 8);
    DCOCTL = calib & 0xff;
}

static void msp430InitClocks(void)
{
#if USE_HARDWARE_TIMERS
    // reset timers
    TACTL = TACLR;
    TBCTL = TBCLR;

    msp430CalibrateDCO();

    // no need to disable oscillator fault NMI: LFXT1 was configured in LF mode
    // IE1 &= ~OFIE;

    // initialize main timers in system mode
    msp430InitTimerA();
    msp430InitTimerB();
#endif
}

void msp430Init(void)
{
    msp430InitPins();
    msp430InitClocks();
}
