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

#ifndef MANSOS_UNALIGNED
#define MANSOS_UNALIGNED

#include <kernel/stdtypes.h>
#include <string.h> // fur PC

// Grr... this all was nice and clean - with structs, like in RouterOS -
// but stupid msp430 compiler screws it up... we have to use memcpy()

static inline uint16_t getU16(const void *ptr) {
    uint16_t result;
    memcpy(&result, ptr, sizeof(result));
    return result;
}

static inline void putU16(void *ptr, uint16_t v) {
    memcpy(ptr, &v, sizeof(v));
}

static inline unsigned getU32(const void *ptr) {
    uint32_t result;
    memcpy(&result, ptr, sizeof(result));
    return result;
}

static inline void putU32(void *ptr, uint32_t v) {
    memcpy(ptr, &v, sizeof(v));
}


static inline uint16_t getU16Network(const void *ptr) {
    uint16_t result;
    result = ((uint8_t *) ptr)[0];
    result <<= 8;
    result |= ((uint8_t *) ptr)[1];
    return result;
}

static inline void putU16Network(void *ptr, uint16_t v) {
    ((uint8_t *) ptr)[0] = v >> 8;
    ((uint8_t *) ptr)[1] = v & 0xff;
}

#endif
