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

//===========================================================================
// 3D Accelerometer - Dice demo app
// Report Dice status
//===========================================================================

#include "stdmansos.h"
#include <string.h>

//=======================================================================
//      Types
//=======================================================================

typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t z;
} AccelVector3D_t;

//===========================================
//  Constants
//===========================================

// Accelerometer ADC ports
#define ACC_X_PORT 0
#define ACC_Y_PORT 1
#define ACC_Z_PORT 2

// Number of warmup iterations to get the accelerometer center offsets
#define cnt_warmup 256

// Moving average constants: ma1/ma2 * old + new/ma2
#define ma1 4
#define ma2 5


// Precision for accelerometer - drop this many bits
#define accel_prec 8

// Precision for displacement - divide displacement by mm
#define mm 2

int delta = 180;

int16_t curX, curY, curZ;


//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    char *ss;
    uint16_t len, i;

    AccelVector3D_t accel;
    
    int16_t ax0, ay0, az0;
    int16_t ax, ay, az;
    int16_t dx0, dy0, dz0;
    int16_t vx0, vy0, vz0;
    int16_t vx, vy, vz;
    int16_t sx, sy, sz;
    int16_t t=1;

    int32_t ssx, ssy, ssz;

    //code
    curX = curY = curZ = 0;
    ax0 = ay0 = az0 = 0;
    ax = ay = az = 0;
    vx0 = vy0 = vz0 = 0;
    vx = vy = vz = 0;

    dx0 = dy0 = dz0 = accel_prec;
    
    vx=0;
    vy=0;
    vz=0;

    ssx = ssy = ssz = 0;
    
    //Turn the Accelerometer on (P4.0=1)!
    PIN_AS_DATA(4,0);
    PIN_AS_OUTPUT(4,0);
    PIN_SET(4,0);

    adcSetChannel(1);
    ADC12MCTL2 = SREF_AVCC_AVSS;

    for( i=0; i<cnt_warmup; i++){
        accel.x = adcRead(ACC_X_PORT);
        accel.y = adcRead(ACC_Y_PORT);
        accel.z = adcRead(ACC_Z_PORT);
        
        ssx += accel.x;
        ssy += accel.y;
        ssz += accel.z;

        msleep(10);
    }
    ax0 = ssx / cnt_warmup;
    ay0 = ssy / cnt_warmup;
    az0 = ssz / cnt_warmup;
    ax0 >>= dx0;
    ay0 >>= dy0;
    az0 >>= dz0;

    while(1) {
        accel.x = adcRead(ACC_X_PORT);
        accel.y = adcRead(ACC_Y_PORT);
        accel.z = adcRead(ACC_Z_PORT);

        ax = accel.x >> dx0;
        ax = (ax - ax0);
        ay = accel.y >> dy0;
        ay = (ay - ay0);
        az = accel.z >> dz0;
        az = (az - az0);


        vx0 = vx;
        vx = vx0 + ax * t;
        sx = ((vx + vx0) / 2) * t;
        curX += (sx / mm);

        vy0 = vy;
        vy = vy0 + ay * t;
        sy = ((vy + vy0) / 2) * t;
        curY += (sy / mm);

        vz0 = vz;
        vz = vz0 + az * t;
        sz = ((vz + vz0) / 2) * t;
        curZ += (sz / mm);


        PRINTF("%4u %4u %4u", accel.x, accel.y, accel.z);
        PRINTF(" %5d %5d %5d", ax0, ay0, az0);
        PRINTF(" %5d %5d %5d", ax, ay, az);
        PRINTF(" %7d %7d %7d", vx, vy, vz);
        PRINTF(" %7d %7d %7d", sx, sy, sz);
        PRINTF(" %7d %7d %7d", curX, curY, curZ);

        PRINTF("\n");

        redLedToggle();
        msleep(10);
    }
}

