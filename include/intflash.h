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

#ifndef MANSOS_INTERNAL_FLASH_H
#define MANSOS_INTERNAL_FLASH_H

//
// Available functions are described below
//

// The real interface is defined in this file
#include "intflash_hal.h"

// Erase flash segments
// void intFlashErase(FlashAddress_t address, uint16_t dataLength);

// Write data in flash
// void intFlashWrite(FlashAddress_t address, const void *data, uint16_t dataLength);

// Write block data in flash (aligned to block size, dataLength must be multiple of block size)
// void intFlashWriteBlock(FlashAddress_t address, const void *data, uint16_t dataLength);

// Read data from flash
// void intFlashRead(FlashAddress_t address, void *buffer, uint16_t bufferLength);

#ifndef INT_FLASH_SEGMENT_SIZE
// Define default values for these

#warning Internal flash constants not defined for this platform!

// Total size of flash memory
#define INT_FLASH_SIZE            0

// Size of minimal flash unit that can be erased at once
#define INT_FLASH_SEGMENT_SIZE    0

// Size of maximal flash unit that can be written at once
#define INT_FLASH_BLOCK_SIZE      0

// Start address of code memory
#define INT_FLASH_START           0x0

// Start address of "information memory"
#define INT_FLASH_INFOMEM_START   0x0

// Unsigned type large enough for holding flash address range
typedef uint16_t FlashAddress_t;

#endif

#endif
