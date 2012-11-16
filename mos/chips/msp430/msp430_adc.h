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

#ifndef _MSP430_ADC_H_
#define _MSP430_ADC_H_

#include <kernel/defines.h>
#include <digital.h>

//===========================================================
// Defines and macros
//===========================================================
// in case of missing defines
#ifndef SHT0_DIV128
#define SHT0_DIV128 SHT0_6
#endif // !SHT0_DIV128

#ifndef SHT0_DIV4
#define SHT0_DIV4 SHT0_0
#endif // !SHT0_DIV4

#ifndef ADC12SSEL_ACLK
#define ADC12SSEL_ACLK ADC12SSEL_1
#endif // !ADC12SSEL_ACLK

#ifndef SREF_AVCC_AVSS
/* VR+ = AVCC and VR- = AVSS */
#define SREF_AVCC_AVSS SREF_0
#endif // !SREF_AVCC_AVSS

#ifndef SREF_VREF_AVSS
/* VR+ = VREF+ and VR- = AVSS */
#define SREF_VREF_AVSS SREF_1
#endif // !SREF_VREF_AVSS

#ifndef SHT0_0
#define SHT0_0 ADC12SHT00
#define SHT0_1 ADC12SHT01
#define SHT0_2 ADC12SHT02
#define SHT0_3 ADC12SHT03
#define SHT1_0 ADC12SHT10
#define SHT1_1 ADC12SHT11
#define SHT1_2 ADC12SHT12
#define SHT1_3 ADC12SHT13
#endif

#if !defined ENC && !defined __IAR_SYSTEMS_ICC__
#define ENC         ADC12ENC
#define SHP         ADC12SHP
#define CSTARTADD_2 ADC12CSTARTADD2
#define SREF_1      ADC12SREF_1
#endif

// pin to which ADC0 is connected. Used for external VRef
#define ADC0_PORT 6
#define ADC0_PIN 0

//===========================================================
// Data types and constants
//===========================================================
#define ADC_INTERNAL_TEMPERATURE 10
#define ADC_INTERNAL_VOLTAGE 11

//===========================================================
// Procedures
//===========================================================

/* 1.5V reference voltage, clock divided by 128 */
/* SAMPCON is sourced from sampling timer */
/* use ACLK for ADC12CLK */
/* We will be working with memory register 2, so let's configure it */
/* start conversion in memory reg 2 */
/* VR+ = VREF+ and VR- = AVSS */
#define hplAdcInit() \
    ADC12CTL0 = SHT0_DIV4;                      \
    ADC12CTL1 = SHP;                            \
    ADC12CTL1 |= ADC12SSEL_ACLK;                \
    ADC12CTL1 |= CSTARTADD_2;                   \
    ADC12MCTL2 = SREF_VREF_AVSS

// Use VeREF
#define hplAdcUseExtVRef() \
    pinAsInput(ADC0_PORT, ADC0_PIN); \
    ADC12MCTL2 &= ~(SREF_VREF_AVSS); \
    ADC12MCTL2 |= SREF_VEREF_AVSS;

// Use 2.5V Internal voltage reference instead of 1.5V
#define hplAdcUse2V5VRef() \
    ADC12CTL0 |= REF2_5V

// Use MCU supply voltage (VCC & GND) as voltage reference
// Set SREFx bits to 0 (keep other bits untouched)
#define hplAdcUseSupplyRef() \
    ADC12MCTL2 &= ~(SREF_7);

#define hplAdcOn()    \
    /* turn on reference generator */           \
    ADC12CTL0 |= REFON;                         \
    /* turn on ADC12 */                         \
    ADC12CTL0 |= ADC12ON;                       \
    /* enable conversion to take place */       \
    ADC12CTL0 |= ENC

#define hplAdcOff()    \
    /* turn off conversions */          \
    ADC12CTL0 &= ~ENC;                  \
    /* turn off reference voltage */    \
    ADC12CTL0 &= ~REFON;                \
    /* turn off adc core */             \
    ADC12CTL0 &= ~ADC12ON

#define ADC_INT_HEADER() interrupt (ADC_VECTOR) adcInt()

// why not use MEM2 as buffer area?
#define hplAdcGetVal() (ADC12MEM2)

#define adcGetChannelCount() (16)

// channel held in four smallest bits
#define hplAdcSetChannel(ch) ADC12MCTL2 = (ADC12MCTL2 & 0xf0) | (ch)

// enable/disable interrupt for mem area 2
#define hplAdcEnableInterrupt() ADC12IE |= (1 << 2)
#define hplAdcDisableInterrupt() ADC12IE &= ~(1 << 2)

#define hplAdcIntsUsed() (ADC12IE & (1 << 2))

#define hplAdcIsBusy() (ADC12CTL1 & ADC12BUSY)

#define hplAdcNotifyValueReady()

#define hplAdcStartConversion() ADC12CTL0 |= ADC12SC;

#endif  // !_MSP430_ADC_H_
