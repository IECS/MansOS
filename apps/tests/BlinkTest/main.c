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

//-------------------------------------------
//      Blink regression test application.
//-------------------------------------------

#include "mansos.h"
#include "leds.h"
#include "scheduler.h"
#include "assert.h"

#define PAUSE 500

//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    while (1)
    {
        static uint_t i;
        // test 1: counter 0-7
        for (i = 0; i < 8; ++i) {
            setLeds(i);
            threadSleep(PAUSE);
        }

        // test 2: all off, then red on/off, then green on/off, finally blue on/off
        setLeds(0);
        threadSleep(PAUSE);
        redLedOn();
        threadSleep(PAUSE);
        redLedOff();
        threadSleep(PAUSE);

        greenLedOn();
        threadSleep(PAUSE);
        greenLedOff();
        threadSleep(PAUSE);

        blueLedOn();
        threadSleep(PAUSE);
        blueLedOff();
        threadSleep(PAUSE);

        // test 3: all on, then blue off, green off, red off
        setLeds(7);
        threadSleep(PAUSE);
        blueLedOff();
        threadSleep(PAUSE);
        greenLedOff();
        threadSleep(PAUSE);
        redLedOff();
        threadSleep(PAUSE);

        // test 4: repeat last two tests with toggle
        toggleRedLed();
        threadSleep(PAUSE);
        toggleRedLed();
        threadSleep(PAUSE);

        toggleGreenLed();
        threadSleep(PAUSE);
        toggleGreenLed();
        threadSleep(PAUSE);

        toggleBlueLed();
        threadSleep(PAUSE);
        toggleBlueLed();
        threadSleep(PAUSE);

        setLeds(7);
        threadSleep(PAUSE);
        toggleBlueLed();
        threadSleep(PAUSE);
        toggleGreenLed();
        threadSleep(PAUSE);
        toggleRedLed();
        threadSleep(PAUSE);

        // test 5: the same with ledNrOn(nr) and ledOff(nr)
        ledNrOn(0);
        threadSleep(PAUSE);
        ledNrOff(0);
        threadSleep(PAUSE);

        ledNrOn(1);
        threadSleep(PAUSE);
        ledNrOff(1);
        threadSleep(PAUSE);

        ledNrOn(2);
        threadSleep(PAUSE);
        ledNrOff(2);
        threadSleep(PAUSE);

        setLeds(7);
        threadSleep(PAUSE);
        ledNrOff(2);
        threadSleep(PAUSE);
        ledNrOff(1);
        threadSleep(PAUSE);
        ledNrOff(0);
        threadSleep(PAUSE);

        // test 6: check that isOn functions work
        setLeds(0);
        ASSERT(!isRedLedOn());
        ASSERT(!isGreenLedOn());
        ASSERT(!isBlueLedOn());
        setLeds(7);
        ASSERT(isRedLedOn());
        ASSERT(isGreenLedOn());
        ASSERT(isBlueLedOn());
    } // EOF while (1)
}

