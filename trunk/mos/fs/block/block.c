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
 * fs/block/block.c -- block storage subsystem
 */

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <fs/prefix.h>
#include <fs/common.h>
#include <fs/types.h>
#include <defines.h>
#include <assert.h>
#include <lib/codec/crc.h>

#include "alloc.h"
#include "common.h"
#include "flash_access.h"
#include "init.h"
#include "meta.h"

/* See fillBuffer() for the reason of this value */
#define MAX_FILESIZE ((fsOff_t)-1 - (fsOff_t)-1 % CHUNK_DATA_SIZE)

static void    *blkOpen(const char *path, fsMode_t mode);
static ssize_t  blkRead(void * restrict id, void * restrict buf, size_t count);
static fsOff_t  blkTell(void *id);
static void     blkSeek(void *id, fsOff_t pos);
static ssize_t  blkWrite(void * restrict id, const void * restrict buf,
                         size_t count);
static bool     blkFlush(void *id);
static bool     blkClose(void *id);

const struct fsOperations fsBlockOps = {
    .stat   = fsBlockStat,
    .open   = blkOpen,
    .read   = blkRead,
    .tell   = blkTell,
    .seek   = blkSeek,
    .write  = blkWrite,
    .flush  = blkFlush,
    .close  = blkClose,
    .remove = fsBlockRemove,
    .rename = fsBlockRename
};

/*
 * A handle for open files. Multiple handles can refer to the same file control
 * block.
 */
struct fsBlockHandle {
    struct fsFileControlBlock *fcb;
    fsMode_t                   mode;
    fsOff_t                    pos;
    blk_t                      curr;    /* Current block */
    char                       buf[CHUNK_DATA_SIZE];
    fsOff_t                    readEnd; /* Stores the end of the read buffer */
};

/* Array of handles */
static struct fsBlockHandle blkHandles[FS_MAX_OPEN_FILES];
mos_mutex_t                 fsBlkHandleMutex;

/* Allocate a handle */
static struct fsBlockHandle *allocHandle(struct fsFileControlBlock *fcb)
{
    struct fsBlockHandle *res = NULL;
    size_t                i;

    mos_mutex_lock(&fsBlkHandleMutex);
    for (i = 0; i < FS_MAX_OPEN_FILES; i++)
    {
        if (!blkHandles[i].fcb)
        {
            res = blkHandles + i;
            res->fcb = fcb;
            break;
        }
    }
    mos_mutex_unlock(&fsBlkHandleMutex);

    return res;
}

/* Free a handle */
static void freeHandle(struct fsBlockHandle *handle)
{
    mos_mutex_lock(&fsBlkHandleMutex);
    handle->fcb = NULL;
    mos_mutex_unlock(&fsBlkHandleMutex);
}

/* Should be called with the mutex locked. */
static void seekBlock(struct fsBlockHandle *handle, fsOff_t pos)
{
    /*
     * Note that if pos % BLOCK_DATA_SIZE == 0, we stop one block before, since
     * the block will be advanced on next read/write operation.
     */

    handle->curr = handle->fcb->first;
    while (pos > BLOCK_DATA_SIZE)
    {
        handle->curr = fsBlockGetNext(handle->curr);
        pos -= BLOCK_DATA_SIZE;
    }
}

static void *blkOpen(const char *path, fsMode_t mode)
{
    struct fsFileControlBlock *fcb;

    if (mode & FS_RDWR || !(mode & (FS_READ | FS_APPEND)) ||
        (mode & FS_READ && mode & FS_APPEND))
    {
        fsSetError(FS_ERR_INVAL);
        return NULL;
    }

    fcb = fsBlockOpenFile(path, mode & FS_APPEND);
    if (fcb)
    {
        struct fsBlockHandle *handle = allocHandle(fcb);
        if (handle)
        {
            handle->mode = mode;

            mos_mutex_lock(&fcb->mutex);
            if (mode & FS_READ) /* Read mode */
            {
                handle->pos     = 0;
                handle->curr    = fcb->first;
                handle->readEnd = 0;
            }
            else /* Write mode */
            {
                /* Traverse blocks until the last one */
                seekBlock(handle, fcb->size);

                handle->pos = fcb->size;
            }
            mos_mutex_unlock(&fcb->mutex);
        }
        else
            fsBlockCloseFile(fcb);
        return handle;
    }
    else
        return NULL;
}

