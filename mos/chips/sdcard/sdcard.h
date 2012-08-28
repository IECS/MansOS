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

#ifndef MANSOS_SDCARD_H
#define MANSOS_SDCARD_H

#include <kernel/stdtypes.h>

// minimal unit that can be erased
#define SDCARD_SECTOR_SIZE    512

// maximal unit that can be written
#define SDCARD_PAGE_SIZE      512

// 1 GB total (the minimal size - use this only for compatibility with flash drivers)
// The maximal size is 4GB due to addressing constraints (32 bit)
#define SDCARD_SECTOR_COUNT   (2 * 1024 * 1024ul)

#define SDCARD_SIZE  (SDCARD_SECTOR_SIZE * SDCARD_SECTOR_COUNT)

// initialize pin directions and SPI in general. Enter low power mode afterwards
bool sdcardInit(void);
// Enter low power mode (wait for last instruction to complete)
void sdcardSleep(void);
// Exit low power mode
void sdcardWake(void);
// Read a block of data from addr
void sdcardRead(uint32_t addr, void* buffer, uint16_t len);
// Write len bytes (len <= 256) to flash at addr
// Block can split over multiple sectors/pages
void sdcardWrite(uint32_t addr, const void *buf, uint16_t len);
// Erase the entire flash
void sdcardBulkErase(void);
// Erase on sector, containing address addr. Addr is not the number of sector,
// rather an address (any) inside the sector
void sdcardEraseSector(uint32_t addr);

// internal
bool sdcardReadBlock(uint32_t addr, void* buffer);
bool sdcardWriteBlock(uint32_t addr, const void *buf);

void sdcardInitUsart(void);

#endif
