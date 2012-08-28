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

//--------------------------------------------------------------------------------
// MSP430-specific memory constants and operations
//--------------------------------------------------------------------------------

#ifndef _MANSOS_ARCH_MEM_H_
#define _MANSOS_ARCH_MEM_H_

#include <kernel/defines.h>
#include <dynamic_memory.h>

// linker will provide symbol for end-of-memory
extern uint8_t _end;

//#define KERNEL_STACK_SIZE 256
#define KERNEL_STACK_SIZE 512

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
