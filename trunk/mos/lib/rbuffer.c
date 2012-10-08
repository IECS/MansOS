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

#include "rbuffer.h"
#include "dprint.h"
#include "assert.h"
#include <errors.h>

void rbufferInit(RingBuffer_t *b, void *space, uint16_t len) {
    ASSERT(len);
    b->data = (uint8_t *) space;
    b->length = len;
    b->readPos = b->writePos = 0;
}

void rbufferReset(RingBuffer_t *b) {
    b->readPos = b->writePos = 0;
}

uint16_t rbufferSize(RingBuffer_t *b) {
    if (b->writePos >= b->readPos) return b->writePos - b->readPos;

    return (b->length - b->readPos) + b->writePos;
}

uint16_t rbufferSpace(RingBuffer_t *b) {
    // XXX: can be optimized
    return b->length - rbufferSize(b);
}

uint8_t rbufferWrite(RingBuffer_t *b, const void *data, uint16_t length) {
    if (rbufferSpace(b) < length) return 0xff; // ENOMEM;

    const uint16_t tailLen = b->length - b->writePos;
    if (tailLen > length) {
        memcpy(b->data + b->writePos, data, length);
        b->writePos += length;
        return 0;
    }

    memcpy(b->data + b->writePos, data, tailLen);
    length -= tailLen;
    memcpy(b->data, data + tailLen, length);
    b->writePos = length;

    return 0;
}

uint8_t rbufferRead(RingBuffer_t *b, uint16_t length) {
    if (rbufferSize(b) < length) return 0xff; // EINVAL;

    b->readPos += length;
    b->readPos %= b->length;

    // XXX: what else should this do?

    return 0;
}

void rbufferDump(RingBuffer_t *b) {
    PRINTF("buffer size %d\n", rbufferSize(b));
    if (b->writePos == b->readPos) return; // empty
    if (b->writePos > b->readPos) {
        debugHexdump(b->data + b->readPos, b->writePos - b->readPos);
        return;
    }
    debugHexdump(b->data + b->readPos, b->length - b->readPos);
    // XXX: output should be glued together
    debugHexdump(b->data, b->writePos);
}


