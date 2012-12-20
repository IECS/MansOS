/**
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

#ifndef MANSOS_POSIX_FILE_H
#define MANSOS_POSIX_FILE_H

#include <kernel/defines.h>
#include "structures.h"

// End-of-file character
#ifndef EOF
# define EOF (-1)
#endif

// defines for fseek() function
#ifndef SEEK_SET
# define SEEK_SET  0  // Seek from beginning of file.
# define SEEK_CUR  1  // Seek from current position.
# define SEEK_END  2  // Seek from end of file.
#endif

//
// File structure. It has a lot of file system-specific information!
//
struct FILE_s {
    // file descriptor, -1 if not opened
    int16_t fd;
    // file size in bytes
    uint32_t fileSize;
    // firts cluster of the file
    cluster_t firstCluster;
    // current cluster (on disk)
    cluster_t currentCluster;
    // directory entry index
    uint16_t directoryEntry;
    // read/write position
    uint32_t position;
    // O_RDONLY, O_WRONLY etc.
    uint16_t flags;
    // EOF indicator set etc.
    uint8_t state;
    // whether the directory entry has been modified
    bool dirEntryDirty;
};

// MansOS file typedef
typedef struct FILE_s MFILE;

// For MCU platforms also typedef FILE
#if !PLATFORM_PC
typedef struct FILE_s FILE;
#else
#include <stdio.h>
#endif

// File access flags
#define O_RDONLY    00000000
#define O_WRONLY    00000001
#define O_RDWR      00000002
#define O_CREAT     00000100
#define O_EXCL      00000200
#define O_TRUNC     00001000
#define O_APPEND    00002000

#define O_WRITE     (O_WRONLY | O_RDWR)


#endif
