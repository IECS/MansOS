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
// PC simulates ADC, reading values from file
//

#ifndef _ADC_HAL_H_
#define _ADC_HAL_H_

#include <kernel/defines.h>

// emulated ADC channels - the same as on TMote Sky
#define ADC_LIGHT_PHOTOSYNTHETIC 4
#define ADC_LIGHT_TOTAL 5
#define ADC_INTERNAL_TEMPERATURE 10
#define ADC_INTERNAL_VOLTAGE 11

void hplAdcInit();

#define PC_ADC_CHANNEL_COUNT 16
#define adcGetChannelCount() (PC_ADC_CHANNEL_COUNT)
uint16_t hplAdcGetVal();
void hplAdcSetChannel(uint8_t ch);

#define hplAdcOn()
#define hplAdcOff()
#define ADC_INT_HEADER() void noAdcInt()
#define hplAdcEnableInterrupt()
#define hplAdcDisableInterrupt()
#define hplAdcIsBusy() (false)
#define hplAdcStartConversion()
#define hplAdcNotifyValueReady()
/** brief PC does not use ADC interrupts, return value on request */
#define hplAdcIntsUsed() (false)
#define hplAdcUseSupplyRef()

#endif  // _ADC_HAL_H_
