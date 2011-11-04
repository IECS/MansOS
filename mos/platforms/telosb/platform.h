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

#ifndef _PLATFORM_TELOSB_H_
#define _PLATFORM_TELOSB_H_

#include <msp430/msp430_clock.h>
#include <msp430/msp430_timers.h>
#include <msp430/msp430_int.h>
#include <msp430/msp430_adc.h>

#include "cc2420_pins.h"
#include "hd4478_pins.h"
#include "m25p80_pins.h"
#include "sht_pins.h"

//===========================================================
// Functions
//===========================================================

void initPlatform(void);

//===========================================================
// Data types and constants
//===========================================================

// On TelosB LEDs are attached to port 5, pins 4-6
#define LEDS_RED_PORT 5
#define LEDS_RED_PIN 4

#define LEDS_GREEN_PORT 5
#define LEDS_GREEN_PIN 5

#define LEDS_BLUE_PORT 5
#define LEDS_BLUE_PIN 6

// LEDs attached to VCC on this platform
// to turn LED on, corresponding PIN must be set LOW
#define LEDS_ON_WITH_HIGH 0

// light sensors on telosb
#define ADC_LIGHT_PHOTOSYNTHETIC 4
#define ADC_LIGHT_TOTAL 5

#define EXT_FLASH_CHIP FLASH_CHIP_M25P80

#define RADIO_CHIP RADIO_CHIP_CC2420

#define SNUM_CHIP SNUM_DS2411

#endif
