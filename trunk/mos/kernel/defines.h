/**
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
// Platform independent defines and constants.
// Also includes platform dependant defines.
//

#ifndef _MANSOS_DEFINES_H_
#define _MANSOS_DEFINES_H_

#include "stdtypes.h"
#include <stdlib.h> // include file where NULL is defined
#include <limits.h> // INT_MAX etc
#include <string.h> // memcpy()

//
// Compiler specific function and data attributes
//
#if defined __GNUC__
#include "defines_gcc.h"
#elif defined __IAR_SYSTEMS_ICC__
#include "defines_iar.h"
#elif defined SDCC
#include "defines_sdcc.h"
#else
#error Unsupported compiler
#endif

//
// Compile time assert (use for type size checing and similar)
//
#define COMPILE_TIME_ASSERT(e, name)                 \
    struct CTA_##name { char name[(e) - 1]; }


//
// Build single number from multiple bytes.
// Handy for creating IP and other addresses.
//
#define MAKE_U16(a, b)         (((a) << 8) | (b))
#define MAKE_U32(a, b, c, d)   (((uint32_t) (a) << 24) | ((uint32_t) (b) << 16ul) | ((c) << 8) | (d))

//
// MIN and MAX of two values
//
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) < (b) ? (b) : (a))

//
// Is 'a' is after 'b'?
// Use for comparing time values. Handles overflow correctly.
//
#define timeAfter16(a, b) ((int16_t) ((b) - (a)) < 0)
#define timeAfter32(a, b) ((int32_t) ((b) - (a)) < 0)
#define timeAfter64(a, b) ((int64_t) ((b) - (a)) < 0)

// define jiffies type
#ifdef USE_LONG_LIFETIME
typedef uint64_t ticks_t;
#define timeAfter(a, b) timeAfter64(a, b)
#else
typedef uint32_t ticks_t;
#define timeAfter(a, b) timeAfter32(a, b)
#endif

//
// Alignment check and ops: work for powers of 2
//
#define IS_ALIGNED(addr, align) (!((uint16_t)(addr) & (align - 1)))
#define ALIGN_DOWN(addr, align) ((uint16_t)(addr) & ~(align - 1))
#define ALIGN_UP(addr, align) (((uint16_t)(addr) + (align - 1)) & ~(align - 1))
#define ALIGN_DOWN_U32(addr, align) ((uint32_t)(addr) & ~(align - 1))
#define ALIGN_UP_U32(addr, align) (((uint32_t)(addr) + (align - 1)) & ~(align - 1))

//
// MSB and LSB of 2-byte word
//
#define MSB(a) ((a & 0xFF00) >> 8)
#define LSB(a) ((a & 0xFF))

//
// Serial port default baudrate (XXX: move this)
//
#ifndef SERIAL_PORT_BAUDRATE
#define SERIAL_PORT_BAUDRATE 38400
#endif

#ifndef CPU_MHZ
#error CPU_MHZ must be defined!
#endif

#if MCU_AVR
// Atmel has straight decimal MHz
#define CPU_HZ (CPU_MHZ * 1000000ul)
#else
// msp430-based devices usually have binary MHz
#define CPU_HZ (CPU_MHZ * 1024ul * 1024)
#endif

// timer frequency, times per second;
#if CPU_MHZ > 4 && MCU_AVR
// sorry, cannot support once-per-10ms ticks - 8-bit timer overflow!
#define TIMER_INTERRUPT_HZ  1000
#else
#define TIMER_INTERRUPT_HZ  100
#endif

// timer A feeds from from ACLK (32'768 Hz)
#define TIMER_SECOND ACLK_SPEED 
#define TIMER_100_MS (ACLK_SPEED / 10 + 1)

#endif
