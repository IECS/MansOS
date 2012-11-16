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

//
// msp430x1xx clock(s)
//

#include "msp430_clock.h"
#include "msp430_timers.h"
#include "msp430_int.h"

//===========================================================
// Data types and constants
//===========================================================

// DCO calibration approach is borrowed from TOS1.x
enum
{
    ACLK_CALIB_PERIOD = 8,
    ACLK_KHZ = ACLK_SPEED / 1000,
    TARGET_DCO_KHZ = CPU_MHZ * 1024, // prescribe the cpu clock rate in kHz
    TARGET_DCO_DELTA = (TARGET_DCO_KHZ / ACLK_KHZ) * ACLK_CALIB_PERIOD,
};

//===========================================================
// Procedures
//===========================================================

static uint16_t test_calib_busywait_delta(uint16_t calib)
{
    int8_t aclk_count = 2;
    uint16_t dco_prev = 0;
    uint16_t dco_curr = 0;

    // set_dco_calib( calib )
    BCSCTL1 = (BCSCTL1 & ~0x07) | ((calib >> 8) & 0x07);
    DCOCTL = calib & 0xff;

    while (aclk_count-- > 0)
    {
        TBCCR0 = TBR + ACLK_CALIB_PERIOD; // set next interrupt
        TBCCTL0 &= ~CCIFG; // clear pending interrupt
        while ((TBCCTL0 & CCIFG) == 0); // busy wait
        dco_prev = dco_curr;
        dco_curr = TAR;
    }

    return dco_curr - dco_prev;
}

//-----------------------------------------------------------
//      Calibrate the DCO and the clock(s)
//-----------------------------------------------------------
static void msp430CalibrateDCO(void)
{
    uint16_t calib;
    uint16_t step;

    // TimerA source is SMCLK, continuous mode
    TACTL = TASSEL_SMCLK | MC_2;
    // TimerB source is ACLK, continuous mode
    TBCTL = TBSSEL_ACLK | MC_2;
    BCSCTL1 = XT2OFF | RSEL2;
    BCSCTL2 = 0;
    TBCCTL0 = CM_1;

    // Binary search for RSEL, DCO, DCOMOD.
    // It's okay that RSEL isn't monotonic.
    for (calib = 0, step = 0x800; step != 0; step >>= 1)
    {
        // if the step is not past the target, commit it
        if (test_calib_busywait_delta(calib | step) <= TARGET_DCO_DELTA) {
            calib |= step;
        }
    }

    // if DCOx is 7 (0x0e0 in calib),
    // then the 5-bit MODx is not useable, set it to 0
    if ((calib & 0x0e0) == 0x0e0) {
        calib &= ~0x01f;
    }

    // Set DCO calibration
    BCSCTL1 = (BCSCTL1 & ~0x07) | ((calib >> 8) & 0x07);
    DCOCTL = calib & 0xff;
}

static void msp430InitClocks(void)
{
#if USE_HARDWARE_TIMERS
    // reset timers
    TACTL = TACLR;
    TBCTL = TBCLR;

    // BCSCTL1
    // .XT2OFF = 1; disable the external oscillator for SCLK and MCLK
    // .XTS = 0; set low frequency mode for LXFT1
    // .DIVA = 0; set the divisor on ACLK to 1
    // .RSEL, do not modify
    BCSCTL1 = XT2OFF | (BCSCTL1 & (RSEL2 | RSEL1 | RSEL0));

    // BCSCTL2
    // .SELM = 0; select DCOCLK as source for MCLK
    // .DIVM = 0; set the divisor of MCLK to 1
    // .SELS = 0; select DCOCLK as source for SCLK
    // .DIVS = 2; set the divisor of SCLK to 4
    // .DCOR = 0; select internal resistor for DCO
    BCSCTL2 = DIVS1;

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
