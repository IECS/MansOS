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

//-------------------------------------------
//  Real-Time Clock chip test, reads and writes ISL1219 RTC chip
//-------------------------------------------
#include "stdmansos.h"
#include <rtc/isl1219/isl1219.h>

// 10:09:08, 11 Dec 2013
const ISL1219_Clock_t date = {
    .second = 8,
    .minute = 9,
    .hour = 10,
    .date = 11,
    .month = 12,
    .year = 13,
}; 

//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    ISL1219_Clock_t readDate;

    PRINTF("testing RTC chip...\n");
    ledOn();

    if (!isl1219Write(&date)) {
        PRINTF("write failed\n");
    }

    if (!isl1219Read(&readDate)) {
        PRINTF("read failed\n");
    }

    if (memcmp(&readDate, &date, sizeof(readDate))) {
        PRINTF("read and write values do not match\n");
    }

    mdelay(100);

    ledOff();
    PRINTF("..done!\n");
}
