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

/*
 * Common LED code
 */

#ifndef _MANSOS_LEDS_IMPL_H_
#define _MANSOS_LEDS_IMPL_H_

#include <platform.h>

// ----------------------------------------------------
// Detect, which LEDs are defined and define corresponding constants
// Also define LED on/off functions based on the attachment of the LEDs (GND/VCC)


#ifdef LEDS_RED_PORT
#  ifdef LEDS_RED_PIN
#    define RED_LED_PRESENT 1
#  else // !def LEDS_RED_PIN
//#    warning Red LED pin (LEDS_RED_PIN) not defined
#  endif // def LEDS_RED_PIN
#else // !def LEDS_RED_PORT
//#  warning Red LED port (LEDS_RED_PORT) not defined
#endif

#ifdef LEDS_GREEN_PORT
#  ifdef LEDS_GREEN_PIN
#    define GREEN_LED_PRESENT 1
#  else // !def LEDS_GREEN_PIN
//#    warning Green LED pin (LEDS_GREEN_PIN) not defined
#  endif // def LEDS_GREEN_PIN
#else // !def LEDS_GREEN_PORT
//#  warning Green LED port (LEDS_GREEN_PORT) not defined
#endif

#ifdef LEDS_BLUE_PORT
#  ifdef LEDS_BLUE_PIN
#    define BLUE_LED_PRESENT 1
#  else // !def LEDS_BLUE_PIN
//#    warning Blue LED pin (LEDS_BLUE_PIN) not defined
#  endif // def LEDS_BLUE_PIN
#else // !def LEDS_BLUE_PORT
//#  warning Blue LED port (LEDS_BLUE_PORT) not defined
#endif

#ifdef LEDS_YELLOW_PORT
#  ifdef LEDS_YELLOW_PIN
#    define YELLOW_LED_PRESENT 1
#  else // !def LEDS_YELLOW_PIN
//#    warning Yellow LED pin (LEDS_YELLOW_PIN) not defined
#  endif // def LEDS_YELLOW_PIN
#else // !def LEDS_YELLOW_PORT
//#  warning Yelow LED port (LEDS_YELLOW_PORT) not defined
#endif

#if LEDS_ON_WITH_HIGH
#  define LED_ON(port, pin) pinSet(port, pin)
#  define LED_OFF(port, pin) pinClear(port, pin)
#else
#  define LED_ON(port, pin) pinClear(port, pin)
#  define LED_OFF(port, pin) pinSet(port, pin)
#endif // LEDS_ON_WITH_HIGH

// ----------------------------------------------------
// define LED macros

#ifdef RED_LED_PRESENT
#define redLedOn()  LED_ON(LEDS_RED_PORT, LEDS_RED_PIN)
#define redLedOff()  LED_OFF(LEDS_RED_PORT, LEDS_RED_PIN)
#define toggleRedLed()  pinToggle(LEDS_RED_PORT, LEDS_RED_PIN)
#define isRedLedOn() \
    (pinRead(LEDS_RED_PORT, LEDS_RED_PIN) == LEDS_ON_WITH_HIGH)
#define initRedLed() \
    pinAsOutput(LEDS_RED_PORT, LEDS_RED_PIN); \
    redLedOff();
#else
#define redLedOn()
#define redLedOff()
#define toggleRedLed()
#define isRedLedOn() 0
#define initRedLed()
#endif

#ifdef GREEN_LED_PRESENT
#define greenLedOn()  LED_ON(LEDS_GREEN_PORT, LEDS_GREEN_PIN)
#define greenLedOff()  LED_OFF(LEDS_GREEN_PORT, LEDS_GREEN_PIN)
#define toggleGreenLed()  pinToggle(LEDS_GREEN_PORT, LEDS_GREEN_PIN)
#define isGreenLedOn() \
    (pinRead(LEDS_GREEN_PORT, LEDS_GREEN_PIN) == LEDS_ON_WITH_HIGH)
#define initGreenLed() \
    pinAsOutput(LEDS_GREEN_PORT, LEDS_GREEN_PIN); \
    greenLedOff();
#else
#define greenLedOn()
#define greenLedOff()
#define toggleGreenLed()
#define isGreenLedOn() 0
#define initGreenLed()
#endif

#ifdef BLUE_LED_PRESENT
#define blueLedOn()  LED_ON(LEDS_BLUE_PORT, LEDS_BLUE_PIN)
#define blueLedOff()  LED_OFF(LEDS_BLUE_PORT, LEDS_BLUE_PIN)
#define toggleBlueLed()  pinToggle(LEDS_BLUE_PORT, LEDS_BLUE_PIN)
#define isBlueLedOn() \
    (pinRead(LEDS_BLUE_PORT, LEDS_BLUE_PIN) == LEDS_ON_WITH_HIGH)
#define initBlueLed() \
    pinAsOutput(LEDS_BLUE_PORT, LEDS_BLUE_PIN); \
    blueLedOff();
#else
#define blueLedOn()
#define blueLedOff()
#define toggleBlueLed()
#define isBlueLedOn() 0
#define initBlueLed()
#endif

#ifdef YELLOW_LED_PRESENT
#define yellowLedOn()  LED_ON(LEDS_YELLOW_PORT, LEDS_YELLOW_PIN)
#define yellowLedOff()  LED_OFF(LEDS_YELLOW_PORT, LEDS_YELLOW_PIN)
#define toggleYellowLed()  pinToggle(LEDS_YELLOW_PORT, LEDS_YELLOW_PIN)
#define isYellowLedOn() \
    (pinRead(LEDS_YELLOW_PORT, LEDS_YELLOW_PIN) == LEDS_ON_WITH_HIGH)
#define initYellowLed() \
    pinAsOutput(LEDS_YELLOW_PORT, LEDS_YELLOW_PIN); \
    yellowLedOff();
#else
#define yellowLedOn()
#define yellowLedOff()
#define toggleYellowLed()
#define isYellowLedOn() 0
#define initYellowLed()
#endif

// toggle pins in output mode and turn off all leds
#define initLeds() \
    initRedLed(); \
    initGreenLed(); \
    initBlueLed(); \
    initYellowLed();

#if PLATFORM_PC
void suppressLedOutput(bool yes);
#else
#define suppressLedOutput(_)
#endif

#endif

