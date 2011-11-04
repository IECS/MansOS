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
 *
 * fs/core.c -- core file system functions
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <hil/fs.h>
#include <kernel/mos_sem.h>
#include <kernel/stdtypes.h>

#include "common.h"
#include "init.h"
#include "prefix.h"
#include "types.h"

/* File handle */
struct fsFileHandle {
    const struct fsOperations * restrict ops;  /* File operations to use */
    void * restrict                      data; /* Unique id to pass to @ops */
};

/* Open files array */
static struct fsFileHandle openFiles[FS_MAX_OPEN_FILES];
static mos_mutex_t         openFilesMutex;

/*
 * Last error number
 *
 * TODO: Need to use thread-local data
 */
static uint8_t lastErr;

/* Initialize the file system */
void fsInit(void)
{
    mos_mutex_init(&openFilesMutex);

    fsInitSubsystems();
}

/* Set last error */
void fsSetError(fsError_t err)
{
    lastErr = err;
}

/* Get last error */
fsError_t fsLastError(void)
{
    return lastErr;
}

/* Allocate a new file handle */
static int8_t fsAllocHandle(const struct fsOperations *ops)
{
    int8_t i;

    mos_mutex_lock(&openFilesMutex);
    for (i = 0; i < FS_MAX_OPEN_FILES; i++)
    {
        if (!openFiles[i].ops)
        {
            openFiles[i].ops = ops;
            break;
        }
    }
    mos_mutex_unlock(&openFilesMutex);

    if (i == FS_MAX_OPEN_FILES)
    {
        fsSetError(FS_ERR_NOMEM);
        return -1;
    }

    return i;
}

/* Free file handle */
static inline void fsFreeHandle(int8_t fd)
{
    mos_mutex_lock(&openFilesMutex);
    openFiles[fd].ops = NULL;
    mos_mutex_unlock(&openFilesMutex);
}


/*
 * Implement the user interface. First define functions that work on paths...
 */

bool fsStat(const char * restrict path, struct fsStat * restrict buf)
{
    const struct fsOperations *ops = fsPrefixLookup(&path);

    return !ops ? false : ops->stat(path, buf);
}

int8_t fsOpen(const char *path, fsMode_t mode)
{
    int8_t                     fd;
    const struct fsOperations *ops;

    if (mode & FS_NOCACHE && mode & FS_CHECKSUM) /* Both don't work */
    {
        fsSetError(FS_ERR_INVAL);
        return -1;
    }

    if (!(ops = fsPrefixLookup(&path)))
        return -1;

    if ((fd = fsAllocHandle(ops)) == -1)
        return -1;

    if (!(openFiles[fd].data = openFiles[fd].ops->open(path, mode)))
    {
        fsFreeHandle(fd);
        return -1;
    }

    return fd;
}

bool fsRemove(const char *path)
{
    const struct fsOperations *ops = fsPrefixLookup(&path);

    return !ops ? false : ops->remove(path);
}

bool fsRename(const char *old, const char *new)
{
    const struct fsOperations *a = fsPrefixLookup(&old),
                              *b = fsPrefixLookup(&new);

    if (a != b)
    {
        /* TODO */
        fsSetError(FS_ERR_NOSYS);
        return false;
    }
    else if (!a)
        return false;

    return !strcmp(old, new) ? true : a->rename(old, new);
}

/*
 * ...then complete the list with functions that work on file handles
 */

ssize_t fsRead(int8_t fd, void *buf, size_t count)
{
    return openFiles[fd].ops->read(openFiles[fd].data, buf, count);
}

fsOff_t fsTell(int8_t fd)
{
    return openFiles[fd].ops->tell(openFiles[fd].data);
}

void fsSeek(int8_t fd, fsOff_t pos)
{
    return openFiles[fd].ops->seek(openFiles[fd].data, pos);
}

ssize_t fsWrite(int8_t fd, const void *buf, size_t count)
{
    return openFiles[fd].ops->write(openFiles[fd].data, buf, count);
}

bool fsFlush(int8_t fd)
{
    return openFiles[fd].ops->flush(openFiles[fd].data);
}

bool fsClose(int8_t fd)
{
    bool res = openFiles[fd].ops->close(openFiles[fd].data);

    fsFreeHandle(fd);

    return res;
}
