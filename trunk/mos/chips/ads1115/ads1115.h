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
 
// Works for ads1113, ads1114 & ads1115
#ifndef MANSOS_ADS1115_H
#define MANSOS_ADS1115_H

#include "i2c_soft.h"
#include "stdmansos.h"

// Command to read ADS1115 register
#define ADS_READ_FLAG 0x91
// Command to write ADS1115 register
#define ADS_WRITE_FLAG 0x90

#define ADS_CONVERSION_REGISTER 0x0
#define ADS_CONFIG_REGISTER 0x1
#define ADS_LO_THRESH_REGISTER 0x2
#define ADS_HI_THRESH_REGISTER 0x3

// Masks are used to set config register values, 
// use them in adsConfig function as 2nd parameter,
// first parameter must be set according to mask, because
// only bits that are set in mask will be written to ads111x config register
#define ADS_OS_MASK 0x8000		// Operational status bit mask
#define ADS_MUX_MASK 0x7000		// Input multiplexer configuration bit mask
#define ADS_PGA_MASK 0xE00		// Programmable gain amplifier configuration bit mask
#define ADS_MODE_MASK 0x100		// Device operating mode bit mask
#define ADS_DR_MASK 0xE0		// Data rate bit mask
#define ADS_COMP_MODE_MASK 0x10 // Comparator mode bit mask
#define ADS_COMP_POL_MASK 0x8	// Comparator polarity bit mask
#define ADS_COMP_LAT_MASK 0x4 	// Latching comparator bit mask
#define ADS_COMP_QUE_MASK 0x3	// Comparator queue and disable bit mask

#define ADS_POWER_DOWN_SINGLE_SHOT_MODE 0x100
#define ADS_BEGIN_SINGLE_CONVERSION 0x8000
#define ADS_DEFAULT_CONFIG 0x583

#define ADS_FIRST_INPUT (0x4 << 12)
#define ADS_SECOND_INPUT (0x5 << 12)
#define ADS_THIRD_INPUT (0x6 << 12)
#define ADS_FORTH_INPUT (0x7 << 12)

#define adsConfig(data, mask)	adsActiveConfig &= ~mask;											\
								adsActiveConfig |= data;											\
								writeAdsRegister(ADS_CONFIG_REGISTER, adsActiveConfig );			\
								adsActiveConfig &= 0x7FFF;

#define adsPowerDownSingleShotMode() adsConfig(ADS_POWER_DOWN_SINGLE_SHOT_MODE, ADS_MODE_MASK)
#define adsContiniousConversionMode() adsConfig(0x0, ADS_MODE_MASK)
#define adsBeginSingleConversion() adsConfig(ADS_BEGIN_SINGLE_CONVERSION, ADS_OS_MASK)

#define adsSelectFirstInput() adsConfig(ADS_FIRST_INPUT, ADS_MUX_MASK)
#define adsSelectSecondInput() adsConfig(ADS_SECOND_INPUT, ADS_MUX_MASK)
#define adsSelectThirdInput() adsConfig(ADS_THIRD_INPUT, ADS_MUX_MASK)
#define adsSelectForthInput() adsConfig(ADS_FORTH_INPUT, ADS_MUX_MASK)
#define adsSelectInput(input) adsConfig(input + ADS_FIRST_INPUT, ADS_MUX_MASK)

uint16_t adsActiveConfig;

#endif	//MANSOS_ADS1115_H

// Write ADS1115 register
bool writeAdsRegister(uint8_t reg, uint16_t val);

// Read ADS1115 register
bool readAdsRegister(uint8_t reg, uint16_t *val);

ISR(PORT2, ads_interrupt);

void adsInit();

bool readAds(uint16_t *val);
