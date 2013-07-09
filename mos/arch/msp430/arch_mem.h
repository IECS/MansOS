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

//--------------------------------------------------------------------------------
// MSP430-specific memory constants and operations
//--------------------------------------------------------------------------------

#ifndef _MANSOS_ARCH_MEM_H_
#define _MANSOS_ARCH_MEM_H_

#include <defines.h>
#include <dynamic_memory.h>

////////////////////////////////////////////////////////////////////////////////
// Memory layout
////////////////////////////////////////////////////////////////////////////////
// MSP430x16x User's Guide, 1.4 Address Space, pages 16-17:
// FFE0 - FFFF:   Interrupt vector table
// .... - FFDF:   Flash 
// 0200 - ....:   RAM
// 0100 - 01FF:   16-Bit Peripheral Modules
//   10 -   FF:   8-Bit Peripheral Modules
//   00 -   0F:   Special Function Registers (SFRs)

// http://en.wikipedia.org/wiki/TI_MSP430#MSP430_address_space:
// FFE0 - FFFF:   Interrupt vector table
// .... - FFDF:   Flash
// 1100 - 38FF:   Extended RAM on models with more than 2048 bytes of RAM. 
//                    (0x1100-0x18FF is a copy of 0x0200-0x09FF)
// 1000 - 10FF:   256 bytes of data flash ROM (flash ROM parts only).
// 0C00 - 0FFF:   1024 bytes of bootstrap loader ROM (flash ROM parts only).
// 0200 - 09FF:   Up to 2048 bytes of RAM
// 0100 - 01FF:   16-bit peripherals. Accessed using 16-bit loads and stores.
//   08 -   FF:   8-bit peripherals. Accessed using 8-bit loads and stores.
//   00 -   07:   Special function registers (interrupt control registers)

// RAM contents:
// 1100 - _end:   Static memory
// _end - 3800:   Dynamic memory,
//                3800 = HEAP_TOP = 1100 + MEM_SIZE - KERNEL_STACK_SIZE
// 3800 - 3900:   Kernel Stack (256B)

// Memory for user thread stacks is allocated from dynamic memory

// linker will provide symbol for end-of-memory
extern uint8_t _end;

#if PLATFORM_FARMMOTE || PLATFORM_MIIMOTE
#define KERNEL_STACK_SIZE 256
#else
#define KERNEL_STACK_SIZE 512
#endif

//--------------------------------------------------------------------------------
// functions
//--------------------------------------------------------------------------------

#define memInit(region, size) memoryInit(region, size)
#define memAlloc(size) memoryAlloc(size)
#define memFree(ptr) memoryFree(ptr)

static inline void platformMemInit(void) {
    int dummy;
    memInit(&_end, ALIGN_DOWN(&dummy, KERNEL_STACK_SIZE));
}

#endif
