/*
 * Copyright (c) 2012-2013 the MansOS team. All rights reserved.
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

#include "fatfs.h"
#include "posix-stdio.h"
#include <errors.h>
#include <lib/dprint.h>

#ifndef MAX_OPEN_FILES
#define MAX_OPEN_FILES 2
#endif

static FILE openFiles[MAX_OPEN_FILES];

void posixStdioInit(void)
{
    int i;
    for (i = 0; i < MAX_OPEN_FILES; ++i) {
        openFiles[i].fd = -1;
    }
}

// flush a file
int fflush(FILE *fp)
{
#if USE_FATFS && USE_SDCARD
    fatFsFileFlush(fp);
#endif
    return 0;
}

// open a file
FILE *fopen(const char *restrict filename,
            const char *restrict modes)
{
#if USE_FATFS && USE_SDCARD
    int i;
    for (i = 0; i < MAX_OPEN_FILES; ++i) {
        if (openFiles[i].fd == -1) break;
    }
    if (i == MAX_OPEN_FILES) {
        // file limit reached
        errno = ENFILE;
        return NULL;
    }

    FILE *result = &openFiles[i];
    result->flags = 0;
    while (*modes) {
        switch (*modes) {
        case 'r':
            result->flags |= O_RDONLY;
            break;
        case 'w':
            result->flags |= O_WRONLY | O_TRUNC | O_CREAT;
            break;
        case 'a':
            result->flags |= O_WRONLY | O_CREAT | O_APPEND;
            break;
        case '+':
            result->flags &= ~(O_WRONLY | O_RDONLY);
            result->flags |= O_RDWR;
            break;
        case 't':
        case 'b':
            break;
        }
        modes++;
    }

    // open it on file system
    DirectoryEntry_t *de = fatFsFileSearch(filename, &result->directoryEntry);
    if (de == NULL) {
        if (result->flags & O_CREAT) {
            de = fatFsFileCreate(filename, &result->directoryEntry);
        }
    }

    if (de == NULL) {
        return NULL;
    }

#if FAT32_SUPPORT
    result->firstCluster = de->startClusterHiword;
    result->firstCluster <<= 16;
#else
    result->firstCluster = 0;
#endif
    result->firstCluster |= de->startClusterLoword;
    result->currentCluster = result->firstCluster;
    result->position = 0;
    if (result->flags & O_TRUNC) {
        // resize the file to zero
        result->fileSize = 0;
        if (de->fileSize != 0) {
            // write the new size to disk
            result->dirEntryDirty = true;
            fatFsFileFlush(result);
        }
    } else {
        result->dirEntryDirty = false;
        result->fileSize = de->fileSize;
    }
    // PRINTF("file opened, size=%lu\n", result->fileSize);
    result->fd = 1; // XXX

    // TODO: truncate if needed

    return result;
#else
    return NULL;
#endif
}

// close a file
int fclose(FILE *fp)
{
#if USE_FATFS && USE_SDCARD
    // close it on the file system
    fatFsFileClose(fp);
#endif

    fp->fd = -1;

    return -1;
}

// read from a file
size_t fread(void *restrict ptr, size_t size,
             size_t n, FILE *restrict fp)
{
#if USE_FATFS && USE_SDCARD
    return fatFsRead(fp, ptr, size * n);
#else
    return 0;
#endif
}

// write to a file
size_t fwrite(const void *restrict ptr, size_t size,
              size_t n, FILE *restrict fp)
{
#if USE_FATFS && USE_SDCARD
    return fatFsWrite(fp, ptr, size * n);
#else
    return 0;
#endif
}

// delete a file
int remove(const char *filename)
{
#if USE_FATFS && USE_SDCARD
    fatFsFileRemove(filename);
#endif
    return 0;
}

// go to a specific position in the file
int fseek(FILE *fp, long offset, int whence)
{
    uint32_t newPos;
    switch (whence) {
    case SEEK_SET:
        newPos = 0;
        break;
    case SEEK_CUR:
        newPos = fp->position;
        break;
    case SEEK_END:
    default:
        newPos = fp->fileSize;
        break;
    }
    if (offset < 0 && newPos < -offset) {
        // wants before the start of the file
        return -1;
    }
    if (offset > 0 && newPos + offset > fp->fileSize) {
        // wants after the end of the file
        return -1;
    }
    // fine; do it!
    newPos += offset;
#if USE_FATFS && USE_SDCARD
    fatfsGoToPosition(fp, newPos);
#endif
    return 0;
}
