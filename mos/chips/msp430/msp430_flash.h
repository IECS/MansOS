/**
 * Copyright (c) 2008-2010 Leo Selavo and the contributors. All rights reserved.
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

#include <stdint.h>
#include <string.h>

//
// MSP430 internal flash memory (ROM) driver.
// On MSP430-F1611 (e.g. TelosB) 48kb internal flash memory is present.
//
#define MSP430_FLASH_START 0x4000
#define MSP430_FLASH_END   0xFFE0 // End of the flash memory: interrupt vector start
#define MSP430_FLASH_SIZE  (0xFFFF - MSP430_FLASH_START + 1)

//
// Flash segment size (this is the minimal flash area that can be erased)
//
#define MSP430_FLASH_SEGMENT_SIZE  512
//
// Flash block size (this is the maximal flash area that can be written at once)
//
#define MSP430_FLASH_BLOCK_SIZE    64

//
// Flash Information Memory range: 256 bytes in two 128-byte segments
//
#define MSP430_FLASH_INFOMEM_START 0x1000
#define MSP430_FLASH_INFOMEM_END   0x10FF
//
// Flash Information Memory segment size
//
#define MSP430_FLASH_INFOMEM_SEGMENT_SIZE  128

typedef uint16_t FlashAddress_t;

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
