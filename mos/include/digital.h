/*
 * Copyright (c) 2008-2013 the MansOS team. All rights reserved.
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

#ifndef MANSOS_DIGITAL_H
#define MANSOS_DIGITAL_H

/** \file
  \brief General, platform independent I/O interface.

  Two versions are present: macros in all caps defined in arch layer,
  and macros implemented here that are allowing macro defined symbols
  as parameters for the ports and pins as readable names.

  Usually, the following macros are supported as defined in the arch layer:

    - PORT_AS_OUTPUT( portnum )
    - PORT_AS_INPUT( portnum )
    - PORT_READ( portnum )
    - PORT_WRITE( portnum, val )

    - PIN_AS_OUTPUT( portnum, pinnum )
    - PIN_AS_INPUT( portnum, pinnum )
    - PIN_AS_FUNCTION( portnum, pinnum )
    - PIN_AS_DATA( portnum, pinnum )

    - PIN_READ( portnum, pinnum )
    - PIN_WRITE( portnum, pinnum, val )
    - PIN_SET( portnum, pinnum )
    - PIN_CLEAR( portnum, pinnum )
    - PIN_TOGGLE( portnum, pinnum )

  The following macros relate to GPIO interrupt handling
  and may not be supported by all the platforms:

    - PIN_ENABLE_INT( portnum, pinnum )
    - PIN_DISABLE_INT( portnum, pinnum )
    - PIN_INT_RISING( portnum, pinnum )
    - PIN_INT_FALLING( portnum, pinnum )
    - PIN_IS_INT_RISING( portnum, pinnum )
    - PORT_INT_FLAG( portnum )
    - PIN_READ_INT_FLAG( portnum, pinnum )
    - PIN_CLEAR_INT_FLAG( portnum, pinnum )

  There is a mechanism provided for defining meaningful pin names 
  that are extended with typical pin functions.
  The full list of the named pin functions is as follows:

    - static inline void nameAsOutput()
    - static inline void nameAsInput()
    - static inline uint_t nameRead()
    - static inline void nameWrite(uint8_t val)
    - static inline void nameHi()
    - static inline void nameLow()
    - static inline void nameToggle()

  Usage example: define a pin with name MyEnable on the port 2 pin 3,
  and use it:

    PIN_DEFINE( myEnable, 2, 3 );
    myEnableAsOutput();
    myEnableWrite(1);


 */

#include <gpio_hal.h>

//! Set a digital output pin to 1
#define pinSet( po, pi ) PIN_SET( po, pi )
//! Set a digital output pin using the pin's bitmask
#define pinSetMask( po, mask ) PIN_SET_MASK( po, mask )
//! Clear a digital output pin to 0
#define pinClear( po, pi ) PIN_CLEAR( po, pi )
//! Clear a digital output pin using the pin's bitmask
#define pinClearMask( po, mask ) PIN_CLEAR_MASK( po, mask )
//! Toggle (change the value of) a digital output pin
#define pinToggle( po, pi ) PIN_TOGGLE( po, pi )
//! Toggle a digital output pin using the pin's bitmask
#define pinToggleMask( po, mask ) PIN_TOGGLE_MASK( po, mask )
//! Read the value of a digital input pin. Returns either 1 or 0
#define pinRead( po, pi ) PIN_READ( po, pi )
//! Set a digital pin output to a specific value (logical 0 or 1). Interprets 'val' as a boolean
#define pinWrite( po, pi, val ) PIN_WRITE( po, pi, val )

//! Configure the pin in data output mode (writing possible)
#define pinAsOutput( po, pi ) PIN_AS_OUTPUT( po, pi )
//! Configure the pin in data input mode (reading possible)
#define pinAsInput( po, pi ) PIN_AS_INPUT( po, pi )
//! Configure the pin in data (the default) mode. Reading/writing possible
#define pinAsData( po, pi ) PIN_AS_DATA( po, pi )
//! Configure the pin in function mode. The function depends on the MCU and the pin.
#define pinAsFunction( po, pi ) PIN_AS_FUNCTION( po, pi )

//! Configure the whole port in output mode
#define portAsOutput( po ) PORT_AS_OUTPUT( po )
//! Configure the whole port in input mode
#define portAsInput( po ) PORT_AS_INPUT( po )
//! Read the whole port at once
#define portRead( po ) PORT_READ( po )
//! Write the whole port at once
#define portWrite( po, val ) PORT_WRITE( po, val )

// Interrupts
//! Enable interrupt on a digital pin
#define pinEnableInt( po, pi ) PIN_ENABLE_INT( po, pi )
//! Disable interrupt on a digital pin
#define pinDisableInt( po, pi ) PIN_DISABLE_INT( po, pi )
//! Configure interrupt in rising edge mode
#define pinIntRising( po, pi ) PIN_INT_RISING( po, pi )
//! Configure interrupt in falling edge mode
#define pinIntFalling( po, pi ) PIN_INT_FALLING( po, pi )
//! Check if interrupt is configured in rising edge mode on the pin
#define pinIsIntRising( po, pi ) PIN_IS_INT_RISING( po, pi )
//! Check if interrupt flag is set on a pin
#define pinReadIntFlag( po, pi ) PIN_READ_INT_FLAG( po, pi )
///
/// Clear the interrupt flag on a pin
///
/// Note: until the flag is cleared, no further interrupts can be recieved on the pin.
///
#define pinClearIntFlag( po, pi ) PIN_CLEAR_INT_FLAG( po, pi )

//! Put a pin in data output mode and write a binary value on it
#define digitalWrite(po, pi, val)  do {                \
        pinAsOutput(po, pi);                           \
        pinWrite(po, pi, val);                         \
    } while (0)

//! Put a pin in data input mode and read a binary value from it
#define digitalRead(po, pi) ({                  \
            pinAsInput(po, pi);                 \
            pinRead(po, pi)                     \
    })


//! Define a pin with a custom name. Enable utility functions for it.
#define PIN_DEFINE(name, port, pin) PIN_DEFINE2( name, port, pin )
#define PIN_DEFINE2(name, port, pin)  \
    static inline void name##AsOutput(void)  { pinAsOutput( port, pin ); }  \
    static inline void name##AsInput(void)   { pinAsInput( port, pin ); }   \
    static inline uint_t name##Read(void)     { return (pinRead(port, pin) ? 1:0); } \
    static inline void name##Write(uint8_t val) { pinWrite( port, pin, ((val)?1:0) ); } \
    static inline void name##High(void)      { pinWrite( port, pin, 1); }  \
    static inline void name##Low(void)       { pinWrite( port, pin, 0); }  \
    static inline void name##Toggle(void)    { pinToggle( port, pin ); }   \


// include the platform-specific header
#include "gpio_hal.h"

#endif
