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
// Right now no GPIO for PC platform
//

#ifndef PC_GPIO_HPL_H
#define PC_GPIO_HPL_H

//===========================================================
//  Macros
//===========================================================

void pinSetPc(int port, int pin);
void pinClearPc(int port, int pin);
int pinReadPc(int port, int pin);

#define PORT_AS_OUTPUT( portnum )
#define PORT_AS_INPUT( portnum )

#define PIN_AS_OUTPUT( portnum, pinnum )
#define PIN_AS_INPUT( portnum, pinnum )
#define PIN_AS_FUNCTION( portnum, pinnum )
#define PIN_AS_DATA( portnum, pinnum )

#define PORT_READ( portnum ) (0)
#define PORT_WRITE( portnum, val )

#define PIN_READ( portnum, pinnum ) pinReadPc(portnum, pinnum)

#define PIN_WRITE( portnum, pinnum, val ) \
    if (val) pinSetPc(portnum, pinnum); \
    else pinClearPc(portnum, pinnum);

#define PIN_SET( portnum, pinnum ) pinSetPc(portnum, pinnum)
#define PIN_SET_MASK( portnum, mask )
#define PIN_CLEAR( portnum, pinnum ) pinClearPc(portnum, pinnum)
#define PIN_CLEAR_MASK( portnum, mask )
#define PIN_TOGGLE( portnum, pinnum ) \
    if (pinReadPc(portnum, pinnum)) pinClearPc(portnum, pinnum); \
    else pinSetPc(portnum, pinnum);

// interrupts
#define PIN_ENABLE_INT( portnum, pinnum )
#define PIN_DISABLE_INT( portnum, pinnum )

// catch interrupt on rising/falling edge
#define PIN_INT_RISING( portnum, pinnum )
#define PIN_INT_FALLING( portnum, pinnum )
// check whether pin int is catched on rising edge
#define PIN_IS_INT_RISING( portnum, pinnum )

// int flag is set on interrupt and must be cleared afterwards
#define PORT_INT_FLAG( portnum )
#define PIN_READ_INT_FLAG( portnum, pinnum )
#define PIN_CLEAR_INT_FLAG( portnum, pinnum )

//===========================================================

#endif
