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

#include <stdio.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <hil/extflash.h>

#define FLASH_FILE_NAME "extflash.dat"

FILE *flashFile;

static void flashFileClose();

void extFlashInit(void)
{
    static bool initCalled;
    if (initCalled) {
        fprintf(stderr, "extFlashInit: already called\n");
        return;
    }

    atexit(flashFileClose);

    struct stat st;
    if (stat(FLASH_FILE_NAME, &st) == 0) {
        // if the file exists and has right size - keep it betwen runs!
        if (st.st_size == EXT_FLASH_SIZE) return;
    }

    // flash file does not exist or has wrong size; open and truncate it
    flashFile = fopen(FLASH_FILE_NAME, "w+");
    // start in "all erased" state
    extFlashBulkErase();
    // close the file again
    fclose(flashFile);
    flashFile = NULL;
}

static void flashFileClose()
{
    // make sure the flash file is closed on application exit
    if (flashFile) {
        fclose(flashFile);
        flashFile = NULL;
    }
}

void extFlashSleep(void)
{
    // close the flash file, but do not clear or delete it
    if (flashFile) {
        fclose(flashFile);
        flashFile = NULL;
    }
}

void extFlashWake(void)
{
    // open the flash file for reading and writing
    if (!flashFile) {
        flashFile = fopen(FLASH_FILE_NAME, "r+");
    }
}

void extFlashRead(uint32_t addr, void *buf, uint16_t len)
{
    if (!flashFile) {
        fprintf(stderr, "extFlashRead: flash not opened\n");
        return;
    }
    if (addr + len > EXT_FLASH_SIZE) {
        fprintf(stderr, "extFlashRead: address out of bounds!\n");
        return;
    }
    if (fseek(flashFile, addr, SEEK_SET) != 0) {
        perror("fseek");
        return;
    }
    if (fread(buf, 1, len, flashFile) != len) {
        perror("fread");
    }
}

void extFlashWrite(uint32_t addr, const void *buf, uint16_t len)
{
    if (!flashFile) {
        fprintf(stderr, "extFlashWrite: flash not opened\n");
        return;
    }
    if (addr + len > EXT_FLASH_SIZE) {
        fprintf(stderr, "extFlashWrite: address out of bounds!\n");
        return;
    }

    // to determine if the flash can be written, compare all contents with 0xff.
    uint8_t *oldContents = malloc(len);
    if (fseek(flashFile, addr, SEEK_SET) != 0) {
        perror("fseek");
        return;
    }
    if (fread(oldContents, 1, len, flashFile) != len) {
        perror("fread");
    }
    uint32_t i;
    for (i = 0; i < len; ++i) {
        if (oldContents[i] != 0xff) {
            fprintf(stderr, "extFlashWrite: attempt to write in a sector that was not erased!\n");
            return;
        }
    }
    free(oldContents);

    // write new contents
    if (fseek(flashFile, addr, SEEK_SET) != 0) {
        perror("fseek");
        return;
    }
    if (fwrite(buf, 1, len, flashFile) != len) {
        perror("fwrite");
    }
}

void extFlashBulkErase(void)
{
    if (!flashFile) {
        fprintf(stderr, "extFlashBulkErase: flash not opened\n");
        return;
    }

    // fill it with 0xff
    uint8_t block[1024];
    memset(block, 0xff, sizeof(block));
    rewind(flashFile);
    uint32_t i;
    for (i = 0; i < EXT_FLASH_SIZE / sizeof(block); i++) {
        fwrite(block, 1, sizeof(block), flashFile);
    }
}

void extFlashEraseSector(uint32_t addr)
{
    // printf("extFlashEraseSector: addr=%u\n", addr);

    if (!flashFile) {
        fprintf(stderr, "extFlashEraseSector: flash not opened\n");
        return;
    }
    addr = ALIGN_DOWN_U32(addr, EXT_FLASH_SECTOR_SIZE);
    if (addr >= EXT_FLASH_SIZE) {
        fprintf(stderr, "extFlashEraseSector: address out of bounds!\n");
        return;
    }

    if (fseek(flashFile, addr, SEEK_SET) != 0) {
        perror("fseek");
        return;
    }

    // fill the sector with 0xff
    uint8_t block[1024];
    memset(block, 0xff, sizeof(block));
    uint32_t i;
    for (i = 0; i < EXT_FLASH_SECTOR_SIZE / sizeof(block); i++) {
        fwrite(block, 1, sizeof(block), flashFile);
    }
}
