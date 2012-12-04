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

#ifndef MANSOS_POSIX_STDIO_H
#define MANSOS_POSIX_STDIO_H

//
// POSIX-compatbile high level file routines
//

//#include "fatfs.h"

#include <kernel/defines.h>

typedef struct FILE_s {
    // file descriptor, -1 if not opened
    int16_t fd;
    // read/write position in the opened file
    uint32_t pos;
} FILE;

// open a file
FILE *fopen(const char *__restrict filename,
            const char *__restrict modes);

// close a file
int fclose(FILE *fp);

// flush a file
int fflush (FILE *fp);

// read from a file
size_t fread(void *__restrict ptr, size_t size,
             size_t n, FILE *__restrict fp);

// write to a file
size_t fwrite(const void *__restrict ptr, size_t size,
              size_t n, FILE *__restrict fp);

// delete a file
int remove (const char *filename);


//
// Initialization routine (interal use only!)
//
void posixStdioInit(void);


#endif
