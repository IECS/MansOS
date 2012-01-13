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
// MansOS Scheduler for PC platforms
// Base taken from Mantis: http://mantis.cs.colorado.edu/
//--------------------------------------------------------------------------------

#ifndef _SCHED_HAL_H_
#define _SCHED_HAL_H_

#include <kernel/defines.h>
#include <pthread.h>

//----------------------------------------------------------
// constants
//----------------------------------------------------------
/** @brief Maximum number of allowed threads */
#define MAX_THREADS 10

#define NUM_PRIORITIES 4 // Total number of possible priorities
#define PRIORITY_IDLE (NUM_PRIORITIES - 1)
#define PRIORITY_NORMAL (NUM_PRIORITIES - 2)
#define PRIORITY_HIGH (NUM_PRIORITIES - 3)
#define PRIORITY_KERNEL 0

//----------------------------------------------------------
// types
//----------------------------------------------------------

/** @brief PC specific Thread structure definition */
typedef struct Thread_s {
    pthread_t pt;
    pthread_cond_t blocker; //used to suspend/resume thread
    pthread_mutex_t blockerMux; //protects blocker
    void (*func)(void);
    struct Thread_s *next;
    uint32_t state;
    uint32_t priority;
} Thread_t;


/** @brief just for compatibility */
typedef uint32_t MemType_t;

#endif  // _SCHED_HAL_H_
