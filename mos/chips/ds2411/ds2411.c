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

#include "ds2411.h"
#include <string.h>
#include "platform.h"

#if SNUM_CHIP == SNUM_DS2411

#define ONEWIRE_PORT DS2411_PORT
#define ONEWIRE_PIN  DS2411_PIN
#include <hil/onewire.h>

static uint8_t ds2411[SERIAL_NUMBER_SIZE];
static bool ds2411Ok;

// application is responsible for giving enough bytes
bool ds2411Get(uint8_t *result) {
    memcpy(result, ds2411, SERIAL_NUMBER_SIZE);
    return ds2411Ok;
}

bool ds2411SnumMatches(const uint8_t *snum) {
    return ds2411Ok && memcmp(ds2411, snum, SERIAL_NUMBER_SIZE) == 0;
}

int ds2411Init() {
    ds2411Ok = owReadROM(ds2411) && ds2411[7] == 0x01; // 0x01 is the family code

#ifdef PLATFORM_TELOSB
    // 00:12:75    Moteiv    # Moteiv Corporation
    ds2411[0] = 0x00;
    ds2411[1] = 0x12;
    ds2411[2] = 0x75;
#endif

    return ds2411Ok;
}

#endif // SNUM_CHIP == SNUM_DS2411
