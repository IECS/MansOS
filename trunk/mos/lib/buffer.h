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

#ifndef MANSOS_BUFFER_H
#define MANSOS_BUFFER_H

#include <string.h> // memcpy
#include <kernel/stdtypes.h>

typedef struct Buffer_s {
    uint8_t *data;
    uint16_t length;
    uint16_t capacity;
} Buffer_t;

void bufferInit(Buffer_t *, void *space, uint16_t capacity);

static inline void bufferReset(Buffer_t *b) {
    b->length = 0;
}

static inline uint16_t bufferSize(Buffer_t *b) {
    return b->length;
}

static inline uint16_t bufferSpace(Buffer_t *b) {
    return b->capacity - b->length;
}

static inline uint8_t bufferWrite(Buffer_t *b, const void *data, uint16_t length) {
    if (b->capacity - b->length < length) return 0xff; // ENOMEM

    memcpy(b->data + b->length, data, length);
    b->length += length;
    return 0;
}

void bufferDump(Buffer_t *);

#endif
