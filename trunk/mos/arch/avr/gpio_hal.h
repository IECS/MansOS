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

//
// Service macros for atmega ports and pins
// Warning: interrupt and function routines not implemented
//

#ifndef _atmega_ports_h_
#define _atmega_ports_h_

#include  <avr/io.h>

//===========================================================
//  Macros
//===========================================================

// Arduino is using similar port definitions (www.arduino.cc)

#define NOT_A_PORT 0

// Some folks use numbers
#define PORT1DIR DDRA
#define PORT2DIR DDRB
#define PORT3DIR DDRC
#define PORT4DIR DDRD
#define PORT5DIR DDRE
#define PORT6DIR DDRF
#define PORT7DIR DDRG
#define PORT8DIR DDRH
#define PORT9DIR NOT_A_PORT
#define PORT10DIR DDRJ
#define PORT11DIR DDRK
#define PORT12DIR DDRL

#define OPORT1 PORTA
#define OPORT2 PORTB
#define OPORT3 PORTC
#define OPORT4 PORTD
#define OPORT5 PORTE
#define OPORT6 PORTF
#define OPORT7 PORTG
#define OPORT8 PORTH
#define OPORT9 NOT_A_PORT
#define OPORT10 PORTJ
#define OPORT11 PORTK
#define OPORT12 PORTL

#define IPIN1 PINA
#define IPIN2 PINB
#define IPIN3 PINC
#define IPIN4 PIND
#define IPIN5 PINE
#define IPIN6 PINF
#define IPIN7 PING
#define IPIN8 PINH
#define IPIN9 NOT_A_PIN
#define IPIN10 PINJ
#define IPIN11 PINK
#define IPIN12 PINL

// Some folks use letters
#define PORTADIR DDRA
#define PORTBDIR DDRB
#define PORTCDIR DDRC
#define PORTDDIR DDRD
#define PORTEDIR DDRE
#define PORTFDIR DDRF
#define PORTGDIR DDRG
#define PORTHDIR DDRH
#define PORTIDIR NOT_A_PORT
#define PORTJDIR DDRJ
#define PORTKDIR DDRK
#define PORTLDIR DDRL

#define OPORTA PORTA
#define OPORTB PORTB
#define OPORTC PORTC
#define OPORTD PORTD
#define OPORTE PORTE
#define OPORTF PORTF
#define OPORTG PORTG
#define OPORTH PORTH
#define OPORTI NOT_A_PORT
#define OPORTJ PORTJ
#define OPORTK PORTK
#define OPORTL PORTL

// set bit to 1 for output and 0 for input
#define PORT_AS_OUTPUT( portnum )  PORT##portnum##DIR = 0xff;
#define PORT_AS_INPUT( portnum )   PORT##portnum##DIR = 0x00;

#define PIN_AS_OUTPUT( portnum, pinnum ) \
  { PORT##portnum##DIR |= (1 << (pinnum)); }

#define PIN_AS_INPUT( portnum, pinnum ) \
  { PORT##portnum##DIR &= (~(1 << (pinnum))); }

// pins are defined as functions differently on atmegas
// by default they are data GPIO pins
#define PIN_AS_FUNCTION( portnum, pinnum )
#define PIN_AS_DATA( portnum, pinnum )


#define PORT_READ( portnum ) IPIN##portnum

#define PORT_WRITE( portnum, val ) OPORT##portnum = val

#define PIN_READ( portnum, pinnum ) \
    ((IPIN##portnum & (1 << (pinnum))) ? 1 : 0)

#define PIN_WRITE( portnum, pinnum, val )       \
  OPORT##portnum =                             \
    ( val ?                                     \
      (OPORT##portnum | (1 << (pinnum))) :       \
      (OPORT##portnum & (~(1 << (pinnum))))      \
    );

#define PIN_SET( portnum, pinnum )              \
      OPORT##portnum |= (1 << (pinnum))

#define PIN_SET_MASK( portnum, mask )           \
      OPORT##portnum |= (mask)

#define PIN_CLEAR( portnum, pinnum )            \
      OPORT##portnum &= (~(1 << (pinnum)))

#define PIN_CLEAR_MASK( portnum, mask )         \
      OPORT##portnum &= ~(mask)

#define PIN_TOGGLE( portnum, pinnum )           \
      OPORT##portnum ^= (1 << (pinnum))

#define PIN_TOGGLE_MASK( portnum, mask )        \
      OPORT##portnum ^= (mask)

// TODO
// interrupts
#if 0
#define PIN_ENABLE_INT( portnum, pinnum )              \
     P##portnum##IE |= (1 << (pinnum))

#define PIN_DISABLE_INT( portnum, pinnum )              \
     P##portnum##IE &= ~(1 << (pinnum))

// catch interrupt on rising/falling edge
#define PIN_INT_RISING( portnum, pinnum )              \
     P##portnum##IES &= ~(1 << (pinnum))

#define PIN_INT_FALLING( portnum, pinnum )              \
     P##portnum##IES |= (1 << (pinnum))

// check whether pin int is catched on rising edge
#define PIN_IS_INT_RISING( portnum, pinnum )              \
     ((P##portnum##IES & (1 << (pinnum))) ? 0 : 1)

// int flag is set on interrupt and must be cleared afterwards
#define PORT_INT_FLAG( portnum )              \
     P##portnum##IFG

#define PIN_READ_INT_FLAG( portnum, pinnum )              \
     P##portnum##IFG & (1 << (pinnum))

#define PIN_CLEAR_INT_FLAG( portnum, pinnum )              \
     P##portnum##IFG &= ~(1 << (pinnum))
#endif // 0

//===========================================================
//===========================================================

#endif  // _atmega_ports_h_
