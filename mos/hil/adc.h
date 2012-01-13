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

#ifndef MANSOS_HIL_ADC
#define MANSOS_HIL_ADC

#include <kernel/defines.h>
#include <platform.h>

// ADC channels which SHOULD be defined for each platform.
// When a particular channel is not present on a platform,
// -1 MUST be used for corresponding constant. These constants can be
// also specified in application's config file
#ifndef ADC_LIGHT_TOTAL
#define ADC_LIGHT_TOTAL -1
#endif
#ifndef ADC_LIGHT_PHOTOSYNTHETIC
#define ADC_LIGHT_PHOTOSYNTHETIC -1
#endif
#ifndef ADC_INTERNAL_VOLTAGE
#define ADC_INTERNAL_VOLTAGE -1
#endif
#ifndef ADC_INTERNAL_TEMPERATURE
#define ADC_INTERNAL_TEMPERATURE -1
#endif

#define ADC_LIGHT ADC_LIGHT_TOTAL

// User-level ADC functions
void adcOn(void);
void adcOff(void);
uint16_t adcRead(uint8_t ch);
void adcSetChannel(uint8_t ch);
uint16_t adcReadFast(void);
// channel count defined in platform-specific part
// uint_t adcGetChannelCount();

void initAdc(void) WEAK_SYMBOL; // for kernel only


#endif
