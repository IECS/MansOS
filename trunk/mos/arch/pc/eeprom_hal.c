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
/*
 * eeprom_hal.c -- non-volatile configuration memory emulation
 */

#define _XOPEN_SOURCE 600 /* For ftruncate() */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <hil/eeprom.h>
#include <lib/assert.h>

#define FILENAME "eeprom"

static FILE *data;

static void eepromClose(void);

void eepromInit(void)
{
    int ret;

    ret = atexit(eepromClose);
    ASSERT(ret == 0);

    fputs("Opening EEPROM image `" FILENAME "'...\n", stderr);

    data = fopen(FILENAME, "r+b");
    if (!data)
    {
        data = fopen(FILENAME, "w+b");
        ASSERT(data != NULL);

        ret = ftruncate(fileno(data), EEPROM_SIZE);
        ASSERT(ret == 0);
    }
}

static void eepromClose(void)
{
    ASSERT(!ferror(data));
    int ret = fclose(data);
    ASSERT(ret == 0);
}

void eepromRead(uint16_t addr, void *buf, size_t len)
{
    ASSERT(!ferror(data) && addr + len <= EEPROM_SIZE);

    int ret = fseek(data, addr, SEEK_SET);
    size_t ret2 = fread(buf, 1, len, data);
    ASSERT(ret == 0 && ret2 == len);
}

void eepromWrite(uint16_t addr, const void *buf, size_t len)
{
    ASSERT(addr + len <= EEPROM_SIZE);

    fseek(data, addr, SEEK_SET);
    fwrite(buf, 1, len, data);
}
