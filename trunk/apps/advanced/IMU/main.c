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

//------------------------------------------------
// Reads all 8 ADC channels, send to serial port
//------------------------------------------------

#include "stdmansos.h"
#include "dprint.h"

#define enableAdcPin(port, pin) \
    pinAsFunction(port, pin); \
    pinAsInput(port, pin);

// On See-Poga mini mote use pin P4.0
// On Lynx-board use pin P1.7
//#define SENSOR_POWER_PORT 4
//#define SENSOR_POWER_PIN 0
#define SENSOR_POWER_PORT 1
#define SENSOR_POWER_PIN 7

#define enableSensorPower() \
    pinAsOutput(SENSOR_POWER_PORT, SENSOR_POWER_PIN); \
    pinSet(SENSOR_POWER_PORT, SENSOR_POWER_PIN);

#define ADC_CH_COUNT 3

//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    // Use AVCC and AVSS as voltage reference
    hplAdcUseSupplyRef();
    hplAdcOn();

    enableAdcPin(6, 0);
    enableAdcPin(6, 1);
    enableAdcPin(6, 2);
    enableAdcPin(6, 3);
    enableAdcPin(6, 4);
    enableAdcPin(6, 5);
    enableAdcPin(6, 6);
    enableAdcPin(6, 7);

    // enable sensor board
    enableSensorPower();

    ledOn(); // to know - we are running

    PRINT_INIT(128); 
    uint16_t a[8];
    uint8_t i;

    for (i = 0; i < 8; ++i) {
        a[i] = 0;
    }
    while(1) {
        for (i = 0; i < ADC_CH_COUNT; ++i) {
            adcSetChannel(i);
            a[i] = adcReadFast();
        }
        if (ADC_CH_COUNT == 3) {
            PRINTF("%u,%u,%u\n",
                   a[0], a[1], a[2]);
        } else if (ADC_CH_COUNT == 8) {
            PRINTF("%u,%u,%u,%u,%u,%u,%u,%u\n",
                   a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7]);
        }
    }
}

