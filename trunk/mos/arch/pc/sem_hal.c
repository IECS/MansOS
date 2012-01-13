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

//--------------------------------------------------------------------------------
// MansOS Semaphores for Linux platforms
// Base taken from Mantis: http://mantis.cs.colorado.edu/
//--------------------------------------------------------------------------------

#include "sem_hal.h"
#include <errno.h>
#include <stdio.h>

#ifdef __APPLE__
#include <mach/task.h>
#include <mach/mach.h>
#endif

// TODO: error handling

#ifndef __APPLE__

void mos_sem_init(sem_t *s, int8_t value)
{
    sem_init(s,0,value);
}

void mos_sem_destroy(sem_t *s)
{
    sem_destroy(s);
}

uint8_t sem_try_wait(sem_t *s)
{
    if (sem_trywait(s)) {
        return SEM_FAIL;
    } else {
        return SEM_SUCCESS;
    }
}

void mos_sem_wait(sem_t *s)
{
    sem_wait(s);
}

void mos_sem_post(sem_t *s)
{
    sem_post(s);
}

int8_t mos_sem_get_val(sem_t *s)
{
    int val;
    if (sem_getvalue(s, &val)) {
        perror("sem_getvalue");
        return 0;
    }
    return val;
}

#else // __APPLE__ defined

void mos_sem_init(sem_t *s, int8_t value)
{
    semaphore_create(mach_task_self(), s, SYNC_POLICY_FIFO, value);
}

void mos_sem_destroy(sem_t *s)
{
    // sem_destroy(s);
}

uint8_t sem_try_wait(sem_t *s)
{
    mach_timespec_t t; // try to emulate very short period
    t.tv_sec = 0;
    t.tv_nsec = 1;    
    if (semaphore_timedwait(*s, t) == KERN_SUCCESS) {
        return SEM_SUCCESS;
    } else {
        return SEM_FAIL;
    }
}

void mos_sem_wait(sem_t *s)
{
    semaphore_wait(*s);
}

void mos_sem_post(sem_t *s)
{
    semaphore_signal(*s);
}

int8_t mos_sem_get_val(sem_t *s)
{
    return 0; // not implemented
}


#endif
