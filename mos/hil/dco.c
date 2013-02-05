/*
 * Copyright (c) 2013 the MansOS team. All rights reserved.
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

#include <platform.h>
#include <print.h>
#include <alarms.h>

#ifndef DCO_RECALIBRATION_PERIOD
#define DCO_RECALIBRATION_PERIOD 60ul * 1000
#endif

Alarm_t recalibrationAlarm;

static void onDcoAlarm(void *x)
{
    // uint16_t calib;
    // calib = DCOCTL;
    // calib |= (BCSCTL1 & 0x07) << 8;
    // PRINTF("before: %#03x\n", calib);

    hplInitClocks();

    // calib = DCOCTL;
    // calib |= (BCSCTL1 & 0x07) << 8;
    // PRINTF("after:  %#03x\n", calib);

#if USE_ALARMS
    // restart the alarm timer (active only when alarm support is configured in)
    ALARM_TIMER_START();
#endif

    alarmSchedule(&recalibrationAlarm, DCO_RECALIBRATION_PERIOD);
}

void dcoRecalibrationInit(void)
{
    alarmInit(&recalibrationAlarm, onDcoAlarm, NULL);
    alarmSchedule(&recalibrationAlarm, DCO_RECALIBRATION_PERIOD);
}
