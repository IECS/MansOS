/**
 * Copyright (c) 2008-2010 Leo Selavo and the contributors. All rights reserved.
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

#ifndef _NRF24_ADC_H_
#define _NRF24_ADC_H_

#include <kernel/defines.h>

//===========================================================
// Data types and constants
//===========================================================
#define ADC_INTERNAL_VOLTAGE 8

//===========================================================
// Procedures
//===========================================================
#define hplAdcInit() nrf24InitAdc()
#define hplAdcOn() ADCSRA |= (1 << ADEN)
#define hplAdcOff() ADCSRA &= ~(1 << ADEN)

#define ADC_INT_HEADER() ISR(ADC_vect)

// important: ADCL must be read before ADCH!
// See note in the datasheet 23.2, page 251
#define hplAdcGetVal() (ADCL | (ADCH << 8))

// use 6 ADC channels
#define adcGetChannelCount() (6)

// channel held in four smallest bits
#define hplAdcSetChannel(ch) ADMUX = (ADMUX & 0xf0) | ch;

// enable/disable interrupt
#define hplAdcEnableInterrupt() ADCSRA |= (1 << ADIE)
#define hplAdcDisableInterrupt() ADCSRA &= ~(1 << ADIE);
#define hplAdcIntsUsed() (ADCSRA & (1 << ADIE))
#define hplAdcIsBusy() (ADCSRA & (1 << ADSC))

#define hplAdcNotifyValueReady()

#define hplAdcStartConversion() ADCSRA |= (1 << ADSC);

// turn off Power Reduction ADC
// use internal 1.1V voltage reference
// sampling_clock = CPU_clock / 128
//FIXME:
#define nrf24InitAdc() \
    power_adc_enable(); \
    ADMUX = (1 << REFS0) | (1 << REFS1); \
    ADCSRA = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

#endif  // !_NRF24_ADC_H_
