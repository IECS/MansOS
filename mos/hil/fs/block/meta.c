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
 *
 * fs/block/meta.c -- control structures and root directory
 */

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <hil/alarms.h>
#include <hil/eeprom.h>
#include <hil/extflash.h>
#include <hil/fs/common.h>
#include <hil/fs/types.h>
#include <kernel/defines.h>

#include "alloc.h"
#include "init.h"
#include "common.h"
#include "flash_access.h"

#define MAGIC 0x8550 /* Chosen by fair dice rolls */

/* File entry in EEPROM */
#define MAX_NAMELEN 7
struct fsFileEntry {
    char     name[MAX_NAMELEN];
    blk_t    first;
    fsOff_t  size;
    uint8_t  attr;
    uint16_t lastCrc;
} PACKED;

/* Offset to the root directory in EEPROM */
#define ROOTDIR_OFFSET (BLKTABLE_OFFSET + BLKTABLE_SIZE)

/* Number of file entries in EEPROM */
#define NUM_FILE_ENTRIES \
    ((EEPROM_SIZE - ROOTDIR_OFFSET) / sizeof(struct fsFileEntry))

/* Array of file control blocks */
static struct fsFileControlBlock fcbs[FS_MAX_OPEN_FILES];

/* Protect the root directory from simultaneous accesses */
static mos_mutex_t rootMutex;

/* Flash sleep timer */
Alarm_t fsBlockFlashAlarm;

/* Process flash timer event */
void fsBlockAlarmCallback(void *data)
{
    (void)data;

    extFlashSleep(); /* TODO: I hope this is safe to call from interrupt */
}

static inline void fsBlockDelEntry(int8_t id);

void fsBlockInit(void)
{
    size_t   i;
    uint16_t check;

    eepromRead(0, &check, sizeof(check));
    if (check != MAGIC)
    {
        /* There is no our signature. Let's initialize an empty filesystem. */

        /* Mark all blocks as available */
        fsBlockFreeAll();

        /* Mark all file entries free */
        for (i = 0; i < NUM_FILE_ENTRIES; i++)
            fsBlockDelEntry(i);

        /* Write magic */
        check = MAGIC;
        eepromWrite(0, &check, sizeof(check));
    }

    mos_mutex_init(&rootMutex);
    mos_mutex_init(&fsBlkTableMutex);
    mos_mutex_init(&fsBlkHandleMutex);

    alarmInit(&fsBlockFlashAlarm, fsBlockAlarmCallback, NULL);

    /* Initialize the file control block array */
    for (i = 0; i < FS_MAX_OPEN_FILES; i++)
    {
        mos_mutex_init(&fcbs[i].mutex);
        fcbs[i].id = -1;
    }
}

/* EEPROM address of a field @f in a file entry with number @id */
#define FIELD_ADDR(id, f) \
    (ROOTDIR_OFFSET + id * sizeof(struct fsFileEntry) \
     + offsetof(struct fsFileEntry, f))

/*
 * Find a file entry by name. Empty name searches for a free file entry.
 * Must be called with rootMutex held.
 */
static int8_t fsBlockFindEntry(const char *name)
{
    char   s[MAX_NAMELEN];
    int8_t i;

    for (i = 0; i < NUM_FILE_ENTRIES; i++)
    {
        eepromRead(FIELD_ADDR(i, name), s, MAX_NAMELEN);
        if (!strncmp(name, s, MIN(strlen(name) + 1, MAX_NAMELEN)))
            return i;
    }

    fsSetError(*name ? FS_ERR_NOENT : FS_ERR_NOMEM);
    return -1;
}

/* Change entry name. Must be called with rootMutex held. */
static inline void fsBlockRenameEntry(int8_t id, const char *name)
{
    eepromWrite(FIELD_ADDR(id, name), name, MIN(strlen(name) + 1, MAX_NAMELEN));
}


/* Should be called with rootMutex held, except for fsBlockInit() */
static inline void fsBlockDelEntry(int8_t id)
{
    fsBlockRenameEntry(id, "");
}

/*
 * Find a matching FCB and return it locked. If locking is not performed, then
 * it's possible that the FCB disappears if another thread unallocates it in the
 * mean time.
 */
static struct fsFileControlBlock *findAndLockFcb(int8_t id)
{
    size_t i;

    for (i = 0; i < FS_MAX_OPEN_FILES; i++)
    {
        mos_mutex_lock(&fcbs[i].mutex);
        if (fcbs[i].id == id)
            return fcbs + i;
        mos_mutex_unlock(&fcbs[i].mutex);
    }

    if (id == -1)
        fsSetError(FS_ERR_NOMEM);
    return NULL;
}

/* Allocate a file control block from array */
static inline struct fsFileControlBlock *allocFcb(void)
{
    return findAndLockFcb(-1);
}

/* Open or create file, allocating a file control block */
struct fsFileControlBlock *fsBlockOpenFile(const char *name, bool write)
{
    struct fsFileControlBlock *res = NULL;
    int8_t                     entry;
    bool                       newfile = false;

    if (strlen(name) > MAX_NAMELEN)
    {
        fsSetError(FS_ERR_INVAL);
        return NULL;
    }

