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

#ifndef MANSOS_POSIX_STDIO_H
#define MANSOS_POSIX_STDIO_H

/// \file
/// POSIX-compatible high level file routines
///
/// Availabe on on platforms with SD card, including PC.
/// Uses FAT file system internally.
///
/// Note: fprintf() is not provided because of efficiency issues!
/// Use fputs() combined with sprintf() in an application-specific buffer.
///

#include "posix-file.h"

//! Open a file
FILE *fopen(const char *restrict filename,
            const char *restrict modes);

//! Close a file
int fclose(FILE *fp);

//! Read from the file
size_t fread(void *restrict ptr, size_t size,
             size_t n, FILE *restrict fp);

///
/// fgetc() reads the next character from stream and returns it as an unsigned
/// char cast to an int, or EOF on end of file or error.
///
static inline int fgetc(FILE *fp)
{
    uint8_t b;
    if (fread(&b, 1, 1, fp) != 1) return EOF;
    return (int) b;
}

///
/// fgets()  reads  in  at  most  one  less than size characters from stream and
/// stores them into the buffer pointed to by s.  Reading stops after an EOF  or
/// a newline.  If a newline is read, it is stored into the buffer.
/// A terminating null byte ('\0') is stored after the last character in the buffer.
///
char *fgets(char *restrict s, int size, FILE *restrict fp);

//! Write to the file
size_t fwrite(const void *restrict ptr, size_t size,
              size_t n, FILE *restrict fp);

//! Put a character in the file
static inline int fputc(int c, FILE *fp)
{
    uint8_t b = (uint8_t) c;
    return fwrite(&b, 1, 1, fp) == 1 ? b : EOF;
}

//! Put a string to the file
static inline int fputs(const char *restrict s, FILE *restrict fp)
{
    uint16_t length = strlen(s);
    if (fwrite(s, 1, length, fp) != length) return EOF;
    return (int) length;
}

//! Tell the current position in the file
static inline long ftell(FILE *fp)
{
    return fp->position;
}

//! Rewind the current position back to the start
static inline void rewind(FILE *fp)
{
    fp->position = 0;
    fp->currentCluster = fp->firstCluster;
}

//! Change the current position in the file
int fseek(FILE *fp, long offset, int whence);

///
/// Returns true if end of file is reached.
///
/// Note: as opposed to POSIX, end-of-file flag is not set.
/// EOF condition can be reverted by simply fseek()'ing back in the file.
///
static inline int feof(FILE *fp)
{
    return fp->position == -1;
}

//! Flush all active buffers to SD card
int fflush(FILE *fp);

//! Delete a file
int remove(const char *filename);


// Initialization routine (internal use only!)
void posixStdioInit(void);


#endif
