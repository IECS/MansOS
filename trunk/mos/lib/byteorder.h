/*
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

#ifndef MOS_BYTEORDER_H
#define MOS_BYTEORDER_H

// -- This file is part of the public MansOS API --

/// \file
/// API for unaligned memory access and little endian / big endian data conversion
///

#include <string.h> // memcpy
#include <stdtypes.h>

#ifdef PLATFORM_PC

#include <byteswap.h>
#include <arpa/inet.h>

#ifndef bswap_32 // in case defined as functions
#define bswap_32 bswap_32
#define bswap_16 bswap_16
#endif

#if BYTE_ORDER == BIG_ENDIAN
#define tobe16(x)       (x) 
#define tole16(x)       bswap_16(x)
#else // __LITTLE_ENDIAN
#define tobe16(x)       bswap_16(x) 
#define tole16(x)       (x) 
#endif // __BYTE_ORDER

#else // PLATFORM_PC not defined

#ifdef MCU_AVR
// AVR is little endian
#define BYTE_ORDER LITTLE_ENDIAN
#else
#ifdef MCU_MSP430
// MSP430 is little endian
#define BYTE_ORDER LITTLE_ENDIAN
#else
#include <endian.h>
#endif
#endif

#if BYTE_ORDER == BIG_ENDIAN
#define ntohl(x)    (x)
#define ntohs(x)    (x)
#define htonl(x)    (x)
#define htons(x)    (x)
#define tobe16(x)   (x) 
#define tole16(x)   bswap_16(x) 
#define tobe32(x)   (x)
#define tole32(x)   bswap_32(x)
#else
#if BYTE_ORDER == LITTLE_ENDIAN
#define ntohl(x)    bswap_32(x)
#define ntohs(x)    bswap_16(x)
#define htonl(x)    ntohl(x)
#define htons(x)    ntohs(x)
#define tobe16(x)   bswap_16(x) 
#define tole16(x)   (x) 
#define tobe32(x)   bswap_32(x)
#define tole32(x)   (x)
#else 
#error Unknown endianness
#endif
#endif

#endif // PLATFORM_PC

#ifndef bswap_32 // check one more time
#define bswap_32(x) (((x) >> 24) | (((x) >> 8) & 0xff00) | (((x) & 0xff00) << 8) | ((x) << 24))
#endif

#ifndef bswap_16
#define bswap_16(x) (((x) >> 8) | (((x) & 0xff) << 8))
#endif

// data reading & writing (for little-endian MCU only!)

#if BYTE_ORDER != LITTLE_ENDIAN
#error The functions are suitable for little-endian architcture only
#endif

//! Read little endian 16 bit value
static inline uint16_t le16Read(const void *ptr) {
    uint16_t result;
    memcpy(&result, ptr, sizeof(result));
    return result;
}

//! Write little endian 16 bit value
static inline void le16Write(void *ptr, uint16_t v) {
    memcpy(ptr, &v, sizeof(v));
}

//! Read little endian 32 bit value
static inline uint32_t le32Read(const void *ptr) {
    uint32_t result;
    memcpy(&result, ptr, sizeof(result));
    return result;
}

//! Write little endian 32 bit value
static inline void le32Write(void *ptr, uint32_t v) {
    memcpy(ptr, &v, sizeof(v));
}

//! Read big endian 16 bit value
static inline uint16_t be16Read(const void *ptr) {
    const uint8_t *p = (const uint8_t *) ptr;
    return ((uint16_t)p[0] << 8) | p[1];
}

//! Write big endian 16 bit value
static inline void be16Write(void *ptr, uint16_t v) {
    uint8_t *p = (uint8_t *) ptr;
    p[0] = v >> 8;
    p[1] = v & 0xff;
}

//! Read big endian 32 bit value
static inline uint32_t be32Read(const void *ptr) {
    const uint8_t *p = (const uint8_t *) ptr;
    return ((uint32_t)p[0] << 24) 
            | ((uint32_t)p[1] << 16) 
            | ((uint16_t)p[2] << 8) 
            | p[3];
}

//! Write big endian 32 bit value
static inline void be32Write(void *ptr, uint32_t v) {
    uint8_t *p = (uint8_t *) ptr;
    p[0] = v >> 24;
    p[1] = (v >> 16) & 0xff;
    p[2] = (v >> 8) & 0xff;
    p[3] = v & 0xff;
}

#endif
