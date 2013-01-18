/*
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
// 3D Accelerometer - Music demo app
// Plays music when rolling the board on y axis: one note in one direction,
// other note in other direction.
// Application designed for LynxNet mote
//===========================================================================

#include "stdmansos.h"
#include "beeper.h"

//===========================================
//  Constants
//===========================================

// Accelerometer ADC ports
#define ACC_X_PORT 0
#define ACC_Y_PORT 1
#define ACC_Z_PORT 2

// On The LynxNet board sensor module must be turned on
#define SENSOR_POWER_PORT 1
#define SENSOR_POWER_PIN 7
#define SENSOR_POWER_ON() \
    pinAsOutput(SENSOR_POWER_PORT, SENSOR_POWER_PIN); \
    pinSet(SENSOR_POWER_PORT, SENSOR_POWER_PIN);

enum {
    // ADC level at which the board is in "silent" position
    Y_ZERO_LEVEL = 1900,
    // how much the level must differ from silence, to play notes
    Y_DELTA = 200,

    Y_NOTE1 = Y_ZERO_LEVEL - Y_DELTA,
    Y_NOTE2 = Y_ZERO_LEVEL + Y_DELTA,

    // Two notes: C7 and E7 (http://www.phy.mtu.edu/~suits/notefreqs.html)
    NOTE1_FREQ = 2093, // C7
    NOTE2_FREQ = 2637  // E7
};


//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    SENSOR_POWER_ON();

    hplAdcUseSupplyRef(); // use Supply power as voltage reference
    beeperInit();

    uint16_t y; // y-axis accelerometer reading
    while(1)
    {
        y = adcRead(ACC_Y_PORT);
        if (y < Y_NOTE1) {
            beeperBeepEx(10, NOTE1_FREQ);
        } else if (y > Y_NOTE2) {
            beeperBeepEx(10, NOTE2_FREQ);
        }
//        PRINTF("X= %u  Y= %u  Z= %u\n", x, y, z);
    }
}

