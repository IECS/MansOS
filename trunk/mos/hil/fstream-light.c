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

#include "fstream-light.h"
#include <dynamic_memory.h>
//#include <sdcard/sdcard.h>
//#include <sdcard_pins.h> // from platform
#include <extflash.h>
#include <platform.h>
#include <lib/assert.h>
#include <lib/codec/crc.h>
#include <print.h>

#if DEBUG
#define SPRINTF PRINTF
#else
#define SPRINTF(...) // nothing
#endif

#define VERIFY 0

#ifndef EXT_FLASH_SPI_ID
#define EXT_FLASH_SPI_ID 0 // XXX
#endif

static volatile uint32_t extFlashAddress;

// TODO: use dynamic alloc
#ifndef FSTREAM_MAX_RECORD_SIZE
#define FSTREAM_MAX_RECORD_SIZE 40
#endif

static uint8_t tmpBuffer[FSTREAM_MAX_RECORD_SIZE];

// XXX: crc, if present, is acutrrently always at buffer[2]
struct HeaderWithCrc_s {
    uint16_t __unused;
    uint16_t crc;
    uint8_t data[0];
} PACKED;
typedef struct HeaderWithCrc_s HeaderWithCrc_t;

void flashStreamReset(void)
{
    extFlashAddress = 0;
}

static void flashStreamFindStart(void *buffer, uint16_t length, bool crc)
{
    bool oneInvalid = false;
    extFlashAddress = EXT_FLASH_RESERVED;

    while (extFlashAddress < EXT_FLASH_SIZE) {
        // PRINTF("%lu\n", extFlashAddress);
        extFlashRead(extFlashAddress, buffer, length);
        bool valid = true;
        if (crc) {
            HeaderWithCrc_t headerWithCrc;
            memcpy(&headerWithCrc, buffer, sizeof(headerWithCrc));
            uint16_t calcCrc = crc16((uint8_t *)buffer + sizeof(headerWithCrc), length - sizeof(headerWithCrc));
            valid = headerWithCrc.crc == calcCrc;
        } 
        if (valid) {
            // check if all fields are 0 - also count as invalid
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
                extFlashAddress -= length;
                break;
            }
        }
        extFlashAddress += length;
    }
}

bool flashStreamWriteRecord(void *data, uint16_t length, bool crc)
{
    ASSERT(length <= sizeof(tmpBuffer));

    // XXX hack
    if (serial[EXT_FLASH_SPI_ID].busy) return false;

    if (extFlashAddress == 0) {
        //void *buffer = memoryAlloc(length);
        flashStreamFindStart(tmpBuffer, length, crc);
        //memoryFree(buffer);
    }

    // PRINTF("write record %u bytes at %lu\n", length, extFlashAddress);

    if (crc) {
        uint16_t calcCrc = crc16((uint8_t *)data + sizeof(HeaderWithCrc_t), length - sizeof(HeaderWithCrc_t));
        memcpy((uint8_t *)data + 2, &calcCrc, sizeof(calcCrc));
    }
    // PRINTF("sdcard write at %lu\n", extFlashAddress);
    extFlashWrite(extFlashAddress, data, length);
    // make sure it's saved to the card, not just in buffers
    //extFlashFlush();
#if VERIFY
    //void *copy = memoryAlloc(length);
    extFlashRead(extFlashAddress, tmpBuffer, length);
    if (memcmp(data, tmpBuffer, length)) {
        ASSERT("writing in SD card failed!" && false);
    }
    //memoryFree(copy);
#endif
    extFlashAddress += length;
    return true;
}

bool flashStreamReadRecord(void *data, uint16_t length, bool crc)
{
    if (extFlashAddress == 0) flashStreamFindStart(data, length, crc);
    extFlashRead(extFlashAddress, data, length);
    if (crc) {
        HeaderWithCrc_t headerWithCrc;
        memcpy(&headerWithCrc, data, sizeof(headerWithCrc));
        uint16_t calcCrc = crc16((uint8_t *)data + sizeof(headerWithCrc), length - sizeof(headerWithCrc));
        return headerWithCrc.crc == calcCrc;
    }
    extFlashAddress += length;
    return true;
}
