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

//
// GCC compiler-specific defines and headers
//

#ifndef MANSOS_DEFINES_GCC_H
#define MANSOS_DEFINES_GCC_H

#ifdef MCU_MSP430
#ifdef CYGWIN
#define const_sfrb(x,x_) sfrb(x,x_)
#define const_sfrw(x,x_) sfrw(x,x_)
#endif
#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 5 // for msp430-gcc version 4.5+
#include <msp430.h>
#else
#include <io.h>
#include <signal.h>
#endif
#if !defined(__MSP430_LIBC__) || __MSP430_LIBC__ < 20111008L
#  define interrupt(x) void __attribute__((interrupt (x)))
#endif
#if __GNUC__ >= 4
#include <isr_compat.h>  // defines ISR macro
#endif
#elif defined MCU_AVR
#include <avr/io.h>
#endif

#ifndef BV
#  define BV(x) (1 << (x))
#endif

#ifndef ISR // Interrupt Service Routine syntax
#define ISR(vec, name) interrupt(vec ## _VECTOR) name(void)
#endif
// use XISR instead of ISR if symbolic names are wanted
#define XXISR(port, func) ISR(PORT ## port, func)
#define XISR(port, func)  XXISR(port, func)

#define ASM_VOLATILE(x) __asm__ __volatile__(x)
#define RAMFUNC __attribute__ ((section (".data")))
#define TEXTDATA __attribute__ ((section (".text")))

#define MEMORY_BARRIER()                           \
    ASM_VOLATILE("" : : : "memory")

#define INLINE __attribute__((always_inline))
#define NORETURN __attribute__((noreturn))
#define PACKED __attribute__((packed))
#define NO_EPILOGUE // nothing
#undef NAKED
#ifndef PLATFORM_PC
#define NAKED __attribute__((naked))
#else
#define NAKED
#endif
//#ifndef PLATFORM_PC
# ifdef __APPLE__
#  define WEAK_SYMBOL  __attribute__ ((weak_import))
# else // !__APPLE__
#  define WEAK_SYMBOL  __attribute__ ((weak))
# endif // __APPLE__
//#else
//# define WEAK_SYMBOL // nothing
//#endif

#define PRAGMA(x) _Pragma(#x)
#define MESSAGE(x) PRAGMA(message x)

#define restrict __restrict__

#endif
