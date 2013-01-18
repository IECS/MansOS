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

//==========================================================
//  Before including this file 
//  Define a macro DOIT(xxx) that does something with xxx.
//  The DOIT macro is applied to the each item xxx listed here.
//
//  Note: the DOIT must include ; or , or nothing at the end as appropriate.
//==========================================================
//      Platform: SADmote
//==========================================================

//-------------------------------------------
// LED implementation with individual parameters per led
// Invoke this definition from outside as follows:
//      ...
//      #define LED_DEFINE(led_name, ...) ...
//      ...
//      #define LEDS_DEFINE
//      #include "ledslist.h"
//      ...
//
//      An example for the LED_DEFINE:
//              LED_DEFINE(ledName, port, pin, value_for_led_on) ...
//-------------------------------------------
#ifdef LEDS_DEFINE
#undef LEDS_DEFINE

// LEDs attached to VDD on this platform
// to turn LED on, corresponding PIN must be set HIGH
#define LEDS_ON_PIN_VALUE 1
#define LEDS_PORT_3 3
#define LEDS_PORT_J J

LED_DEFINE(led0,  LEDS_PORT_J, 0, LEDS_ON_PIN_VALUE)
LED_DEFINE(led1,  LEDS_PORT_J, 1, LEDS_ON_PIN_VALUE)
LED_DEFINE(led2,  LEDS_PORT_J, 2, LEDS_ON_PIN_VALUE)
LED_DEFINE(led3,  LEDS_PORT_J, 3, LEDS_ON_PIN_VALUE)
LED_DEFINE(led4,  LEDS_PORT_3, 4, LEDS_ON_PIN_VALUE)
LED_DEFINE(led5,  LEDS_PORT_3, 5, LEDS_ON_PIN_VALUE)
LED_DEFINE(led6,  LEDS_PORT_3, 6, LEDS_ON_PIN_VALUE)
LED_DEFINE(led7,  LEDS_PORT_3, 7, LEDS_ON_PIN_VALUE)

// Define some colored aliases

LED_ALIAS(redLed,     led0)
LED_ALIAS(greenLed,   led1)
LED_ALIAS(blueLed,    led2)
LED_ALIAS(yellowLed,  led3)
LED_ALIAS(orangeLed,  led4)
LED_ALIAS(magentaLed, led5)
LED_ALIAS(whiteLed,   led6)

#define RED_LED_DEFINED    1
#define GREEN_LED_DEFINED  1
#define BLUE_LED_DEFINED   1
#define YELLOW_LED_DEFINED   1

// Should define the default led here!
#define LED_DEFAULT redLed

#endif // LEDS_DEFINE


//-------------------------------------------
// List the items (LEDs) with the DOIT macro here
// Do iteration if DOIT(item) is defined
//-------------------------------------------
#ifdef DOIT

DOIT(led0)
DOIT(led1)
DOIT(led2)
DOIT(led3)
DOIT(led4)
DOIT(led5)
DOIT(led6)
DOIT(led7)

// Reset DOIT for the next use
#undef DOIT

//-------------------------------------------
#endif // ifdef DOIT