/* TODO: Move it. */
static uint16_t crc16Acc(const void *data, size_t len, uint16_t acc)
{
    const uint8_t *d = data;
    size_t         i;

    for (i = 0; i < len; i++)
        acc = crc16Add(acc, d[i]);

    return acc;
}

/* Should be called with the mutex locked. */
static void doRead(struct fsBlockHandle * restrict handle, fsOff_t start,
                   void * restrict buf, size_t len)
{
    if (start != 0 && start % BLOCK_DATA_SIZE == 0)
        handle->curr = fsBlockGetNext(handle->curr);

    blkExtFlashRead(chunkAddr(handle->curr, start), buf, len);
}

/*
 * Fill the buffer, possibly compare checksums. Should be called with the mutex
 * locked.
 */
static bool fillBuffer(struct fsBlockHandle *handle)
{
    blk_t   backup = handle->curr;
    fsOff_t end = handle->readEnd - handle->readEnd % CHUNK_DATA_SIZE
                  + CHUNK_DATA_SIZE;
    if (end > handle->fcb->size)
        end = handle->fcb->size;

    doRead(handle, handle->readEnd,
           handle->buf + handle->readEnd % CHUNK_DATA_SIZE,
           end - handle->readEnd);

    if (handle->mode & FS_CHECKSUM)
    {
        uint16_t crc;

        if (handle->fcb->size / CHUNK_DATA_SIZE
            == handle->readEnd / CHUNK_DATA_SIZE)
        {
            /* This is an unfinished chunk */
            crc = handle->fcb->crc;
        }
        else
        {
            blkExtFlashRead(serviceAddr(handle->curr, handle->readEnd),
                            &crc, sizeof(crc));
        }

        if (crc16Acc(handle->buf, end - handle->readEnd, 0) != crc)
        {
            fsSetError(FS_ERR_IO);
            /* In case the block number was changed by doRead() */
            handle->curr = backup;
            return false;
        }
    }

    handle->readEnd = end;

    return true;
}

static ssize_t blkRead(void * restrict id, void * restrict buf, size_t count)
{
    struct fsBlockHandle * restrict handle = id;
    ssize_t                         res;
    ASSERT(handle->mode & FS_READ);

    mos_mutex_lock(&handle->fcb->mutex);

    count = MIN(count, handle->fcb->size - handle->pos);
    res = count = MIN(count, CHUNK_DATA_SIZE - handle->pos % CHUNK_DATA_SIZE);

    if (count != 0)
    {
        if (!(handle->mode & FS_NOCACHE))
        {
            /* Cached version */

            if (handle->pos == handle->readEnd)
            {
                if (!fillBuffer(handle))
                    res = -1;
            }
            if (res != -1)
            {
                memcpy(buf, handle->buf + handle->pos % CHUNK_DATA_SIZE, count);
                handle->pos += count;
            }
        }
        else
        {
            /* Non-cached version */

            doRead(handle, handle->pos, buf, count);
            handle->pos += count;
        }
    }

    mos_mutex_unlock(&handle->fcb->mutex);

    return res;
}

static fsOff_t blkTell(void *id)
{
    struct fsBlockHandle *handle = id;

    return handle->pos;
}

static void blkSeek(void *id, fsOff_t pos)
{
    struct fsBlockHandle *handle = id;
    ASSERT(handle->mode & FS_READ);

    mos_mutex_lock(&handle->fcb->mutex);
    handle->pos = MIN(handle->fcb->size, pos);
    seekBlock(handle, handle->pos);
    mos_mutex_unlock(&handle->fcb->mutex);

    /* Buffer should be re-read */
    if (handle->mode & FS_CHECKSUM)
        handle->readEnd = handle->pos - handle->pos % CHUNK_DATA_SIZE;
    else
        handle->readEnd = handle->pos;
}

