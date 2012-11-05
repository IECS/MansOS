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

#include "stdmansos.h"

static void secondThreadFunction(void);

#define PRIORITY_MAIN   1 // higher
#define PRIORITY_SECOND 0 // lower

#define INTERVAL 32768

void appMain(void)
{
    setPriority(currentThread, PRIORITY_MAIN);

    // create second thread
    threadCreate(1, secondThreadFunction);

    uint16_t nextTime = INTERVAL;
    for (;;) {
        if (timeAfter16(ALARM_TIMER_VALUE(), nextTime)) {
            nextTime += INTERVAL;
            PRINTF("in appMain...\n");
            redLedToggle();
            mdelay(100);
        }
        // a simple yield() will not let the lower priority thread to run!
        msleep(10);
    }
}

void secondThreadFunction(void)
{
    setPriority(currentThread, PRIORITY_SECOND);

    uint16_t nextTime = INTERVAL;
    for (;;) {
        if (timeAfter16(ALARM_TIMER_VALUE(), nextTime)) {
            nextTime += INTERVAL;
            PRINTF("in thread...\n");
            greenLedToggle();
            mdelay(100);
        }
        msleep(10);
    }
}
