/**
 * Copyright (c) 2008-2010 Leo Selavo and the contributors. All rights reserved.
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

#include "stdmansos.h"
#include "dprint.h"
#include "intflash.h"

#define START_ADDRESS  0x6000

uint8_t buffer[INT_FLASH_SEGMENT_SIZE];

#define NUM_SEGMENTS 1

void test(uint8_t value)
{
    FlashAddress_t address;
    int i;

    PRINT_INIT(INT_FLASH_SEGMENT_SIZE);
    redLedOn();

    for (i = 0; i < INT_FLASH_SEGMENT_SIZE; ++i) {
        buffer[i] = (uint8_t) (i + value);
    }

    // flash must be erased before writing in it
    intFlashErase(START_ADDRESS, sizeof(buffer) * NUM_SEGMENTS);

    address = START_ADDRESS;
    for (i = 0; i < NUM_SEGMENTS; ++i) {
#if 1
        intFlashWrite(address, buffer, sizeof(buffer));
#else
        intFlashWriteBlock(address, buffer, sizeof(buffer));
#endif
        address += sizeof(buffer);
    }

    redLedOff();
    blueLedOn();

    address = START_ADDRESS;
    for (i = 0; i < NUM_SEGMENTS; ++i) {
        intFlashRead(address, buffer, sizeof(buffer));

        debugHexdump(buffer, sizeof(buffer));
        PRINTLN();
        PRINTLN();

        address += sizeof(buffer);
    }

    blueLedOff();
    greenLedOn();
}

void appMain(void)
{
    test(0);
    test(128);
}
