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

#ifndef MANSOS_CONTEXT_SWITCH_H
#define MANSOS_CONTEXT_SWITCH_H

#include <defines.h>

//
// Assembly routines required to perform a context switch on AVR architecture.
//

//
// Prepare state for a later context switch
//
#define CONTEXT_SWITCH_PREAMBLE(threadStart, sp)    \
{                                                   \
    MemoryAddress_t oldSp;                          \
    /* Save current stack pointer */                \
    GET_SP(oldSp);                                  \
    /* Change stack pointer to process stack */     \
    SET_SP(stackAddress);                           \
    /* Push address of start function onto stack */ \
    PUSH_VAR(threadStart);                          \
    /* Push zeroes for initial register values */   \
    uint_t i;                                       \
    for (i = 0; i < 30; i++)                        \
        ASM_VOLATILE("push __zero_reg__");          \
    /* Save adjusted stack pointer */               \
    GET_SP(sp);                                     \
    /* Restore old stack pointer */                 \
    SET_SP(oldSp);                                  \
}

//
// Switch from the current thread to a next one
//
#define SWITCH_THREADS(current, next)           \
    GET_SP(current->sp);                        \
    current = next;                             \
    SET_SP(current->sp);                        \

//
// Store current register values
// Called when entering a function that contains a potential context switch
//
#define SAVE_ALL_REGISTERS()                    \
{                                               \
    ASM_VOLATILE("cli");                        \
    ASM_VOLATILE("push r31");                   \
    ASM_VOLATILE("push r30");                   \
    ASM_VOLATILE("push r29");                   \
    ASM_VOLATILE("push r28");                   \
    ASM_VOLATILE("push r27");                   \
    ASM_VOLATILE("push r26");                   \
    ASM_VOLATILE("push r25");                   \
    ASM_VOLATILE("push r24");                   \
    ASM_VOLATILE("push r23");                   \
    ASM_VOLATILE("push r22");                   \
    ASM_VOLATILE("push r21");                   \
    ASM_VOLATILE("push r20");                   \
    ASM_VOLATILE("push r19");                   \
    ASM_VOLATILE("push r18");                   \
    ASM_VOLATILE("push r17");                   \
    ASM_VOLATILE("push r16");                   \
    ASM_VOLATILE("push r15");                   \
    ASM_VOLATILE("push r14");                   \
    ASM_VOLATILE("push r13");                   \
    ASM_VOLATILE("push r12");                   \
    ASM_VOLATILE("push r11");                   \
    ASM_VOLATILE("push r10");                   \
    ASM_VOLATILE("push r9");                    \
    ASM_VOLATILE("push r8");                    \
    ASM_VOLATILE("push r7");                    \
    ASM_VOLATILE("push r6");                    \
    ASM_VOLATILE("push r5");                    \
    ASM_VOLATILE("push r4");                    \
    ASM_VOLATILE("push r3");                    \
    ASM_VOLATILE("push r2");                    \
    MEMORY_BARRIER();                           \
}

//
// Restore previous register values
// Called before return from a function that potentiallly did a context switch
//
#define RESTORE_ALL_REGISTERS()                 \
{                                               \
    MEMORY_BARRIER();                           \
    ASM_VOLATILE("pop r2");                     \
    ASM_VOLATILE("pop r3");                     \
    ASM_VOLATILE("pop r4");                     \
    ASM_VOLATILE("pop r5");                     \
    ASM_VOLATILE("pop r6");                     \
    ASM_VOLATILE("pop r7");                     \
    ASM_VOLATILE("pop r8");                     \
    ASM_VOLATILE("pop r9");                     \
    ASM_VOLATILE("pop r10");                    \
    ASM_VOLATILE("pop r11");                    \
    ASM_VOLATILE("pop r12");                    \
    ASM_VOLATILE("pop r13");                    \
    ASM_VOLATILE("pop r14");                    \
    ASM_VOLATILE("pop r15");                    \
    ASM_VOLATILE("pop r16");                    \
    ASM_VOLATILE("pop r17");                    \
    ASM_VOLATILE("pop r18");                    \
    ASM_VOLATILE("pop r19");                    \
    ASM_VOLATILE("pop r20");                    \
    ASM_VOLATILE("pop r21");                    \
    ASM_VOLATILE("pop r22");                    \
    ASM_VOLATILE("pop r23");                    \
    ASM_VOLATILE("pop r24");                    \
    ASM_VOLATILE("pop r25");                    \
    ASM_VOLATILE("pop r26");                    \
    ASM_VOLATILE("pop r27");                    \
    ASM_VOLATILE("pop r28");                    \
    ASM_VOLATILE("pop r29");                    \
    ASM_VOLATILE("pop r30");                    \
    ASM_VOLATILE("pop r31");                    \
    ASM_VOLATILE("sei");                        \
}

#endif
