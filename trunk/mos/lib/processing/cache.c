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

#include "cache.h"
#include <hil/timers.h>

typedef struct SensorCache_s {
    union {
        uint8_t u8;
        uint16_t u16;
        uint32_t u32;
    } value;
    ticks_t expireTime; // in jiffies
//    uint16_t code;
} SensorCache_t;

#ifndef TOTAL_CACHEABLE_SENSORS
#error define TOTAL_CACHEABLE_SENSORS before using cache.c!
#endif
static SensorCache_t sensorCache[TOTAL_CACHEABLE_SENSORS];

uint8_t cacheReadSensorU8(uint16_t code, ReadFunctionU8 func, uint16_t expireTime)
{
    ticks_t now = getJiffies();
    SensorCache_t *cacheValue = &sensorCache[code];
    if (cacheValue->expireTime && !timeAfter(now, cacheValue->expireTime)) {
        // take from cache
        return cacheValue->value.u8;
    }
    uint8_t result = func();
    if (expireTime) {
        // add to cache
        cacheValue->value.u8 = result;
        cacheValue->expireTime = now + expireTime;
    }
    return result;
}

uint16_t cacheReadSensorU16(uint16_t code, ReadFunctionU16 func, uint16_t expireTime)
{
    ticks_t now = getJiffies();
    SensorCache_t *cacheValue = &sensorCache[code];
    if (cacheValue->expireTime && !timeAfter(now, cacheValue->expireTime)) {
        // take from cache
        return cacheValue->value.u16;
    }
    uint16_t result = func();
    if (expireTime) {
        // add to cache
        cacheValue->value.u16 = result;
        cacheValue->expireTime = now + expireTime;
    }
    return result;
}

uint32_t cacheReadSensorU32(uint16_t code, ReadFunctionU32 func, uint16_t expireTime)
{
    ticks_t now = getJiffies();
    SensorCache_t *cacheValue = &sensorCache[code];
    if (cacheValue->expireTime && !timeAfter(now, cacheValue->expireTime)) {
        // take from cache
        return cacheValue->value.u32;
    }
    uint32_t result = func();
    if (expireTime) {
        // add to cache
        cacheValue->value.u32 = result;
        cacheValue->expireTime = now + expireTime;
    }
    return result;
}
