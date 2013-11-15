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

#ifndef UDELAY_HAL_H
#define UDELAY_HAL_H

#include <defines.h>

///
/// Delay macros and functions.
///
/// Use udelay(), mdelay(), and clock_cycles() for constant-period delays.
/// Use udelayVariable(), mdelayVariable(), etc. for variable-period delays.
///

#if defined(__IAR_SYSTEMS_ICC__) || \
    (defined(__GNUC__) && __GNUC__ >= 4 && __GNUC_MINOR__ >= 6)

#include "intrinsics.h"
#define udelay(x) __delay_cycles((uint32_t) x * CPU_MHZ)
#define mdelay(x) __delay_cycles((uint32_t) x * CPU_MHZ * 1000ul)
#define clock_delay(x) __delay_cycles((uint32_t) x * 4)

#else // Old GCC

#define udelay(x) udelayVariable(x)
#define mdelay(x) mdelayVariable(x)
#define clock_delay(x) clock_delayVariable(x)

#endif // __delay_cycles provided by the compiler


#include "iomacros.h"

//
// This function takes ~4 CPU cycles for CPU_MHZ <= 4,
// and correspondingly more for faster CPU frequencies.
// It should be safe to optimizations.
// If very short time interval is needed, using NOPs might be a better idea.
//
static inline void clock_delayVariable(uint16_t n) INLINE;
static inline void clock_delayVariable(uint16_t n)
{
    __asm__ __volatile__ (
            "1:\n"
            " nop \n"
#if CPU_MHZ >= 8
            " nop \n"
            " nop \n"
            " nop \n"
            " nop \n"
#if CPU_MHZ >= 12
            " nop \n"
            " nop \n"
            " nop \n"
            " nop \n"
#if CPU_MHZ == 16
            " nop \n"
            " nop \n"
            " nop \n"
            " nop \n"
#else
#error Only clock rates up to 16MHz supported! (for now)
#endif
#endif
#endif
            " dec %[n]\n"
            " jne 1b \n"
            : [n] "+r"(n));
}

#if CPU_MHZ >= 4
#define CPU_MHZ_DIVIDER 1
#else
#define CPU_MHZ_DIVIDER (4 / CPU_MHZ)
#endif

//
// Delay for approximate amount of milliseconds.
// The function is not very precise! If precise delays are required, use timers.
//
static inline void mdelayVariable(uint16_t ms) {
    while (ms > 64) {
        ms -= 64;
        clock_delay(64000 / CPU_MHZ_DIVIDER);
    }
    clock_delayVariable(ms * 1000 / CPU_MHZ_DIVIDER);
}

#if CPU_MHZ == 1
#define NOP_FOR_1US() __delay_cycles(1)
#elif CPU_MHZ == 2
#define NOP_FOR_1US() __delay_cycles(2)
#elif CPU_MHZ == 4
#define NOP_FOR_1US() __delay_cycles(4)
#elif CPU_MHZ == 8
#define NOP_FOR_1US() __delay_cycles(8)
#elif CPU_MHZ == 16
#define NOP_FOR_1US() __delay_cycles(16)
#else
#error Only 1, 2, 4, 8 and 16 MHz clock rates supported! (for now)
#endif

//
// Delay for specified amount of microseconds.
// This works only for delays under 64k usec under 4Mhz CPU!
// Use mdelay() or msleep() and sleep() for larger delays.
//
static inline void udelayVariable(uint16_t us) {
    if (__builtin_constant_p(us) && us < 6) {
        switch (us) {
        case 5:
            NOP_FOR_1US();
        case 4:
            NOP_FOR_1US();
        case 3:
            NOP_FOR_1US();
        case 2:
            NOP_FOR_1US();
        case 1:
            NOP_FOR_1US();
        default:
            return;
        }
    }
    clock_delayVariable((uint32_t) us / CPU_MHZ_DIVIDER);
}

/*
 * Where call overhead dominates, use a macro!
 */
#define udelay_6() \
    { _NOP(); _NOP(); _NOP(); _NOP(); _NOP(); _NOP(); }

#define NOP1() {nop();}
#define NOP2() {nop(); nop();}
#define NOP3() {nop(); nop(); nop();}
#define NOP4() {nop(); nop(); nop(); nop();}
#define NOP5() {nop(); nop(); nop(); nop(); nop();}
#define NOP6() {nop(); nop(); nop(); nop(); nop(); nop();}
#define NOP8() {NOP4(); NOP4();}
#define NOP10() {NOP5(); NOP5();}


#endif
