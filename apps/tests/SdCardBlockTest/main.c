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

#include "stdmansos.h"
#include <extflash.h>
#include <utils.h>
//#include <sdcard/sdcard.h>
#include <lib/assert.h>

#define DATA_SIZE 10

uint8_t readBuffer[EXT_FLASH_SECTOR_SIZE];
uint8_t writeBuffer[DATA_SIZE];

void appMain(void)
{
    uint8_t i;
    uint32_t addr;

    PRINTF("appMain\n");

    addr = 0;
    extFlashRead(addr, readBuffer, sizeof(readBuffer));
    debugHexdump(readBuffer, DATA_SIZE);

    for (i = 0; i < DATA_SIZE; ++i) {
        writeBuffer[i] = i;
    }
    extFlashWrite(addr, writeBuffer, sizeof(writeBuffer));

    extFlashRead(addr, readBuffer, sizeof(readBuffer));
    debugHexdump(readBuffer, DATA_SIZE);
    for (i = 0; i < DATA_SIZE; ++i) {
        ASSERT(readBuffer[i] == i);
    }
    PRINTF("all done\n");
    blink(3, 2000);
    ASSERT(false);
}
