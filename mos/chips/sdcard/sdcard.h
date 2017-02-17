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

#ifndef MANSOS_SDCARD_H
#define MANSOS_SDCARD_H

#include <stdtypes.h>

// The minimal unit that can be erased
// (Actually this is "block"; the name "sector" comes from analogy with flash,
// as in MansOS a SD card can be accessed as a type of flash memory chip)
#define SDCARD_SECTOR_SIZE    512

// The maximal unit that can be written
#define SDCARD_PAGE_SIZE      512

// 16 MB total default card size.
// The real size can be found using sdcardGetSize() function.
// Maximal supported size is 4GB due to addressing constraints (32 bit)
#ifndef SDCARD_SECTOR_COUNT
#define SDCARD_SECTOR_COUNT   (16ul * 1024 * 1024 / SDCARD_SECTOR_SIZE)
#endif

#define SDCARD_SIZE  (SDCARD_SECTOR_SIZE * SDCARD_SECTOR_COUNT)

// some of these functions are kept for compatibility with external flash only

// initialize the card (I/O pins, SPI interface)
bool sdcardInit(void);
// Erase the entire flash
void sdcardBulkErase(void);
// Erase on sector, containing address addr. Addr is not the number of sector,
// rather an address (any) inside the sector
void sdcardEraseSector(uint32_t addr);

//
// Internal functions; also used by FAT FS code
//
bool sdcardReadBlock(uint32_t addr, void* buffer);
bool sdcardWriteBlock(uint32_t addr, const void *buf);

#if USE_SDCARD_DIVIDED_WRITE
uint32_t targetAddress;
uint8_t sdWriteStep;

uint8_t *start;
uint8_t *end;
uint8_t status;

void initDividedSdCardWrite(uint32_t address, const void *buf);
inline uint8_t dividedSdCardWrite();

#endif //USE_SDCARD_DIVIDED_WRITE
//
// Higher-level API in case filesystem is not used.
// Do not use both filesystem and these functions together.
//
#if USE_SDCARD_LOW_LEVEL_API
// Read a block of data from addr
void sdcardRead(uint32_t addr, void* buffer, uint16_t len);
// Write len bytes (len <= 512) to SD card at 'addr'
// Block can split over multiple blocks
void sdcardWrite(uint32_t addr, const void *buf, uint16_t len);
// Flush cache buffers to SD card
void sdcardFlush(void);
#endif // USE_SDCARD_LOW_LEVEL_API

// return the number of 512-byte sectors
uint32_t sdcardGetSize(void);

// (re)initialize serial (actually SPI) interface
void sdcardInitSerial(void);


#endif
