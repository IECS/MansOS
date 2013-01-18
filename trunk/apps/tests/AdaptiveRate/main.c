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
 * AdaptiveRate -- implement adaptive sample rate using file system facilities
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <fs.h>
#include <mansos.h>
#include <lib/assert.h>
#include <lib/dprint.h>

#define FILE_PARTS 2

static int8_t files[FILE_PARTS];
static int    current, fileCount;

/* Handle errors */
static void error(const char *msg)
{
    PRINTF("error: %s\n", msg);
    ASSERT(false);
}

/* Generate a file name */
static const char *name(int i)
{
#define NAME_LEN 12
    static char s[NAME_LEN];

    snprintf(s, NAME_LEN, "/blk/file%02d", i);
    return s;
}

/* Open stream */
static void streamOpen(fsMode_t mode)
{
    int i;

    fileCount = current = 0;

    for (i = 0; i < FILE_PARTS; i++)
    {
        if (mode & FS_APPEND)
        {
            /* Erase any previous files */
            fsRemove(name(i));
        }

        files[i] = fsOpen(name(i), mode);
        if (files[i] != -1)
            fileCount++;
    }

    if ((mode & FS_APPEND && fileCount != FILE_PARTS)
        || (mode & FS_READ && fileCount < 1))
    {
        error("failed to open stream");
    }
}

/* Close stream */
static void streamClose(void)
{
    int i;

    for (i = 0; i < FILE_PARTS; i++)
    {
        if (files[i] != -1)
            fsClose(files[i]); /* Ignore errors */
    }
}

/* Write to stream. One call counts as one data record. */
static bool streamWrite(const void *buf, size_t size)
{
    /* Skip the record if the file is closed */
    if (files[current] != -1)
    {
        size_t written = 0;
        while (written < size)
        {
            ssize_t ret = fsWrite(files[current], buf + written, size - written);
            if (ret < 1)
                break;
            else
                written += ret;
        }
        if (written < size) /* An error occured */
        {
            if (fsLastError() == FS_ERR_NOSPC) /* No free space */
            {
                if (fileCount == 1) /* No more files to spare */
                {
                    return false;
                }
                else /* Close and delete the file, ignoring errors */
                {
                    fsClose(files[current]);
                    fsRemove(name(current));
                    files[current] = -1;
                    fileCount--;
                }
            }
            else
                error("input/output failure");
        }
    }

    current = (current + 1) % FILE_PARTS;
    return true;
}

/* Read one record from stream */
static bool streamRead(void *buf, size_t size)
{
    size_t read = 0;

    /* Skip non-open files */
    while (files[current] == -1)
        current = (current + 1) % FILE_PARTS;

    while (read < size)
    {
        ssize_t ret = fsRead(files[current], buf + read, size - read);
        if (ret == -1)
            error("input/output failure");
        else if (ret == 0) /* End of file */
            return false;
        else
            read += ret;
    }

    current = (current + 1) % FILE_PARTS;
    return true;
}


/* Test program */
void appMain(void)
{
#define REC_SIZE 4
    static char buf[REC_SIZE];

    long count = 0;
    streamOpen(FS_APPEND);
    while (streamWrite(buf, REC_SIZE))
        count++;
    streamClose();
    PRINTF("wrote %ld records\n", count);

    count = 0;
    streamOpen(FS_READ);
    while (streamRead(buf, REC_SIZE))
        count++;
    streamClose();
    PRINTF("read %ld records\n", count);
}
