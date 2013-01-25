/*
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

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <assert.h>
#include <print.h>

#define SDCARD_SECTOR_COUNT 32768 // 512 * 32768 = 16 MB card size
#include <sdcard/sdcard.h>

#define FILENAME "sdcard.dat"

static const uint8_t zeroSector[SDCARD_SECTOR_SIZE];

bool sdcardInit(void)
{
    PRINTF("Opening SDCARD image `" FILENAME "'...\n");

    int data = open(FILENAME, O_RDONLY);
    if (data < 0) {
        sdcardBulkErase();
    }
    close(data);
    return true;
}

bool sdcardReadBlock(uint32_t addr, void* buffer)
{
    ASSERT(addr + SDCARD_SECTOR_SIZE <= SDCARD_SIZE);

    int data = open(FILENAME, O_RDONLY);
    if (data < 0) return false;
    lseek(data, addr, SEEK_SET);
    size_t ret = read(data, buffer, SDCARD_SECTOR_SIZE);
    close(data);
    return ret == SDCARD_SECTOR_SIZE;
}

bool sdcardWriteBlock(uint32_t addr, const void *buffer)
{
    ASSERT(addr + SDCARD_SECTOR_SIZE <= SDCARD_SIZE);

    int data = open(FILENAME, O_WRONLY);
    if (data < 0) return false;
    lseek(data, addr, SEEK_SET);
    int ret = write(data, buffer, SDCARD_SECTOR_SIZE);
    close(data);
    return ret == SDCARD_SECTOR_SIZE;
}

void sdcardBulkErase(void)
{
    int data = creat(FILENAME, 0644);
    ASSERT(data > 0);
    uint16_t i;
    for (i = 0; i < SDCARD_SECTOR_COUNT; ++i) {
        int r = write(data, zeroSector, sizeof(zeroSector));
        (void) r;
    }
}

void sdcardEraseSector(uint32_t address)
{
    int data = open(FILENAME, O_WRONLY);
    if (data < 0) return;
    lseek(data, address, SEEK_SET);
    int r = write(data, zeroSector, sizeof(zeroSector));
    (void) r;
}

void sdcardRead(uint32_t addr, void *buf, uint16_t len)
{
    ASSERT(addr + len <= SDCARD_SIZE);

    int data = open(FILENAME, O_RDONLY);
    if (data < 0) return;
    int ret = lseek(data, addr, SEEK_SET);
    size_t ret2 = read(data, buf, len);
    ASSERT(ret == 0 && ret2 == len);
    close(data);
}

void sdcardWrite(uint32_t addr, const void *buf, uint16_t len)
{
    ASSERT(addr + len <= SDCARD_SIZE);

    int data = open(FILENAME, O_WRONLY);
    if (data < 0) return;

    lseek(data, addr, SEEK_SET);
    int r = write(data, buf, len);
    (void) r;
    close(data);
}

void sdcardFlush(void)
{
    // nothing
}

uint32_t sdcardGetSize(void)
{
    return SDCARD_SECTOR_COUNT;
}
 
