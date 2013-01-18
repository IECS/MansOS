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
#include <lib/assert.h>

// test read/write in two blocks
#define NUM_TESTS 41
#define DATA_LEN 13

// test read/write in two clusters
// #define NUM_TESTS 2050
// #define DATA_LEN 16

// test read/write in many clusters
//#define NUM_TESTS 10000
//#define DATA_LEN 13

#define TEST_READ 1

void appMain(void)
{
    uint16_t i;
    char buffer[DATA_LEN] = "0123456789abcdef";
    FILE *f = fopen("hello.txt", "w+");
    ASSERT(f);
    ASSERT(f->fd != -1);

    redLedOn();

    for (i = 0; i < NUM_TESTS; ++i) {
        int len = fwrite(buffer, 1, DATA_LEN, f);
        // PRINTF("len=%d\n", len);
        ASSERT(len == DATA_LEN);
    }

    fclose(f);
    ASSERT(f->fd == -1);
    rewind(f);

#if TEST_READ
    char readBuffer[DATA_LEN];
    for (i = 0; i < NUM_TESTS; ++i) {
        int len = fread(readBuffer, 1, DATA_LEN, f);
        ASSERT(len == DATA_LEN);
        ASSERT(!memcmp(readBuffer, buffer, DATA_LEN));
    }
#endif

    redLedOff();

    PRINTF("done!\n");
}
