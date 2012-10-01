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

#ifndef MANSOS_CACHE_H
#define MANSOS_CACHE_H

#include <kernel/defines.h>

//
// Sensor cache module.
// Config file should define CONST_TOTAL_CACHEABLE_SENSORS before useing this.
//

typedef int8_t (*ReadFunction8)(bool *isFilteredOut);
typedef int16_t (*ReadFunction16)(bool *isFilteredOut);
typedef int32_t (*ReadFunction32)(bool *isFilteredOut);

int8_t cacheReadSensor8(uint16_t code, ReadFunction8 func,
                        uint16_t expireTime, bool *isFilteredOut);

int16_t cacheReadSensor16(uint16_t code, ReadFunction16 func,
                          uint16_t expireTime, bool *isFilteredOut);

int32_t cacheReadSensor32(uint16_t code, ReadFunction32 func,
                          uint16_t expireTime, bool *isFilteredOut);

#endif
