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
// msp430x5xx series clock calibration
//

#include "msp430_clock.h"
#include "msp430_timers.h"
#include "msp430_int.h"
#include <delay.h>

static void msp430CalibrateDCO(void) {
    //
    // ACLK = REFO = 32kHz, MCLK = SMCLK = 8MHz
    //

    UCSCTL3 |= SELREF_2;                      // Set DCO FLL reference = REFO
    UCSCTL4 |= SELA_2;                        // Set ACLK = REFO

#ifdef __IAR_SYSTEMS_ICC__
    __bis_SR_register(SCG0);                  // Disable the FLL control loop
#else
    ASM_VOLATILE("bis %0, r2" : : "r" (SCG0));
#endif

    UCSCTL0 = 0x0000;                         // Set lowest possible DCOx, MODx
    UCSCTL1 = DCORSEL_5;                      // Select DCO range 16MHz operation
    UCSCTL2 = FLLD_1 + 249;                   // Set DCO Multiplier for 8MHz
                                              // (N + 1) * FLLRef = Fdco
                                              // (249 + 1) * 32768 = 8MHz
                                              // Set FLL Div = fDCOCLK/2
#ifdef __IAR_SYSTEMS_ICC__
    __bic_SR_register(SCG0);                  // Enable the FLL control loop
#else
    ASM_VOLATILE("bic %0, r2" : : "r" (SCG0));
#endif

    // Worst-case settling time for the DCO when the DCO range bits have been
    // changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
    // UG for optimization.
    // 32 x 32 x 8 MHz / 32,768 Hz = 250000 = MCLK cycles for DCO to settle
//    __delay_cycles(250000);
    udelay(31250);

    // Loop until XT1,XT2 & DCO fault flag is cleared
    do {
        UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + XT1HFOFFG + DCOFFG);
        // Clear XT2,XT1,DCO fault flags
        SFRIFG1 &= ~OFIFG;                      // Clear fault flags
    } while (SFRIFG1 & OFIFG);                  // Test oscillator fault flag
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
