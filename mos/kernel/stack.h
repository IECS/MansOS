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

#ifndef MANSOS_STACK_H
#define MANSOS_STACK_H

#include <defines.h>
#include <assert.h>
#if USE_THREADS
#include <kernel/threads/threads.h>
#endif

// something like these are used by compilers (GCC and IAR)
#define KERNEL_STACK_BOTTOM  ((MemoryAddress_t)0x3800)
#define KERNEL_STACK_TOP     ((MemoryAddress_t)0x3900)

#define STACK_ADDR_LOW()                                                \
    (currentThread->index == KERNEL_THREAD_INDEX ?                      \
     KERNEL_STACK_BOTTOM :                                       \
     (MemoryAddress_t)threadStackBuffer + (currentThread->index * THREAD_STACK_SIZE))
 
#define STACK_ADDR_HIGH()                                               \
    (currentThread->index == KERNEL_THREAD_INDEX ?                      \
     KERNEL_STACK_TOP :                                          \
     (MemoryAddress_t)threadStackBuffer + ((currentThread->index + 1) * THREAD_STACK_SIZE))

#if USE_THREADS
#define STACK_GUARD() do {                                         \
        /* declare a stack pointer variable */                     \
        MemoryAddress_t currentSp;                                 \
        /* read the current stack pointer into the variable */     \
        GET_SP(currentSp);                                         \
        /* compare the current stack pointer with stack bottom */  \
        /* and abort in case of overflow */                        \
        ASSERT_NOSTACK(currentSp >= STACK_ADDR_LOW());             \
    } while (0)
#else
#define STACK_GUARD()
#endif

// Check if 'x' is an address that belongs to any of stack memory regions
static inline bool isStackAddress(void *x)
{
    MemoryAddress_t ix = (MemoryAddress_t) x;
    if (ix >= KERNEL_STACK_BOTTOM && ix < KERNEL_STACK_TOP) {
        return true;
    }
#if USE_THREADS
    uint8_t *ux = (uint8_t *) x;
    if (ux >= threadStackBuffer
            && ux < threadStackBuffer + THREAD_STACK_SIZE * NUM_USER_THREADS) {
        return true;
    }
#endif
    return false;
}


#endif // MANSOS_STACK_H
