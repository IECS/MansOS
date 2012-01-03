/**
 * Copyright (c) 2008-2012 Leo Selavo and the contributors. All rights reserved.
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

#include "leds.h"

//=========================================================
// Services for multiple LEDs
// referenced by a bitmask.
// Each led should support ledMask() function that identifies it
//=========================================================


//----------------------------------------------------------
void ledsInit(void)
{
#  define DOIT(_led) _led##Init();
#  include "ledslist.h"
}

//----------------------------------------------------------
void ledsSet(uint_t led_bitmask)
{
    suppressLedOutput(true);
#  define DOIT(_led) if(_led##_mask & led_bitmask) _led##On(); else _led##Off();
#  include "ledslist.h"
    suppressLedOutput(false);
}

//----------------------------------------------------------
uint_t ledsGet(void)
{
    uint_t led_bitmask = 0;
#  define DOIT(_led) if( _led##Get() ) led_bitmask |= _led##_mask;
#  include "ledslist.h"
    return led_bitmask;
}
