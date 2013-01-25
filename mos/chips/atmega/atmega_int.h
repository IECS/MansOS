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
// MSP430 Interrupts
//----------------------------------------------------------

#ifndef _ATMEGA_INT_H_
#define _ATMEGA_INT_H_

#include <defines.h>

#define ENABLE_INTS() sei()
#define DISABLE_INTS() cli()

#define GET_INTERRUPT_STATUS()       SREG
#define SET_INTERRUPT_STATUS(status) SREG = status

// get stack pointer
#define GET_SP(sp)                                  \
    ASM_VOLATILE(                                   \
            "in %A0, __SP_L__\n\t"                  \
            "in %B0, __SP_H__\n\t"                  \
            : "=r" (sp) : );

// set stack pointer
#define SET_SP(sp)                                  \
    ASM_VOLATILE(                                   \
            "out __SP_H__, %B0\n\t"                 \
            "out __SP_L__, %A0\n\t"                 \
            :: "r" (sp) );
#define SET_SP_IMMED(sp)                            \
    ASM_VOLATILE(                                   \
            "out __SP_H__, %B0\n\t"                 \
            "out __SP_L__, %A0\n\t"                 \
            :: "" (sp) );

// push a 2-byte variable on the stack
#define PUSH_VAR(var)                               \
    ASM_VOLATILE(                                   \
            "push %A0\n\t"                          \
            "push %B0\n\t"                          \
            :: "r" (var) );

//
// Atomic block start and end
//
#define ATOMIC_START(handle) do {               \
        handle = GET_INTERRUPT_STATUS();        \
        DISABLE_INTS();                         \
        MEMORY_BARRIER();                       \
    } while (0)

#define ATOMIC_END(handle) do {                 \
        MEMORY_BARRIER();                       \
        SET_INTERRUPT_STATUS(handle);           \
    } while (0)

#define INTERRUPT_ENABLED_START(handle) do {    \
        handle = GET_INTERRUPT_STATUS();        \
        ENABLE_INTS();                          \
        MEMORY_BARRIER();                       \
    } while (0)

#define INTERRUPT_ENABLED_END(handle) do {      \
        MEMORY_BARRIER();                       \
        SET_INTERRUPT_STATUS(handle);           \
    } while (0);

#endif
