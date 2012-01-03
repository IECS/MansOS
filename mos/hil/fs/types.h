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
 * fs/types.h -- file system type definitions
 */

#ifndef _FS_TYPES_H_
#define _FS_TYPES_H_

#include <stdint.h>

/* Error enumeration */
typedef enum {
    FS_ERR_OK,    /* No error */
    FS_ERR_NOMEM, /* No free slot or memory space */
    FS_ERR_IO,    /* Input/output error */
    FS_ERR_NOSPC, /* No space left on device */
    FS_ERR_NOENT, /* No such entity */
    FS_ERR_EXIST, /* Name already exists */
    FS_ERR_NOSYS, /* Function not implemented */
    FS_ERR_INVAL, /* Invalid parameter */
    FS_ERR_OPEN,  /* File is open */
} fsError_t;

/* Unsigned type capable of representing any position in a file */
typedef uint32_t fsOff_t;

/* Holds information about files */
struct fsStat {
    fsOff_t size;
};

/* File open modes */
typedef enum {
    FS_READ     = 1 << 0,  /* Open for reading */
    FS_APPEND   = 1 << 1,  /* Open for appending */
    FS_RDWR     = 1 << 2,  /* Open for reading and writing */
    FS_NOCACHE  = 1 << 3,  /* Disable buffering */
    FS_CHECKSUM = 1 << 4   /* Calculate and store data checksums, incompatible
                              with FS_NOCACHE */
} fsMode_t;

/* Temp fix to make filesystem code compile with new threads - Atis */
#include <kernel/threads/mutex.h>
typedef Mutex_t mos_mutex_t;
#define mos_mutex_init    mutexInit
#define mos_mutex_lock    mutexLock
#define mos_mutex_unlock  mutexUnlock

#endif /* _FS_TYPES_H_ */
