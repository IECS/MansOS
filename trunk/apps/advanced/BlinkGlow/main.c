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

//==========================================================================
// BlinkGlow - glowing LEDs counter
//
// Note: due to high-speed dutycycling of the LEDs, 
// this may not simulate fast on the PC platform
//==========================================================================


#include "stdmansos.h"


#define CNT_LIMIT 8

static int ledold=0;

//-------------------------------------------
//-------------------------------------------
void setLedsGlow(int lednew)
{
    int ledon, ledup, leddn;
    int i0, i1, ii, i3;
    
    //ledold = getLeds();
    ledup = lednew | ~ledold;
    leddn = ~lednew | ledold;
    ledon = lednew | ledold;

    for(i0=0; i0<32; i0++){
        for( i1=0; i1<32; i1++ ){
            ii = ledon;
            if(i1 <= i0) ii &= ledup;
            else ii &= leddn;
            setLeds(ii);
            for(i3=0; i3<100; i3++);
        }
    }
    ledold = lednew;
}

//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    int i = 0;
    while(1){
        if( i >= CNT_LIMIT ) i=0;
        setLedsGlow(i);
        i++;
        msleep(250);
    }
}

