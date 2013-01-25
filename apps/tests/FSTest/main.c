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
/*
 * FSTest -- basic filesystem tests
 *
 * Should be run on a clean filesystem.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <extflash.h>
#include <fs.h>
#include <leds.h>
#include <sleep.h>
#include <defines.h>
#include <assert.h>
#include <print.h>

/* Internal header, not for users */
#include <fs/block/common.h>

static void mark(void)
{
    static unsigned counter = 0;

    counter++;
    PRINTF("Test %2u passed\n", counter);
#ifndef PLATFORM_PC
    ledsSet(counter);
#endif
}

#define DATA_SIZE (EXT_FLASH_SECTOR_COUNT * BLOCKS_PER_SEGMENT * BLOCK_DATA_SIZE)

#define BUFSIZE 42
static char data[BUFSIZE];

static void writeFile(int8_t f, fsOff_t size)
{
    fsOff_t written = 0;

    while (written < size)
    {
        ssize_t r = fsWrite(f, data + written % BUFSIZE,
                            MIN(BUFSIZE - written % BUFSIZE, size - written));
        ASSERT(r > 0);
        written += r;
    }
}

static void readFile(int8_t f, fsOff_t size)
{
    static char buf[BUFSIZE];

    fsOff_t     read = 0;

    while (read < size)
    {
        ssize_t r;

        /* Make sure we get new data */
        memset(buf, 0, BUFSIZE - read % BUFSIZE);

        r = fsRead(f, buf, BUFSIZE - read % BUFSIZE);
        ASSERT(r > 0);
        ASSERT(!memcmp(buf, data + read % BUFSIZE, r));
        read += r;
    }

    ASSERT(fsRead(f, buf, BUFSIZE) == 0);
}

void appMain(void)
{
    int8_t f;

    size_t i;
    for (i = 0; i < BUFSIZE; i++)
        data[i] = i;

    /*
     * Remove files that might have been left from a previous run. Ignore
     * errors. Note that this does not help if someone created a file with
     * another name and allocated some space for it.
     */
    fsRemove("/blk/test");
    fsRemove("/blk/test1");
    fsRemove("/blk/test2");
    
    
    /* Test /dev */
    {
        char   buf[5];

        ASSERT((f = fsOpen("/dev/leds", FS_RDWR)) != -1);
        ASSERT(fsWrite(f, ".0x1\n", 5) == 5);
        ASSERT(fsRead(f, buf, 5) == 5);
        ASSERT(!memcmp("0011\n", buf, 5));
        ASSERT(fsClose(f));
    }
    mark();

    /* Test regular writing and appending */
    {
#define BOUNDARY (DATA_SIZE / 2 - (DATA_SIZE / 2) % BUFSIZE)

        ASSERT((f = fsOpen("/blk/test", FS_APPEND)) != -1);
        writeFile(f, BOUNDARY);
        ASSERT(fsClose(f));

        ASSERT((f = fsOpen("/blk/test", FS_APPEND)) != -1);
        writeFile(f, DATA_SIZE - BOUNDARY);
        ASSERT(fsClose(f));
    }
    mark();

    /* Test regular reading */
    {
        ASSERT((f = fsOpen("/blk/test", FS_READ)) != -1);
        readFile(f, DATA_SIZE);
        ASSERT(fsClose(f));
    }
    mark();

    /* Test seeking */
    {
#define OFFSET (DATA_SIZE / 3 - (DATA_SIZE / 3) % BUFSIZE)

        ASSERT((f = fsOpen("/blk/test", FS_READ)) != -1);
        fsSeek(f, OFFSET);
        readFile(f, DATA_SIZE - OFFSET);
        ASSERT(fsClose(f));
    }
    mark();

    /* Test fsStat */
    {
        struct fsStat s;
        ASSERT(fsStat("/blk/test", &s));
        ASSERT(s.size == DATA_SIZE);
    }
    mark();

    /* Test renaming */
    {
        struct fsStat s;
        ASSERT(fsRename("/blk/test", "/blk/newfile"));
        ASSERT(fsStat("/blk/newfile", &s));
    }
    mark();

    /* Test deleting */
    {
        struct fsStat s;
        ASSERT(fsRemove("/blk/newfile"));
        ASSERT(!fsStat("/blk/newfile", &s));
    }
    mark();

    /* Test opening a file multiple times */
    {
        int8_t f2;

        ASSERT((f = fsOpen("/blk/test", FS_APPEND)) != -1);
        writeFile(f, 3 * CHUNK_DATA_SIZE / 2);

        ASSERT((f2 = fsOpen("/blk/test", FS_READ)) != -1);
        readFile(f2, CHUNK_DATA_SIZE);

        ASSERT(fsClose(f));
        ASSERT(fsClose(f2));
        ASSERT(fsRemove("/blk/test"));
    }
    mark();

    /* Test unbuffered I/O */
    {
        ASSERT((f = fsOpen("/blk/test", FS_APPEND | FS_NOCACHE)) != -1);
        writeFile(f, DATA_SIZE);
        ASSERT(fsClose(f));

        ASSERT((f = fsOpen("/blk/test", FS_READ | FS_NOCACHE)) != -1);
        readFile(f, DATA_SIZE);
        ASSERT(fsClose(f));

        ASSERT(fsRemove("/blk/test"));
    }
    mark();

    /* Test I/O with data integrity checking */
    {
        ASSERT((f = fsOpen("/blk/test", FS_APPEND | FS_CHECKSUM)) != -1);
        writeFile(f, DATA_SIZE);
        ASSERT(fsClose(f));

        ASSERT((f = fsOpen("/blk/test", FS_READ | FS_CHECKSUM)) != -1);
        readFile(f, DATA_SIZE);
        ASSERT(fsClose(f));

        ASSERT(fsRemove("/blk/test"));
    }
    mark();

    /* Test fsFlush(), test reading and verifying partial chunks */
    {
        ASSERT((f = fsOpen("/blk/test", FS_APPEND | FS_CHECKSUM)) != -1);
        writeFile(f, 3 * CHUNK_DATA_SIZE / 2);
        ASSERT(fsFlush(f));
        ASSERT(fsClose(f));

        ASSERT((f = fsOpen("/blk/test", FS_READ | FS_CHECKSUM)) != -1);
        readFile(f, 3 * CHUNK_DATA_SIZE / 2);
        ASSERT(fsClose(f));

        ASSERT(fsRemove("/blk/test"));
    }
    mark();

    /* Test multiple files */
    {
#define HALF_SIZE ((DATA_SIZE - BLOCK_DATA_SIZE) / 2)

        ASSERT((f = fsOpen("/blk/test1", FS_APPEND)) != -1);
        writeFile(f, HALF_SIZE);
        ASSERT(fsClose(f));
        ASSERT((f = fsOpen("/blk/test2", FS_APPEND)) != -1);
        writeFile(f, HALF_SIZE);
        ASSERT(fsClose(f));

        ASSERT((f = fsOpen("/blk/test1", FS_READ)) != -1);
        readFile(f, HALF_SIZE);
        ASSERT(fsClose(f));
        ASSERT((f = fsOpen("/blk/test2", FS_READ)) != -1);
        readFile(f, HALF_SIZE);
        ASSERT(fsClose(f));
    }
    mark();


    PRINTF("All tests passed\n");
#ifndef PLATFORM_PC
    {
        uint_t l = 1;
        while (true)
        {
            l = ((l << 1) | (l >> 2)) & 0x7;
            ledsSet(l);
            msleep(333);
        }
    }
#endif
}
