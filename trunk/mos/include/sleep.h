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

#ifndef MANSOS_SLEEP_H
#define MANSOS_SLEEP_H

/// \file
/// Routines for putting the system in low power mode
///

#include <defines.h>
#include <platform.h>

///
/// Milliseconds sleep
///
void msleep(uint16_t milliseconds);

#ifdef PLATFORM_PC
// sleep already defined on PC platform
# include <unistd.h>
#else

///
/// Allow another thread to execute. Equivalent to msleep(0)
///
extern inline void yield(void);

// Implementation
#ifdef USE_THREADS
#include <threads/threads.h>
#else
#define yield() // nothing
#endif

///
/// Sleep for n seconds. The signature is compatible with POSIX sleep()
///
static inline uint16_t sleep(unsigned int seconds)
{
    // 
    // Maximal supported sleeping time is 15984 msec.
    // XXX: we do not account for the time that was spent
    // in the loop and in function calls.
    //
    uint32_t ms = seconds * 1000;
    while (ms > PLATFORM_MAX_SLEEP_MS) {
        ms -= PLATFORM_MAX_SLEEP_MS;
        msleep(PLATFORM_MAX_SLEEP_MS);
    }
    msleep(ms);

    //  return 0 on success to keep this function POSIX-compatible
    return 0;
}

#endif // !PLATFORM_PC

#endif
