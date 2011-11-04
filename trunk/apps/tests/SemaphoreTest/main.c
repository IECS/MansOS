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
#include "scheduler.h"
#include "mos_sem.h"
#include "leds.h"

//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void producer1();
void producer2();
void consumer1();
void consumer2();

static mos_sem_t localSem;

void appMain(void)
{
    mos_sem_init(&localSem, 0);
    defaultThreadCreate(producer1);
    defaultThreadCreate(producer1);
    defaultThreadCreate(producer2);
    defaultThreadCreate(producer2);
    defaultThreadCreate(producer2);
    defaultThreadCreate(consumer1);
    defaultThreadCreate(consumer2);
    defaultThreadCreate(consumer2);
    defaultThreadCreate(consumer2);
    defaultThreadCreate(consumer2);
}

void producer1()
{
    while (1) {
        threadSleep(20);
        mos_sem_post(&localSem);
    }
}

void producer2()
{
    while (1) {
        threadSleep(50);
        mos_sem_post(&localSem);
        toggleGreenLed();
    }
}

void consumer1()
{
    while (1) {
        mos_sem_wait(&localSem);
        threadSleep(10);
        toggleRedLed();
   }
}


void consumer2()
{
    while (1) {
        mos_sem_wait(&localSem);
        threadSleep(20);
        toggleGreenLed();
   }
}
