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
/*
 * lib/assert.h -- handle program logic errors
 */

#ifndef MANSOS_ASSERT_H
#define MANSOS_ASSERT_H

#include <kernel/defines.h>
#include <platform.h>
#ifdef USE_THREADS
# include <kernel/threads/threads.h>
#endif

/* Ensure we have a valid stack pointer to call functions */
#ifdef USE_THREADS
#  define RESET_SP() SET_SP(threadStackBuffer + THREAD_STACK_SIZE)
/* It makes sense to define RESET_SP() always, but the above will fail if
 * expthreads are not linked in. It would be nice for <platform.h> to
 * define RESET_SP(), which would allow to drop the expthreads reference in
 * this file.
 */
#else
#  define RESET_SP() ((void)0)
#endif


/* Print a message describing the failure, then panic */
void assertionFailed(const char * restrict msg, const char * restrict file,
                     int line) NORETURN;

/* Panic right away */
void panic(void) NORETURN;

/* If printing is enabled, print a message and panic. Otherwise just do the
 * latter. */
#ifdef USE_PRINT
#  define ASSERT_FAILED(e, file, line) assertionFailed(e, file, line)
#else
#  define ASSERT_FAILED(e, file, line) panic()
#endif

#ifdef USE_ASSERT
#  define ASSERT(e) ((e) ? (void)0 : ASSERT_FAILED(#e, __FILE__, __LINE__))
/* Safer ASSERT() that works if the stack pointer is corrupted */
#  define ASSERT_NOSTACK(e) do if (!(e)) {                             \
                                RESET_SP();                            \
                                ASSERT_FAILED(#e, __FILE__, __LINE__); \
                            } while (0)
#else
#  define ASSERT(e)         ((void)0)
#  define ASSERT_NOSTACK(e) ((void)0)
#endif

#endif
