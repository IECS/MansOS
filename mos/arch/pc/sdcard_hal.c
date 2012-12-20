/**
 * Copyright (c) 2008-2011 the MansOS team. All rights reserved.
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
// SD card emulation driver
//

#define _XOPEN_SOURCE 600 /* For ftruncate() */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <eeprom.h>
#include <lib/assert.h>

#define FILENAME "sdcard"

#define SDCARD_SIZE 16 * 1024 * 1024 // 16 MB
#define SDCARD_SECTOR_SIZE 512

void sdcardInit(void)
{
    fputs("Opening SDCARD image `" FILENAME "'...\n", stderr);

    FILE *data = fopen(FILENAME, "r+b");
    if (!data)
    {
        data = fopen(FILENAME, "w+b");
        ASSERT(data != NULL);

        int ret = ftruncate(fileno(data), SDCARD_SIZE);
        ASSERT(ret == 0);
    }
    fclose(data);
}

bool sdcardReadBlock(uint32_t addr, void* buffer)
{
    ASSERT(addr + SDCARD_SECTOR_SIZE <= SDCARD_SIZE);

    FILE *data = fopen(FILENAME, "r+b");
    if (!data) return false;
    fseek(data, addr, SEEK_SET);
    size_t ret = fread(buffer, 1, SDCARD_SECTOR_SIZE, data);
    fclose(data);
    return ret == SDCARD_SECTOR_SIZE;
}

bool sdcardWriteBlock(uint32_t addr, const void *buffer)
{
    ASSERT(addr + SDCARD_SECTOR_SIZE <= SDCARD_SIZE);

    FILE *data = fopen(FILENAME, "w+b");
    if (!data) return false;
    fseek(data, addr, SEEK_SET);
    int ret = fwrite(buffer, 1, SDCARD_SECTOR_SIZE, data);
    fclose(data);
    return ret == SDCARD_SECTOR_SIZE;
}

#ifndef USE_FATFS
void sdcardRead(uint32_t addr, void *buf, uint16_t len)
{
    ASSERT(!ferror(data) && addr + len <= EEPROM_SIZE);

    FILE *data = fopen(FILENAME, "r+b");
    if (!data) return false;
    int ret = fseek(data, addr, SEEK_SET);
    size_t ret2 = fread(buf, 1, len, data);
    ASSERT(ret == 0 && ret2 == len);
    fclose(data);
}

void sdcardWrite(uint32_t addr, const void *buf, uint16_t len)
{
    ASSERT(addr + len <= SDCARD_SIZE);

    FILE *data = fopen(FILENAME, "w+b");
    if (!data) return;

    fseek(data, addr, SEEK_SET);
    fwrite(buf, 1, len, data);
    fclose(data);
}

void sdcardFlush(void)
{
}

#endif
