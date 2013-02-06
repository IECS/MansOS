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
// msp430FR57xx series clock calibration
//

#include "msp430_clock.h"
#include "msp430_timers.h"
#include "msp430_int.h"
#include <delay.h>

static void msp430CalibrateDCO(void) {
#if 0  // To run in ~8MHz mode:

  // Startup clock system in max. DCO setting ~8MHz
  // This value is closer to 10MHz on untrimmed parts  
  CSCTL0_H = 0xA5;                          // Unlock register
  CSCTL1 |= DCOFSEL0 + DCOFSEL1;            // Set max. DCO setting
  CSCTL2 = SELA_1 + SELS_3 + SELM_3;        // set ACLK = vlo; MCLK = DCO
  CSCTL3 = DIVA_0 + DIVS_0 + DIVM_0;        // set all dividers 
  CSCTL0_H = 0x01;                          // Lock Register

#endif
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
