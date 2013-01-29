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

#ifndef MANSOS_DELAY_H
#define MANSOS_DELAY_H

/// \file
/// Interface for small software delays (without entering low-power mode!)
///

#include <stdtypes.h>

///
/// Delay for approximate number of microseconds.
///
/// udelay() is suitable even for very small delays (1-6 us),
/// because they are handled as series of NOPs.
///
extern inline void udelay(uint16_t microseconds);

///
/// Delay for approximate number of milliseconds.
///
/// The function is not very precise! If precise delays are required, use timers.
///
extern inline void mdelay(uint16_t miliseconds);

#if defined __GNUC__
# if !(__GNUC__ >= 4 && __GNUC_MINOR__ >= 6) // < MSPGCC 4.6

///
/// Define __delay_cycles() macro as well, to keep compatibility with IAR code
///
#ifndef __delay_cycles
#define __delay_cycles(x) do {          \
    if (__builtin_constant_p(x) && x <= 15) {  \
        switch (x) {                    \
        case 15:                        \
            _NOP();                     \
        case 14:                        \
            _NOP();                     \
        case 13:                        \
            _NOP();                     \
        case 12:                        \
            _NOP();                     \
        case 11:                        \
            _NOP();                     \
        case 10:                        \
            _NOP();                     \
        case 9:                         \
            _NOP();                     \
        case 8:                         \
            _NOP();                     \
        case 7:                         \
            _NOP();                     \
        case 6:                         \
            _NOP();                     \
        case 5:                         \
            _NOP();                     \
        case 4:                         \
            _NOP();                     \
        case 3:                         \
            _NOP();                     \
        case 2:                         \
            _NOP();                     \
        case 1:                         \
            _NOP();                     \
            break;                      \
        }                               \
   }                                    \
   else {                               \
       udelay((x) / CPU_MHZ);           \
   } }  while (0)
#endif

#endif
#elif !defined __IAR_SYSTEMS_ICC__

///
/// Define __delay_cycles() macro as well, to keep compatibility with IAR code
///
#ifndef __delay_cycles
#define __delay_cycles(x) udelay((x) / CPU_MHZ)
#endif

#endif // __GNUC__ but not __IAR_SYSTEMS_ICC__ defined


// implementation
#include <udelay_hal.h>

#endif
