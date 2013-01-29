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

#ifndef MANSOS_DEFINES_GCC_H
#define MANSOS_DEFINES_GCC_H

/// \file
/// GCC compiler-specific defines and headers
///

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

//! Bit vector
#ifndef BV
#  define BV(x) (1 << (x))
#endif

//! Interrupt Service Routine
#ifndef ISR
#define ISR(vec, name) interrupt(vec ## _VECTOR) name(void)
#endif
#define XXISR(port, func) ISR(PORT ## port, func)
//! Use XISR instead of ISR in case name of the port is a #define
#define XISR(port, func)  XXISR(port, func)

//! Put inline assembler code in the program
#define ASM_VOLATILE(x) __asm__ __volatile__(x)
//! Put a function in .data segment (i.e. in RAM)
#define RAMFUNC __attribute__ ((section (".data")))
//! Put data in .text segment (i.e. in the flash memory). For constant data only.
#define TEXTDATA __attribute__ ((section (".text")))

//! Memory barrier for the compiler
#define MEMORY_BARRIER()                           \
    ASM_VOLATILE("" : : : "memory")

//! Always inline this funcion, even when not optimizing
#define INLINE __attribute__((always_inline))
//! This function does not return
#define NORETURN __attribute__((noreturn))
//! This structure is packed (i.e. aligned to 1 byte, i.e. unaligned)
#define PACKED __attribute__((packed))
#define NO_EPILOGUE // nothing
#undef NAKED
#ifndef PLATFORM_PC
//! A "naked" function has no prologue or epilogue
#define NAKED __attribute__((naked))
#else
#define NAKED
#endif

#define PRINTF_LIKE __attribute__ ((format (printf, 1, 2)))

/// WEAK_SYMBOL attribute tells that the function might be left unresolved by the linker.
///
/// Before callling such a fucntion f, always check if it for NULL, e.g.: if (f != NULL) f()
#ifdef __APPLE__
# define WEAK_SYMBOL  __attribute__ ((weak_import))
#else // !__APPLE__
# define WEAK_SYMBOL  __attribute__ ((weak))
#endif // __APPLE__

#define PRAGMA(x) _Pragma(#x)
//! Print a message during the compilation
#define MESSAGE(x) PRAGMA(message x)

//! Restrict keyword, as in C99 standard
#define restrict __restrict__

#endif
