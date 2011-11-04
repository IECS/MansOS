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

#include "leds.h"

// Generic functions:
void ledNrOn(uint_t ledNr)
{
    if (ledNr == RED_LED_NR) {
        redLedOn();
    } else if (ledNr == GREEN_LED_NR) {
        greenLedOn();
    } else if (ledNr == BLUE_LED_NR) {
        blueLedOn();
    }
}

void ledNrOff(uint_t ledNr)
{
    if (ledNr == RED_LED_NR) {
        redLedOff();
    } else if (ledNr == GREEN_LED_NR) {
        greenLedOff();
    } else if (ledNr == BLUE_LED_NR) {
        blueLedOff();
    }
}

void toggleNrLed(uint_t ledNr)
{
    if (ledNr == RED_LED_NR) {
        toggleRedLed();
    } else if (ledNr == GREEN_LED_NR) {
        toggleGreenLed();
    } else if (ledNr == BLUE_LED_NR) {
        toggleBlueLed();
    }
}

uint_t ledNrIsOn(uint_t ledNr)
{
    if (ledNr == RED_LED_NR) {
        return isRedLedOn();
    } else if (ledNr == GREEN_LED_NR) {
        return isGreenLedOn();
    } else if (ledNr == BLUE_LED_NR) {
        return isBlueLedOn();
    } else {
        return 0;
    }
}


// Get and set all LEDs using a bitmask. MAx number of leds depends on
// bit count in uint_t (register size)
uint_t getLeds(void) {
    uint_t l = 0;
    if (isRedLedOn()) l |= RED_LED_BIT;
    if (isGreenLedOn()) l |= GREEN_LED_BIT;
    if (isBlueLedOn()) l |= BLUE_LED_BIT;
    if (isYellowLedOn()) l |= YELLOW_LED_BIT;
    return l;
}

void setLeds(uint_t bitmap) {
    suppressLedOutput(true);

    if (bitmap & RED_LED_BIT) {
        redLedOn();
    } else {
        redLedOff();
    }
    if (bitmap & GREEN_LED_BIT) {
        greenLedOn();
    } else {
        greenLedOff();
    }
    if (bitmap & BLUE_LED_BIT) {
        blueLedOn();
    } else {
        blueLedOff();
    }
    if (bitmap & YELLOW_LED_BIT) {
        yellowLedOn();
    } else {
        yellowLedOff();
    }
    suppressLedOutput(false);
}
