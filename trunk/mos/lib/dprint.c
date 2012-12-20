/**
 * Copyright (c) 2008-2012 the MansOS team. All rights reserved.
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

#include "dprint.h"
#include <string.h>
#include <stdarg.h>

extern int vsnprintf(char *str, size_t size, const char *format, va_list ap);

static char *_print_buf;

void printInitReal(void)
{
    PRINT_INIT(PRINT_BUFFER_SIZE);
}

void debugPrintf(PrintFunction_t outputFunction, const char* str, ...)
{
    va_list args;

    if (!_print_buf) return;

    va_start(args, str);
#if USE_PUTCHAR
    vprintf(str, args);
#else
    vsnprintf(_print_buf, PRINT_BUFFER_SIZE, str, args);
    outputFunction(_print_buf);
#endif
}

#if defined DEBUG || defined ENABLE_DEBUG_HEXDUMP
#if 1
// hexdump version #1
void debugHexdump(void *data_, unsigned len) {
    static const char digits[] = "0123456789abcdef";
    uint8_t *data = (uint8_t *) data_;
    unsigned i;
    char line[3 * 16 + 2] = {0};
    char *p = line;
    for (i = 0; i < len; ++i) {
        *p++ = digits[data[i] >> 4];
        *p++ = digits[data[i] & 0xF];
        if ((i & 15) == 15) {
            *p = '\n';
            PRINT_FUNCTION(line);
            p = line;
        } else {
            *p++ = ' ';
        }
    }
    if (i & 15) {
        *p++ = '\n';
        *p = '\0';
        PRINT_FUNCTION(line);
    }
}
#else
// hexdump version #2
void debugHexdump(void *data_, unsigned len) {
    static const char digits[] = "0123456789abcdef";
    uint8_t *data = (uint8_t *) data_;
    unsigned i;
    char line[5 * 16 + 3] = {0};
    char *p = line;
    for (i = 0; i < len; ++i) {
        *p++ = '0';
        *p++ = 'x';
        *p++ = digits[data[i] >> 4];
        *p++ = digits[data[i] & 0xF];
        if ((i & 15) == 15) {
            *p++ = ',';
            *p++ = '\n';
            *p = '\t';
            PRINT_FUNCTION(line);
            p = line;
        } else {
            *p++ = ',';
        }
    }
    if (i & 15) {
        *p++ = '\n';
        *p = '\0';
        PRINT_FUNCTION(line);
    }
}
#endif

#else // DEBUG not defined
void debugHexdump(void *data, unsigned len)
{
}
#endif // DEBUG
