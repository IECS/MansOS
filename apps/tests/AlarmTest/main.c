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

Alarm_t timer1;
Alarm_t timer2;

void onTimer1(void *x)
{
    static uint8_t counter;

    PRINTF("onTimer1\n");
    redLedToggle();
//    if (++counter == 3) {
//        PRINTF("reregister alarm\n");
//        removeAlarm(&timer2);
//        // when removing an alarm, nextMs is cleared, restore it
//        timer2.nextMsec = timer2.msecs = 200;
//        alarmRegister(&timer2);
//    }
    alarmSchedule(&timer1, 700);
}

void onTimer2(void *x)
{
    PRINTF("onTimer2\n");
    blueLedToggle();
}

void appMain(void)
{
    alarmInit(&timer1, onTimer1, NULL);
    alarmSchedule(&timer1, 700);

    alarmInit(&timer2, onTimer2, NULL);
    alarmSchedule(&timer2, 1000);
}
