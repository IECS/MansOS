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
// MansOS memory block management for MCU platforms
// Base taken from Mantis: http://mantis.cs.colorado.edu/
//--------------------------------------------------------------------------------

#ifndef MANSOS_MEM_H
#define MANSOS_MEM_H

#include <kernel/stdtypes.h>

//----------------------------------------------------------
// functions
//----------------------------------------------------------
/** @brief Initialize the heap with a single free region of memory.
 * @param region Memory region to initialize
 * @param size Size of the memory region
 */
void memoryInit(void *region, uint16_t size);

/** @brief Try to allocate a region of memory.
 * @param size Size of the region to allocate.
 * @return NULL if not enough memory is available, else the allocated memory.
 */
void *memoryAlloc(uint16_t size);

/** @brief Free a block of memory.
 * @param block Block of memory to free
 */
void memoryFree(void *block);

#endif