    mos_mutex_lock(&rootMutex);

    entry = fsBlockFindEntry(name);
    if (entry != -1)
    {
        /* Check if the file is already open */
        res = findAndLockFcb(entry);
        if (res)
        {
            if (write && res->openForWrite)
            {
                /* Can't have two writers */
                fsSetError(FS_ERR_OPEN);
                mos_mutex_unlock(&res->mutex);
                entry = -1; res = NULL;
            }
            else
            {
                res->refcount++;
                if (write)
                    res->openForWrite = true;
            }
        }
    }
    else if (write) /* Else create it if allowed */
    {
        entry = fsBlockFindEntry("");
        newfile = true;
    }
    if (entry != -1 && !res) /* Allocate FCB if needed */
    {
        res = allocFcb();
        if (res)
        {
            res->id           = entry;
            res->refcount     = 1;
            res->openForWrite = write;

            if (newfile)
            {
                res->size  = 0;
                res->first = BLOCK_INVAL;
                res->crc   = 0;
                fsBlockRenameEntry(entry, name);
            }
            else
            {
                eepromRead(FIELD_ADDR(entry, size),
                           &res->size, sizeof(res->size));
                eepromRead(FIELD_ADDR(entry, first),
                           &res->first, sizeof(res->first));
                eepromRead(FIELD_ADDR(entry, lastCrc),
                           &res->crc, sizeof(res->crc));
            }
        }
    }
    if (res)
        mos_mutex_unlock(&res->mutex);

    mos_mutex_unlock(&rootMutex);

    return res;
}

void fsBlockCloseFile(struct fsFileControlBlock *file)
{
    mos_mutex_lock(&file->mutex);

    if (--file->refcount == 0)
    {
        /* There are no more handles referring to this file */

        fsBlockSyncEntry(file);
        file->id = -1; /* Make it available for allocation again */
    }

    mos_mutex_unlock(&file->mutex);
}

void fsBlockSyncEntry(struct fsFileControlBlock *file)
{
    /*
     * Locking @rootMutex is not necessary here, because no action is
     * performed on file names.
     */
    eepromWrite(FIELD_ADDR(file->id, size), &file->size, sizeof(file->size));
    eepromWrite(FIELD_ADDR(file->id, first), &file->first, sizeof(file->first));
    eepromWrite(FIELD_ADDR(file->id, lastCrc), &file->crc, sizeof(file->crc));
}

bool fsBlockStat(const char * restrict path, struct fsStat * restrict buf)
{
    int8_t entry;

    if (strlen(path) > MAX_NAMELEN)
    {
        fsSetError(FS_ERR_NOENT);
        return false;
    }

    mos_mutex_lock(&rootMutex);
    entry = fsBlockFindEntry(path);
    if (entry != -1)
    {
        struct fsFileControlBlock *file = findAndLockFcb(entry);
        if (file)
        {
            buf->size = file->size;
            mos_mutex_unlock(&file->mutex);
        }
        else
            eepromRead(FIELD_ADDR(entry, size), &buf->size, sizeof(buf->size));
    }
    mos_mutex_unlock(&rootMutex);

    return entry != -1;
}

bool fsBlockRemove(const char *path)
{
    int8_t entry;

    if (strlen(path) > MAX_NAMELEN)
    {
        fsSetError(FS_ERR_NOENT);
        return false;
    }

    mos_mutex_lock(&rootMutex);
    entry = fsBlockFindEntry(path);
    if (entry != -1)
    {
        struct fsFileControlBlock *file = findAndLockFcb(entry);
        if (file)
        {
            /* File is open */
            mos_mutex_unlock(&file->mutex);
            fsSetError(FS_ERR_OPEN);
            entry = -1;
        }
        else
        {
            fsOff_t size;
            blk_t   curr, next;

            eepromRead(FIELD_ADDR(entry, size), &size, sizeof(size));
            eepromRead(FIELD_ADDR(entry, first), &curr, sizeof(curr));

            /* Walk the file and delete its blocks */
            while (size)
            {
                next = fsBlockGetNext(curr);
                fsBlockFree(curr);
                curr = next;
                size -= MIN(BLOCK_DATA_SIZE, size);
            }

            fsBlockDelEntry(entry);
        }
    }
    mos_mutex_unlock(&rootMutex);

    return entry != -1;
}

bool fsBlockRename(const char *old, const char *new)
{
    int8_t entry;

    if (strlen(old) > MAX_NAMELEN)
    {
        fsSetError(FS_ERR_NOENT);
        return false;
    }
    else if (strlen(new) > MAX_NAMELEN)
    {
        fsSetError(FS_ERR_INVAL);
        return false;
    }

    mos_mutex_lock(&rootMutex);
    entry = fsBlockFindEntry(old);
    if (entry != -1)
    {
        if (fsBlockFindEntry(new) != -1)
        {
            /* File with the new name already exists */
            fsSetError(FS_ERR_EXIST);
            entry = -1;
        }
        else
            fsBlockRenameEntry(entry, new);
    }
    mos_mutex_unlock(&rootMutex);

    return entry != -1;
}
