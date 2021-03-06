/*
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

#include "stdmansos.h"
#include <assert.h>

// test read/write in multiple clusters 
#define NUM_TESTS 1441
#define DATA_LEN 13

void appMain(void)
{
    static FILE fileBuffer;
    FILE *f = fopenEx("hello.txt", "w", &fileBuffer);
    ASSERT(f);
    ASSERT(f->isOpened);

    char buffer[DATA_LEN] = "0123456789abcdef";
    char readBuffer[DATA_LEN];
    int len;

    uint16_t i;
    for (i = 0; i < NUM_TESTS; ++i) {
        len = fwrite(buffer, 1, DATA_LEN, f);
        ASSERT(len == DATA_LEN);
    }
    fseek(f, 1001 * DATA_LEN, SEEK_SET);
    len = fread(readBuffer, 1, DATA_LEN, f);
    ASSERT(len == DATA_LEN);
    ASSERT(!memcmp(readBuffer, buffer, DATA_LEN));

    // close and reopen the file for reading
    fclose(f);
    memset(f, 0, sizeof(*f));
    f = fopenEx("hello.txt", "r", &fileBuffer);
    ASSERT(f);
    ASSERT(f->isOpened);

    fseek(f, 1001 * DATA_LEN, SEEK_SET);
    len = fread(readBuffer, 1, DATA_LEN, f);
    ASSERT(len == DATA_LEN);
    ASSERT(!memcmp(readBuffer, buffer, DATA_LEN));

    fclose(f);
    ASSERT(!f->isOpened);

    PRINTF("done!\n");
}
