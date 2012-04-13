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

#include <platform.h>
#if USE_EXP_THREADS
#include "threads/threads.h"
#endif

#ifndef CUSTOM_TIMER_INTERRUPT_HANDLERS

//
// sleep timer interrupt for the case when threads are not used
//
SLEEP_TIMER_INTERRUPT()
{
    if (!SLEEP_TIMER_EXPIRED()) return;

    // restart alarm interrupts
    ENABLE_ALARM_INTERRUPT();

    // sleep timer should not automatically restart
    SLEEP_TIMER_STOP();

#if USE_EXP_THREADS
    // wake up the current thread
    threadWakeup(currentThread->index, THREAD_RUNNING);
#endif

    // exit low power mode
    EXIT_SLEEP_MODE();
}

#endif // !CUSTOM_TIMER_INTERRUPT_HANDLERS

#ifndef PLATFORM_PC

#if USE_EXP_THREADS
#define MSLEEP_FUNCTION_NAME doMsleep
#else
#define MSLEEP_FUNCTION_NAME msleep
#endif

void MSLEEP_FUNCTION_NAME(uint16_t milliseconds)
{
    // setup sleep timer
    SLEEP_TIMER_SET(milliseconds);
    // start timer B
    SLEEP_TIMER_START();
    // stop timer A
    DISABLE_ALARM_INTERRUPT();
    // enter low power mode 3
    ENTER_SLEEP_MODE();
}

#endif // !PLATFORM_PC
