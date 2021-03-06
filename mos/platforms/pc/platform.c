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

//----------------------------------------------------------
//      Platform HPL code
//----------------------------------------------------------
#include <stdlib.h>
#include <pthread.h>
#include "platform.h"

uint16_t pcAlarmTimerRegister;
uint16_t pcSleepTimerRegister;

// semaphore used for SLEEP implementation - call to SLEEP
// waits on a semaphore which is incremented when wake-up
// must be processed
sem_t sleepSem;

void *alarmIntHandler(void *);

static void loopForever(void) {
    // this is needed to emulate behaviour of microconrolleer compilers:
    // they insert an infinite loop after the end of main()
    for (;;);
}

//----------------------------------------------------------
//      Init the platform as if on cold reset
//----------------------------------------------------------
void initPlatform(void)
{
    mos_sem_init(&sleepSem, 0);

#if USE_ALARMS
    // this is a "specific thread", not part of the scheduler
    // create it even when threads are turned off
    static pthread_t alarmThread;
    pthread_create(&alarmThread, NULL, alarmIntHandler, NULL);
#endif

    atexit(loopForever);
}

