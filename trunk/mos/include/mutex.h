/*
 * Copyright (c) 2008-2013 the MansOS team. All rights reserved.
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

#ifndef MANSOS_MUTEX_H
#define MANSOS_MUTEX_H

/// \file
/// Locking primitives for multithreaded applications
///

#if USE_THREADS && !DISABLE_LOCKING

#include "threads.h"

//! MansOS mutex structure
typedef struct Mutex_s {
    // only single value: locked or not
    bool locked;
    // XXX: should be a list in case of multiple threads in the system
    Thread_t *waiter;
} Mutex_t;

///
/// Initialize a mutex in unlocked state
///
static inline void mutexInit(Mutex_t *m)
{
    m->locked = false;
    m->waiter = NULL;
}

///
/// Lock a mutex. If a mutex is already locked, the thread yield()s until it becomes free
///
/// Cannot be called in interrupt context
///
void mutexLock(Mutex_t *m);

///
/// Unlock a mutex. yield()s if there is a waiter on the mutex
///
/// Cannot be called in interrupt context
///
void mutexUnlock(Mutex_t *m);

#else

typedef struct Mutex_s { } Mutex_t;
static inline void mutexInit(Mutex_t *m) {}
static inline void mutexLock(Mutex_t *m) {}
static inline void mutexUnlock(Mutex_t *m) {}

#endif // USE_THREADS

#endif
