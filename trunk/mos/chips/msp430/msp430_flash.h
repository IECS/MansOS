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

#ifndef MANSOS_MSP430_FLASH_H
#define MANSOS_MSP430_FLASH_H

//
// MSP430 internal flash memory (ROM) driver.
//

#include <defines.h>
#include "flash.h" // platform-specific constants

//
// Erase flash segments
//
void msp430flashErase(FlashAddress_t address, uint16_t dataLength);

//
// Write data in flash
//
void msp430flashWrite(FlashAddress_t address, const void *data, uint16_t dataLength);

//
// Write block data in flash (aligned to block size, dataLength must be multiple of block size)
//
void msp430flashWriteBlock(FlashAddress_t address, const void *data, uint16_t dataLength);


//
// Write a single uint16_t in flash
//
static inline void msp430flashWriteWord(FlashAddress_t address, uint16_t word)
{
    msp430flashWrite(address, &word, sizeof(uint16_t));
}

//
// Read data from flash
//
static inline void msp430flashRead(FlashAddress_t address, void *buffer,
                                   uint16_t bufferLength) {
    memcpy(buffer, (const void *)address, bufferLength);
}

//
// Read a single uint16_t from flash
//
static inline void msp430flashReadWord(FlashAddress_t address, uint16_t *word)
{
    memcpy(word, (const void *)address, sizeof(uint16_t));
}

#endif
