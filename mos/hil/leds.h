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

#ifndef _MANSOS_LEDS_H_
#define _MANSOS_LEDS_H_

#include <kernel/defines.h>
#include "leds_impl.h"

// -----------------------------------------------------

// common numbering of leds for all platforms
#define RED_LED_NR 0
#define GREEN_LED_NR 1
#define BLUE_LED_NR 2
#define YELLOW_LED_NR 3

// bits used for leds in bitmask (do not mix with xxx_LED_PIN!)
#define RED_LED_BIT (1 << RED_LED_NR)
#define GREEN_LED_BIT (1 << GREEN_LED_NR)
#define BLUE_LED_BIT (1 << BLUE_LED_NR)
#define YELLOW_LED_BIT (1 << YELLOW_LED_NR)

// -----------------------------------------------------

// Generic functions:
void ledNrOn(uint_t ledNr);
void ledNrOff(uint_t ledNr);
void toggleNrLed(uint_t ledNr);
uint_t ledNrIsOn(uint_t ledNr);

// Get and set all LEDs using a bitmask. MAx number of leds depends on
// bit count in uint_t (register size)
uint_t getLeds(void);
void setLeds(uint_t bitmap);



/*
 * Supported LED functions (defined as macros, therefore not declared):
 *
 * // turn on/off/toggle first available led
 * void ledOn();
 * void ledOff();
 * void toggleLed();
 *
 * // turn led on
 * void redLedOn();
 * void greenLedOn();
 * void blueLedOn();
 * void yellowLedOn();
 *
 * // turn led off
 * void redLedOff();
 * void greenLedOff();
 * void blueLedOff();
 * void yellowLedOff();
 *
 * // toggle led
 * void toggleRedLed();
 * void toggleGreenLed();
 * void toggleBlueLed();
 * void toggleYellowLed();
 *
 * uint_t isRedLedOn(); // 1, when led is on, 0 otherwise
 * uint_t isGreenLedOn();
 * uint_t isBlueLedOn();
 * uint_t isYellowLedOn();
 *
 * void initLeds() - for kernel only
 */

#if RED_LED_PRESENT
// use red as default led
#define ledOn() redLedOn()
#define ledOff() redLedOff()
#define toggleLed() toggleRedLed()
#elif GREEN_LED_PRESENT
// use green as default led
#define ledOn() greenLedOn()
#define ledOff() greenLedOff()
#define toggleLed() toggleGreenLed()
#elif BLUE_LED_PRESENT
#define ledOn() blueLedOn()
#define ledOff() blueLedOff()
#define toggleLed() toggleBlueLed()
// use blue as default led
#elif YELLOW_LED_PRESENT
// use yellow as default led
#define ledOn() yellowLedOn()
#define ledOff() yellowLedOff()
#define toggleLed() toggleYellowLed()
#else
// no leds present
#define ledOn()
#define ledOff()
#define toggleLed()
#endif

enum {
    // When LEDS idx = LEDS_ALL, the operation is applied to all LEDs
    LEDS_ALL = 0xff,
};

#endif // _MANSOS_LEDS_H

