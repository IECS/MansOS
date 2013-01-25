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

//-------------------------------------------
//      Blink regression test application.
//-------------------------------------------

#include "stdmansos.h"
#include <assert.h>

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
            ledsSet(i);
            msleep(PAUSE);
        }

        // test 2: all off, then red on/off, then green on/off, finally blue on/off
        ledsSet(0);
        msleep(PAUSE);
        redLedOn();
        msleep(PAUSE);
        redLedOff();
        msleep(PAUSE);

        greenLedOn();
        msleep(PAUSE);
        greenLedOff();
        msleep(PAUSE);

        blueLedOn();
        msleep(PAUSE);
        blueLedOff();
        msleep(PAUSE);

        // test 3: all on, then blue off, green off, red off
        ledsSet(7);
        msleep(PAUSE);
        blueLedOff();
        msleep(PAUSE);
        greenLedOff();
        msleep(PAUSE);
        redLedOff();
        msleep(PAUSE);

        // test 4: repeat last two tests with toggle
        redLedToggle();
        msleep(PAUSE);
        redLedToggle();
        msleep(PAUSE);

        greenLedToggle();
        msleep(PAUSE);
        greenLedToggle();
        msleep(PAUSE);

        blueLedToggle();
        msleep(PAUSE);
        blueLedToggle();
        msleep(PAUSE);

        ledsSet(7);
        msleep(PAUSE);
        blueLedToggle();
        msleep(PAUSE);
        greenLedToggle();
        msleep(PAUSE);
        redLedToggle();
        msleep(PAUSE);

        // test 5: check that isOn functions work
        ledsSet(0);
        ASSERT(!redLedGet());
        ASSERT(!greenLedGet());
        ASSERT(!blueLedGet());
        ledsSet(7);
        ASSERT(redLedGet());
        ASSERT(greenLedGet());
        ASSERT(blueLedGet());
    } // EOF while (1)
}

