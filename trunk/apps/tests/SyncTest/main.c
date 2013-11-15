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

#include "stdmansos.h"
#include "assert.h"

#define BLINK_INTERVAL (8 * 1024)

Alarm_t blinkTimer;
Alarm_t randTimer;

#define PERIOD 5000

void timerBInit(void) 
{
    // Configure Timer B
    TBCTL = TBCLR;
    // start Timer B
    TBCTL = TBSSEL_1 + MC_CONT + TBIE; // clock = ACLK, continuos, interrupt
}

void onBlinkTimer(void *x)
{
    redLedOn();
    mdelay(100);
    redLedOff();

    uint32_t now = getTimeMs();
    uint32_t untilFrameEnd = BLINK_INTERVAL - now % BLINK_INTERVAL;
    alarmSchedule(&blinkTimer, untilFrameEnd);
}

void onRandTimer(void *x)
{
    alarmSchedule(&randTimer, 100 + randomNumberBounded(100));
}

void appMain(void)
{
    timerBInit();

    alarmInit(&blinkTimer, onBlinkTimer, NULL);
    alarmInit(&randTimer, onRandTimer, NULL);

    alarmSchedule(&blinkTimer, 100);
    alarmSchedule(&randTimer, 101);

    PRINTF("\n*** starting the app ***\n\n");

    for (;;) {
       msleep(1000);
    }
}

ISR(TIMERB1, b1)
{
    volatile uint16_t x = TBIV;
    (void) x;
    PRINTF("interrupt, TAR=%u\n", TAR);
}
