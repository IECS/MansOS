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
 *
 * fs/dev.c -- device file subsystem
 */

#include <stdbool.h>
#include <string.h>

#include <fs/common.h>
#include <fs/prefix.h>
#include <fs/types.h>
#include <kernel/defines.h>

#include "dev.h"
#include "devices.h"
#include "init.h"

/* Device file entry */
struct fsDevEntry {
    const char * restrict                   name;
    const struct fsDevOperations * restrict ops;
};

#define FS_DEVICE(Name, Ops) { .name = Name, .ops = Ops },
static const struct fsDevEntry fsDevEntries[] = { FS_DEVICE_LIST };

/*
 * Subsystem interface functions.
 *
 * In this subsystem we use @id to directly store a pointer to const struct
 * fsDevOperations which are associated with an open file.
 */
static bool     devStat(const char * restrict path,
                        struct fsStat * restrict buf);
static void    *devOpen(const char *path, fsMode_t mode);
static ssize_t  devRead(void * restrict id, void * restrict buf, size_t count);
static fsOff_t  devTell(void *id);
static void     devSeek(void *id, fsOff_t pos);
static ssize_t  devWrite(void * restrict id, const void * restrict buf,
                         size_t count);
static bool     devFlush(void *id);
static bool     devClose(void *id);
static bool     devRemove(const char *path);
static bool     devRename(const char *old, const char *new);

const struct fsOperations fsDevOps = {
    .stat    = devStat,
    .open    = devOpen,
    .read    = devRead,
    .tell    = devTell,
    .seek    = devSeek,
    .write   = devWrite,
    .flush   = devFlush,
    .close   = devClose,
    .remove  = devRemove,
    .rename  = devRename
};

void fsDevInit(void)
{

}

/* Find a device file */
static const struct fsDevEntry *fsFindDevEntry(const char *name)
{
    size_t i;

    for (i = 0; i < sizeof(fsDevEntries) / sizeof(struct fsDevEntry); i++)
    {
        if (!strcmp(fsDevEntries[i].name, name))
            return fsDevEntries + i;
    }

    fsSetError(FS_ERR_NOENT);
    return NULL;
}


/*
 * Define interface functions
 */

static bool devStat(const char * restrict path, struct fsStat * restrict buf)
{
    if (!fsFindDevEntry(path))
        return false;
    else
    {
        buf->size = 0;
        return true;
    }
}

static void *devOpen(const char *path, fsMode_t mode)
{
    const struct fsDevEntry *entry;

    if (mode != FS_RDWR) /* Disallow any other mode flags */
    {
        fsSetError(FS_ERR_INVAL);
        return NULL;
    }

    entry = fsFindDevEntry(path);
    if (entry)
        return entry->ops->open() ? (void *)entry->ops : NULL;
    else
        return NULL;
}

static ssize_t devRead(void * restrict id, void * restrict buf, size_t count)
{
    const struct fsDevOperations *ops = id;
    return ops->read(buf, count);
}

static ssize_t devWrite(void * restrict id, const void * restrict buf,
                          size_t count)
{
    const struct fsDevOperations *ops = id;
    return ops->write(buf, count);
}

static bool devClose(void *id)
{
    const struct fsDevOperations *ops = id;
    return ops->close();
}

/*
 * Now define remaining functions that don't do anything.
 *
 * TODO: Maybe part of them could bear some meaning.
 */

static fsOff_t devTell(void *id)
{
    (void)id;
    return 0;
}

static void devSeek(void *id, fsOff_t pos)
{
    (void)id; (void)pos;
}

static bool devFlush(void *id)
{
    (void)id;
    return true;
}

static bool devRemove(const char *path)
{
    (void)path;
    fsSetError(FS_ERR_NOSYS);
    return false;
}

static bool devRename(const char *old, const char *new)
{
    (void)old; (void)new;
    fsSetError(FS_ERR_NOSYS);
    return false;
}
