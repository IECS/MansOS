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

//===========================================================================
// 3D Accelerometer - Dice demo app
// Report Dice status
//===========================================================================

#include "stdmansos.h"
#include <string.h>

//===========================================
//  Constants
//===========================================

// Accelerometer ADC ports
#define ACC_X_PORT 0
#define ACC_Y_PORT 1
#define ACC_Z_PORT 2

int delta = 180;

char *smsg[8] = {
    "NO",
    "TOMORROW",
    "MAYBE",
    "WHATEVER",
    "YES",
    "TAKE A BREAK",
    "Nekantovakj!",
    "7",
};

//=======================================================================
//      Types
//=======================================================================

typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t z;
} AccelVector3D_t;

//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    uint16_t threshold = 2048;
    uint16_t dt = delta;
    //uint16_t xx,yy,zz,stat;
    char *ss;
    uint16_t len;

    AccelVector3D_t accel;

    //code
    radioInit();

    //Turn the Accelerometer on (P4.0=1)!
    PIN_AS_DATA(4,0);
    PIN_AS_OUTPUT(4,0);
    PIN_SET(4,0);

    adcSetChannel(1);
    ADC12MCTL2 = SREF_AVCC_AVSS;

    while(1) {
        accel.x = adcRead(ACC_X_PORT);
        msleep(10);
        accel.y = adcRead(ACC_Y_PORT);
        msleep(10);
        accel.z = adcRead(ACC_X_PORT);
        msleep(10);

        if( accel.x > threshold + dt ) ss = smsg[0];
        else if( accel.x < threshold - dt ) ss = smsg[1];
        else if( accel.y > threshold + dt ) ss = smsg[2];
        else if( accel.y < threshold - dt ) ss = smsg[3];
        else if( accel.z > threshold + dt ) ss = smsg[4];
        else if( accel.z < threshold - dt ) ss = smsg[5];
        else ss = smsg[6];

        PRINTF("X= %4u  Y= %4u  Z= %4u  msg= %s\n",
                accel.x, accel.y, accel.z, ss);


        len = strlen(ss);
        radioSend(ss, len);

        redLedToggle();

        msleep(10);
    }
}

