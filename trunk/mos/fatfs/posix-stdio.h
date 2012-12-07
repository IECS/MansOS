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
// POSIX-compatible high level file routines
//

#include "posix-file.h"

// open a file
FILE *fopen(const char *__restrict filename,
            const char *__restrict modes);

// close a file
int fclose(FILE *fp);

// flush a file
int fflush(FILE *fp);

// read from a file
size_t fread(void *__restrict ptr, size_t size,
             size_t n, FILE *__restrict fp);

//
// fgetc(3):
// fgetc() reads the next character from stream and returns it as an unsigned
// char cast to an int, or EOF on end of file or error.
//
static inline int fgetc(FILE *fp)
{
    uint8_t b;
    if (fread(&b, 1, 1, fp) != 1) return EOF;
    return (int) b;
}

//
// fgets(3):
// fgets()  reads  in  at  most  one  less than size characters from stream and
// stores them into the buffer pointed to by s.  Reading stops after an EOF  or
// a newline.  If a newline is read, it is stored into the buffer.  A terminat‐
// ing null byte ('\0') is stored after the last character in the buffer.
//
//
static inline char *fgets(char *__restrict s, int size, FILE *__restrict fp)
{
    uint16_t i;
    if (size == 0) return s;
    for (i = 0; i < size - 1; ++i) {
        uint8_t b;
        fread(&b, 1, 1, fp);
        if (b == EOF) {
            if (i == 0) return NULL;
            break;
        }
        s[i] = b;
        if (b == '\n') break;
    }
    i++;
    s[i] = '\0';
    return s;
}

// write to a file
size_t fwrite(const void *__restrict ptr, size_t size,
              size_t n, FILE *__restrict fp);

static inline int fputc(int c, FILE *fp)
{
    uint8_t b = (uint8_t) c;
    return fwrite(&b, 1, 1, fp) == 1 ? b : EOF;
}

static inline int fputs(const char *__restrict s, FILE *__restrict fp)
{
    uint16_t length = strlen(s);
    if (fwrite(s, 1, length, fp) != length) return EOF;
    return (int) length;
}

// tell current position in a file
static inline long ftell(FILE *fp)
{
    return fp->position;
}

static inline void rewind(FILE *fp)
{
    fp->position = 0;
    fp->currentCluster = fp->firstCluster;
}

static inline int fseek(FILE *fp, long offset, int whence)
{
    // only position from start supported
    if (whence != SEEK_SET) return -1;
    
    // onlye zero offset supported
    if (offset != 0) return -1;

    rewind(fp);

    return 0;
}

static inline int feof(FILE *fp)
{
    return fp->position == -1;
}

// delete a file
int remove (const char *filename);

//
// Initialization routine (interal use only!)
//
void posixStdioInit(void);


#endif
