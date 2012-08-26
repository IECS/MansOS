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

#ifndef MANSOS_DIGITAL_H
#define MANSOS_DIGITAL_H

/*
  General, platform independant I/O interface.

  Two versions are present: macros in all caps defined in HPL,
  and macros implemented here that are allowing macro defined symbols
  as parameters for the ports and pins as readable names.

  Usually, the following macros are supported as defined in HPL:

#define PORT_AS_OUTPUT( portnum )
#define PORT_AS_INPUT( portnum )
#define PORT_READ( portnum )
#define PORT_WRITE( portnum, val )

#define PIN_AS_OUTPUT( portnum, pinnum )
#define PIN_AS_INPUT( portnum, pinnum )
#define PIN_AS_FUNCTION( portnum, pinnum )
#define PIN_AS_DATA( portnum, pinnum )

#define PIN_READ( portnum, pinnum )
#define PIN_WRITE( portnum, pinnum, val )
#define PIN_SET( portnum, pinnum )
#define PIN_CLEAR( portnum, pinnum )
#define PIN_TOGGLE( portnum, pinnum )

  The following macros relate to GPIO interrupt handling
  and may not be supported by all the platforms

#define PIN_ENABLE_INT( portnum, pinnum )
#define PIN_DISABLE_INT( portnum, pinnum )
#define PIN_INT_RISING( portnum, pinnum )
#define PIN_INT_FALLING( portnum, pinnum )
#define PIN_IS_INT_RISING( portnum, pinnum )
#define PORT_INT_FLAG( portnum )
#define PIN_READ_INT_FLAG( portnum, pinnum )
#define PIN_CLEAR_INT_FLAG( portnum, pinnum )

 */

#define pinSet( po, pi ) PIN_SET( po, pi )
#define pinSetMask( po, pi ) PIN_SET_MASK( po, pi )
#define pinClear( po, pi ) PIN_CLEAR( po, pi )
#define pinClearMask( po, pi ) PIN_CLEAR_MASK( po, pi )
#define pinToggle( po, pi ) PIN_TOGGLE( po, pi )
#define pinToggleMask( po, pi ) PIN_TOGGLE_MASK( po, pi )
#define pinRead( po, pi ) PIN_READ( po, pi )
#define pinWrite( po, pi, val ) PIN_WRITE( po, pi, val )

#define pinAsOutput( po, pi ) PIN_AS_OUTPUT( po, pi )
#define pinAsInput( po, pi ) PIN_AS_INPUT( po, pi )
#define pinAsData( po, pi ) PIN_AS_DATA( po, pi )
#define pinAsFunction( po, pi ) PIN_AS_FUNCTION( po, pi )

#define portAsOutput( po ) PORT_AS_OUTPUT( po )
#define portAsInput( po ) PORT_AS_INPUT( po )
#define portRead( po ) PORT_READ( po )
#define portWrite( po, val ) PORT_WRITE( po, val )

// Interrupts
#define pinEnableInt( po, pi ) PIN_ENABLE_INT( po, pi )
#define pinDisableInt( po, pi ) PIN_DISABLE_INT( po, pi )
#define pinIntRising( po, pi ) PIN_INT_RISING( po, pi )
#define pinIntFalling( po, pi ) PIN_INT_FALLING( po, pi )
#define pinIsIntRising( po, pi ) PIN_IS_INT_RISING( po, pi )
#define pinReadIntFlag( po, pi ) PIN_READ_INT_FLAG( po, pi )
#define pinClearIntFlag( po, pi ) PIN_CLEAR_INT_FLAG( po, pi )

#define digitalWrite(po, pi, value) do {        \
        pinAsOutput(po, pi);                    \
        pinWrite(po, pi, value);                \
    } while (0)
#define digitalRead(po, pi) ({                  \
            pinAsInput(po, pi);                 \
            pinRead(po, pi)                     \
    })

// include the platform-specific header
#include "gpio_hal.h"

#endif