/* Should be called with the mutex locked. */
static void doWrite(struct fsBlockHandle * restrict handle, fsOff_t start,
                    const void * restrict buf, size_t len)
{
    if (start % BLOCK_DATA_SIZE == 0)
    {
        /* We don't want to do this often for performance reasons */
        fsBlockSyncEntry(handle->fcb);
    }

    blkExtFlashWrite(chunkAddr(handle->curr, start), buf, len);
}

/*
 * Write the buffer to the flash. Should be called with the mutex locked.
 */
static void doFlush(struct fsBlockHandle *handle)
{
    if (handle->fcb->size == handle->pos) /* Nothing to do */
        return;

    /* We operate only on one chunk at a time */
    ASSERT(handle->fcb->size / CHUNK_DATA_SIZE
           == (handle->pos - 1) / CHUNK_DATA_SIZE);

    doWrite(handle, handle->fcb->size,
            handle->buf + handle->fcb->size % CHUNK_DATA_SIZE,
            handle->pos - handle->fcb->size);

    if (handle->mode & FS_CHECKSUM)
    {
        handle->fcb->crc =
            crc16Acc(handle->buf + handle->fcb->size % CHUNK_DATA_SIZE,
                     handle->pos - handle->fcb->size, handle->fcb->crc);
        if (handle->pos % CHUNK_DATA_SIZE == 0) /* Ending this chunk */
        {
            blkExtFlashWrite(serviceAddr(handle->curr, handle->fcb->size),
                             &handle->fcb->crc, sizeof(handle->fcb->crc));
            handle->fcb->crc = 0;
        }
    }
    handle->fcb->size = handle->pos;
}

static ssize_t blkWrite(void * restrict id, const void * restrict buf,
                        size_t count)
{
    struct fsBlockHandle * restrict handle = id;
    ssize_t                         res = 0;
    ASSERT(handle->mode & FS_APPEND);

    if (count == 0)
        return 0;

    mos_mutex_lock(&handle->fcb->mutex);

    count = MIN(count, MAX_FILESIZE - handle->pos);
    if (count == 0)
    {
        /* File has maximum size */
        fsSetError(FS_ERR_NOSPC);
        res = -1;
    }
    else
    {
        /* Check if we need to allocate new block */
        if (handle->pos % BLOCK_DATA_SIZE == 0)
        {
            blk_t new = fsBlockAllocate(handle->curr);
            if (new == BLOCK_INVAL)
                res = -1;
            else
            {
                handle->curr = new;
                if (handle->fcb->first == BLOCK_INVAL)
                    handle->fcb->first = handle->curr;
            }
        }

        /* Write data */
        if (res != -1)
        {
            res = count = MIN(count,
                              CHUNK_DATA_SIZE - handle->pos % CHUNK_DATA_SIZE);

            if (!(handle->mode & FS_NOCACHE))
            {
                /* Cached version, use the buffer */

                memcpy(handle->buf + handle->pos % CHUNK_DATA_SIZE, buf, count);

                handle->pos += count;
                if (handle->pos % CHUNK_DATA_SIZE == 0) /* Buffer is full */
                    doFlush(handle);
            }
            else
            {
                /* Non-cached version, write everything directly to flash */

                doWrite(handle, handle->pos, buf, count);

                handle->pos += count;
                handle->fcb->size = handle->pos;
            }
        }
    }

    mos_mutex_unlock(&handle->fcb->mutex);

    return res;
}

static bool blkFlush(void *id)
{
    struct fsBlockHandle *handle = id;

    ASSERT(handle->mode & FS_APPEND);

    mos_mutex_lock(&handle->fcb->mutex);
    if (!(handle->mode & FS_NOCACHE))
        doFlush(handle);
    fsBlockSyncEntry(handle->fcb);
    mos_mutex_unlock(&handle->fcb->mutex);

    return true;
}

static bool blkClose(void *id)
{
    struct fsBlockHandle *handle = id;

    if (handle->mode & FS_APPEND)
    {
        mos_mutex_lock(&handle->fcb->mutex);
        if (!(handle->mode & FS_NOCACHE))
            doFlush(handle);
        handle->fcb->openForWrite = false;
        mos_mutex_unlock(&handle->fcb->mutex);
    }

    fsBlockCloseFile(handle->fcb);
    freeHandle(handle);

    return true;
}
