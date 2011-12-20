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
// msp430 ports for pins and devices
//

#ifndef _MSP430_PORTS_H_
#define _MSP430_PORTS_H_

#include <kernel/defines.h>

//===========================================================
//  Macros
//===========================================================


#define PORT_AS_OUTPUT( portnum )  P##portnum##DIR = 0xff;
#define PORT_AS_INPUT( portnum )   P##portnum##DIR = 0x00;

#define PIN_AS_OUTPUT( portnum, pinnum )   \
  (P##portnum##DIR |= (1 << (pinnum)))

#define PIN_AS_INPUT( portnum, pinnum )    \
  (P##portnum##DIR &= (~(1 << (pinnum))))

#define PIN_AS_FUNCTION( portnum, pinnum ) \
  (P##portnum##SEL |= (1 << (pinnum)))

#define PIN_AS_DATA( portnum, pinnum )     \
  (P##portnum##SEL &= (~(1 << (pinnum))))



#define PORT_READ( portnum ) P##portnum##OUT

#define PORT_WRITE( portnum, val ) P##portnum##OUT = (val)

#define PIN_READ( portnum, pinnum ) \
    ((P##portnum##IN & (1 << (pinnum))) ? 1 : 0)

#define PIN_WRITE( portnum, pinnum, val )         \
  P##portnum##OUT =                               \
          (val) ?                                 \
          (P##portnum##OUT | (1 << (pinnum))) :   \
          (P##portnum##OUT & (~(1 << (pinnum))))  \

#define PIN_SET( portnum, pinnum )                \
      P##portnum##OUT |= (1 << (pinnum))      

#define PIN_SET_MASK( portnum, mask )             \
      P##portnum##OUT |= (mask)      

#define PIN_CLEAR( portnum, pinnum )              \
      P##portnum##OUT &= (~(1 << (pinnum)))

#define PIN_CLEAR_MASK( portnum, mask )           \
      P##portnum##OUT &= ~(mask)      

#define PIN_TOGGLE( portnum, pinnum )             \
      P##portnum##OUT ^= (1 << (pinnum))    

#define PIN_TOGGLE_MASK( portnum, mask )          \
      P##portnum##OUT ^= (mask)     

// interrupts
#define PIN_ENABLE_INT( portnum, pinnum )         \
      P##portnum##IE |= (1 << (pinnum))     

#define PIN_DISABLE_INT( portnum, pinnum )        \
      P##portnum##IE &= ~(1 << (pinnum))     

// catch interrupt on rising/falling edge
#define PIN_INT_RISING( portnum, pinnum )         \
      P##portnum##IES &= ~(1 << (pinnum))     

#define PIN_INT_FALLING( portnum, pinnum )        \
      P##portnum##IES |= (1 << (pinnum))     

// check whether pin int is catched on rising edge
#define PIN_IS_INT_RISING( portnum, pinnum )      \
      ((P##portnum##IES & (1 << (pinnum))) ? 0 : 1)     

// int flag is set on interrupt and must be cleared afterwards
#define PORT_INT_FLAG( portnum )                  \
      P##portnum##IFG     

#define PIN_READ_INT_FLAG( portnum, pinnum )      \
      P##portnum##IFG & (1 << (pinnum))     

#define PIN_CLEAR_INT_FLAG( portnum, pinnum )     \
      P##portnum##IFG &= ~(1 << (pinnum))     

//===========================================================
//===========================================================

#endif
