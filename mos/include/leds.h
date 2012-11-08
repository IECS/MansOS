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

#ifndef MANSOS_LEDS_H
#define MANSOS_LEDS_H

#include <kernel/defines.h>
#include "digital.h"

// NOTE: a platform must define ledslist.h for listing all the leds 
//      and implementation specific details. See ledslist.h for the details
//      The ledslist.h file is included several times: 
//      once to define LEDs and once or more to iterate through the LEDs.

// -----------------------------------------------------
// Generic functions for all leds accessed using a bitmask:
// -----------------------------------------------------
void ledsInit(void);               // Init All leds (kernel uses this)

void ledsSet(uint_t led_bitmask);  // Set LEDs from a bitmask
uint_t ledsGet(void);              // Which leds are on? Returns a bitmask

#define ledsOn(led_bitmask)     ledsSet(  (led_bitmask) | ledsGet() )
#define ledsOff(led_bitmask)    ledsSet( ~(led_bitmask) & ledsGet() )
#define ledsToggle(led_bitmask) ledsSet(  (led_bitmask) ^ ledsGet() )


//=========================================================
// LED definition (uses platform dependant ledslist.h)
//=========================================================

//----------------------------------------------------------
// LED_DEFINE defines the functions for each individual led.
// "name" must be the same as used in the leds_t enum declaration.
// "port" and "pin" determine where the led is attached to the MCU.
// "valOn" specifies whether to set pin to 1 or 0 to turn the LED on.
// Note: the defines use double indirection here so that the 
// parameters that are macros get evaluated.
//----------------------------------------------------------

#ifndef LED_DEFINE
#define LED_DEFINE( name, port, pin, valOn )  LED_DEFINE2( name, port, pin, valOn )
#define LED_DEFINE2( name, port, pin, valOn )                            \
    static inline uint_t name##Mask()    { return ((uint_t)(name##_mask)); } \
    static inline uint_t name##Get()     { return (pinRead(port, pin) ^ (1-valOn)); } \
    static inline void name##Set( val )  { pinWrite( port, pin, ((1-valOn) ^ val) ); } \
    static inline void name##On()        { name##Set(1); }              \
    static inline void name##Off()       { name##Set(0); }              \
    static inline void name##Toggle()    { pinToggle( port, pin ); }    \
    static inline void name##Init()      { pinAsOutput( port, pin ); name##Off(); } \
    
#endif  // LED_DEFINE

// LED_ALIAS is used to provide aliases to the original led names 

#ifndef LED_ALIAS
#define LED_ALIAS( alias, name ) LED_ALIAS2( alias, name )
#define LED_ALIAS2( alias, name )                                       \
    enum { alias##_mask = name##_mask };                                \
    static inline uint_t alias##Mask()   { return name##Mask(); }       \
    static inline uint_t alias##Get()    { return name##Mask(); }       \
    static inline void alias##Set( val ) { name##Set( val ); }          \
    static inline void alias##On()       { name##On(); }                \
    static inline void alias##Off()      { name##Off(); }               \
    static inline void alias##Toggle()   { name##Toggle(); }            \
    static inline void alias##Init()     { name##Init(); }              \

#endif // LED_ALIAS     


//----------------------------------------------------------
// Declare all LEDs.
// Prerequisite: ledslist.h file with the LED names.
//----------------------------------------------------------
typedef enum {
#  define DOIT(_led) _led##_num, 
#  include "ledslist.h"
} leds_num_t;

typedef enum {
#  define DOIT(_led) _led##_mask  = (1 << _led##_num),
#  include "ledslist.h"
} leds_t;


// Implementation of pin-attached LEDs, defined in ledslist.h
//----------------------------------------------------------
#define  LEDS_DEFINE
#include "ledslist.h"


// Count all the LEDs to LEDS_COUNT
typedef enum { 
   LEDS_COUNT = 0
#  define DOIT(_led)  +1
#  include "ledslist.h"
} leds_count_t;  // Terminating the LEDS_COUNT construction


// Compute a mask for all the LEDs to LEDS_ALL_MASK
typedef enum { 
   LEDS_ALL_MASK = 0
#  define DOIT(_led)  | _led##_mask
#  include "ledslist.h"
} leds_all_t;    // Terminating the LEDS_ALL_MASK construction


//----------------------------------------------------------
// The default LED "led" support. 
// If LED_DEFAULT is not defined in ledslist.h  - (no leds are present),
// then ignore the led* calls.
//----------------------------------------------------------
#ifdef LED_DEFAULT
LED_ALIAS(led, LED_DEFAULT)
#else // LED_DEFAULT
#warning "Default LED not defined !"
#define ledMask()  0
#define ledOn()
#define ledOff()
#define ledToggle()
#define ledSet(_)
#define ledGet()   0
#define ledInit()
#endif // LED_DEFAULT


//===============================================================
// Compilation support for some default leds
#ifndef RED_LED_DEFINED
#define redLed_mask 0
#define redLedMask()  0
#define redLedOn()
#define redLedOff()
#define redLedToggle()
#define redLedSet(_)
#define redLedGet()   0
#endif

#ifndef GREEN_LED_DEFINED
#define greenLed_mask 0
#define greenLedMask()  0
#define greenLedOn()
#define greenLedOff()
#define greenLedToggle()
#define greenLedSet(_)
#define greenLedGet()   0
#endif

#ifndef BLUE_LED_DEFINED
#define blueLed_mask 0
#define blueLedMask()  0
#define blueLedOn()
#define blueLedOff()
#define blueLedToggle()
#define blueLedSet(_)
#define blueLedGet()   0
#endif

#ifndef YELLOW_LED_DEFINED
#define yellowLed_mask 0
#define yellowLedMask()  0
#define yellowLedOn()
#define yellowLedOff()
#define yellowLedToggle()
#define yellowLedSet(_)
#define yellowLedGet()   0
#endif

//===============================================================
// This should be moved to the PC platform.h
#if PLATFORM_PC
   void suppressLedOutput(bool yes);
#else
#  define suppressLedOutput(_)
#endif

#endif
