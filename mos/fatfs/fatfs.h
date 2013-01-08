/**
 * Copyright (c) 2012 the MansOS team. All rights reserved.
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

#ifndef MANSOS_FATFS_H
#define MANSOS_FATFS_H

//
// FAT file system high-level interface
//

#include "structures.h"
#include "posix-file.h"

// Memory card sector size
#define SDCARD_SECTOR_SIZE 512

//
// We do not support FAT32 at the moment;
// FAT16 is fine for accessing 2GB SD cards.
//
#define FAT32_SUPPORT 0

bool fatFsInitPartition(uint8_t partition);

static inline bool fatFsInit(void)
{
    // try to use partition #1 by default
    if (fatFsInitPartition(1)) return true;
    // if this fails, try to init the device as without partition table
    if (fatFsInitPartition(0)) return true;
    return false;
}

DirectoryEntry_t *fatFsFileSearch(const char *__restrict name, uint16_t *__restrict entryIndex);

DirectoryEntry_t *fatFsFileCreate(const char *__restrict name, uint16_t *__restrict entryIndex);

void fatFsFileRemove(const char *name);

void fatFsFileClose(FILE *handle);

uint16_t fatFsRead(FILE *handle, void *buffer, uint16_t maxLength);

uint16_t fatFsWrite(FILE *handle, const void *buffer, uint16_t length);

void fatFsFileFlush(FILE *handle);

bool fatfsGoToPosition(FILE *handle, uint32_t newPosition);

uint16_t fatFsGetFiles(char *buffer, uint16_t bufferSize);

#endif
