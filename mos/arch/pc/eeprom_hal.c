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
/*
 * eeprom_hal.c -- non-volatile configuration memory emulation
 */

#define _XOPEN_SOURCE 600 /* For ftruncate() */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <eeprom.h>
#include <assert.h>
#include <print.h>

#define FILENAME "eeprom"

void eepromInit(void)
{
    PRINTF("Opening EEPROM image `" FILENAME "'...\n");

    int data = open(FILENAME, O_RDONLY);
    if (data < 0) {
        data = creat(FILENAME, 0644);
        ASSERT(data > 0);

        int ret = ftruncate(data, EEPROM_SIZE);
        ASSERT(ret == 0);
    }
    close(data);
}

void eepromRead(uint16_t addr, void *buf, size_t len)
{
    int data = open(FILENAME, O_RDONLY);
    if (data < 0) return;

    ASSERT(addr + len <= EEPROM_SIZE);

    int ret = lseek(data, addr, SEEK_SET);
    size_t ret2 = read(data, buf, len);
    ASSERT(ret == 0 && ret2 == len);
    close(data);
}

void eepromWrite(uint16_t addr, const void *buf, size_t len)
{
    ASSERT(addr + len <= EEPROM_SIZE);

    int data = open(FILENAME, O_WRONLY);
    if (data < 0) return;

    lseek(data, addr, SEEK_SET);
    int r = write(data, buf, len);
    (void) r;
    close(data);
}
