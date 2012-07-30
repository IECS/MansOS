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

//
// TODO: rewrite this to allow record with different sizes
//

#include "sdstream.h"
#include <sdcard/sdcard.h>
#include <lib/assert.h>
#include <lib/codec/crc.h>

#if DEBUG
#define SPRINTF PRINTF
#else
#define SPRINTF(...) // nothing
#endif

#define VERIFY 1

static uint32_t sdCardAddress;

// XXX: crc, if present, is acutrrently always at buffer[2]
struct HeaderWithCrc_s {
    uint16_t __unused;
    uint16_t crc;
    uint8_t data[0];
} PACKED;
typedef struct HeaderWithCrc_s HeaderWithCrc_t;

void sdStreamReset(void)
{
    sdCardAddress = 0;
}

static void sdStreamFindStart(void *buffer, uint16_t length, bool crc)
{
    bool oneInvalid = false;
    sdCardAddress = SDCARD_RESERVED;
    while (sdCardAddress < SDCARD_SIZE) {
        sdcardRead(sdCardAddress, buffer, length);
        bool valid;
        if (crc) {
            HeaderWithCrc_t headerWithCrc;
            memcpy(&headerWithCrc, buffer, sizeof(headerWithCrc));
            uint16_t calcCrc = crc16((uint8_t *)buffer + sizeof(headerWithCrc), length - sizeof(headerWithCrc));
            valid = headerWithCrc.crc == calcCrc;
        } else {
            uint16_t i;
            valid = false;
            for (i = 0; i < length; ++i) {
                if (((uint8_t *)buffer)[i] != 0) {
                    valid = true;
                    break;
                }
            }
        }
        if (valid) {
            oneInvalid = false;
        } else {
            // XXX: this is supposed to find first non-packet, but it will find first two invalid packets!!
            if (!oneInvalid) {
                oneInvalid = true;
            } else {
                sdCardAddress -= length;
                break;
            }
        }
        sdCardAddress += length;
    }
}


bool sdStreamWriteRecord(void *data, uint16_t length, bool crc)
{
    if (sdCardAddress == 0) {
        void *buffer = malloc(length);
        sdStreamFindStart(buffer, length, crc);
        free(buffer);
    }
    if (crc) {
        uint16_t calcCrc = crc16((uint8_t *)data + sizeof(HeaderWithCrc_t), length - sizeof(HeaderWithCrc_t));
        memcpy((uint8_t *)data + 2, &calcCrc, sizeof(calcCrc));
    }
    sdcardWrite(sdCardAddress, data, length);
#if VERIFY
    void *copy = malloc(length);
    sdcardRead(sdCardAddress, copy, length);
    if (memcmp(data, copy, length)) {
        ASSERT("writing in SD card failed!" && false);
    }
    free(copy);
#endif
    sdCardAddress += length;
    return true;
}

bool sdStreamReadRecord(void *data, uint16_t length, bool crc)
{
    if (sdCardAddress == 0) sdStreamFindStart(data, length, crc);
    sdcardRead(sdCardAddress, data, length);
    if (crc) {
        HeaderWithCrc_t headerWithCrc;
        memcpy(&headerWithCrc, data, sizeof(headerWithCrc));
        uint16_t calcCrc = crc16((uint8_t *)data + sizeof(headerWithCrc), length - sizeof(headerWithCrc));
        return headerWithCrc.crc == calcCrc;
    }
    sdCardAddress += length;
    return true;
}
