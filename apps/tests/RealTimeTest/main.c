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

//-------------------------------------------
//  RealTimeTest application.
//	Calls getRealTime periodically, displays value on serial
//-------------------------------------------

#include "stdmansos.h"

void appMainx(void);
void appMainLong(void);
void appMainTimesave(void);

void appMain(void) {
    appMainx();
}

//
// Simple time accounting, no tricks
//
void appMainx(void)
{
    for (;;) {
        uint32_t t = getRealTime();
        PRINTF("real time = %lu\n", t);
        mdelay(1000);
    }
}

//
// Demo that time accounting works after 32-bit value wraparound
// (specify USE_LONG_LIFETIME=y in config file)
//
void appMainLong(void)
{
    extern volatile ticks_t jiffies;
    jiffies = ULONG_MAX - 2000;
    for (;;) {
        ticks_t time = getRealTime();
        uint8_t *t = (uint8_t *)&time;
        PRINTF("real time = 0x%02x%02x%02x%02x%02x%02x%02x%02x\n",
                t[7], t[6], t[5], t[4],
                t[3], t[2], t[1], t[0]);
        mdelay(1000);
    }
}


//
// Demo that time accounting works even when interrupts are disabled
//
void appMainTimesave(void)
{
    for (;;) {
        uint32_t t = getRealTime();
        PRINTF("real time = %lu\n", t);
        Handle_t handle;
        ATOMIC_START_TIMESAVE(handle);
        mdelay(1000);
        ATOMIC_END_TIMESAVE(handle);
    }
}


