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

//-------------------------------------------
//      Test External Flash read &write
//-------------------------------------------

#include "stdmansos.h"
#include "extflash.h"
#include <string.h>
#include <utils.h>
#include <lib/assert.h>

// Write <BYTE_COUNT> bytes to external flash memory, starting at
// address <START_ADDR>. Write 8-bit counter, starting at value <START_BYTE>
// Note: If BYTE_COUNT is greater than 256 (EXT_FLASH_PAGE_SIZE), only 256 bytes will be written
// Print results to serial port

enum {
//    START_ADDR = 0x12345678,
//    START_ADDR = 0x1234,
//    START_ADDR = EXT_FLASH_SECTOR_SIZE,
    START_ADDR = 0x13,
    START_BYTE = 12,
    BYTE_COUNT = 320
};

static uint8_t buf[BYTE_COUNT];
static uint16_t i;
static uint32_t i32;
static uint8_t c;

void eraseData() {
    // erase 3 sectors to be sure
    PRINTF("erasing flash...");
    extFlashEraseSector(START_ADDR - EXT_FLASH_SECTOR_SIZE);
    PRINTF("+");
    extFlashEraseSector(START_ADDR);
    PRINTF("+");
    extFlashEraseSector(START_ADDR + EXT_FLASH_SECTOR_SIZE);
    PRINTLN("+ DONE");
}

uint8_t data[10];

void eraseAllBySector() {
    PRINTLN("erasing all by sector\n");
    for (i = 0, c = START_BYTE; i < BYTE_COUNT; ++i, ++c) {
        buf[i] = c;
    }
    uint32_t ii;
    for (ii = 13; ii < 1000000; ii += 11177) {
        extFlashEraseSector(ii);
        extFlashWrite(ii, buf, 10);
        memset(data, 0, 10);
        extFlashRead(ii, data, 10);
        ASSERT(!memcmp(buf, data, 10));
    }
    PRINTLN("..done\n");
}

void writeData() {
    for (i = 0, c = START_BYTE; i < BYTE_COUNT; ++i, ++c) {
        buf[i] = c;
    }
    PRINTF("writing flash from %lu to %lu\n",
            (uint32_t)START_ADDR, (uint32_t)START_ADDR + BYTE_COUNT - 2);
    extFlashWrite(START_ADDR, buf, BYTE_COUNT - 2);
    PRINTLN("done");
}

void writeDataSpeedTest() {
    uint32_t startTime = getTimeMs();
    uint32_t i;
    for (i = 0; i < 0x10000; i += 100) {
        extFlashWrite(START_ADDR + i, buf, 100);
    }
    PRINTF("to write 64 kb: %lu ms \n", getTimeMs() - startTime);
}

void printResults() {
    // clear buffer
    memset(buf, 0, BYTE_COUNT);

    PRINTF("reading flash...");
    extFlashRead(START_ADDR, buf, BYTE_COUNT);
    PRINTLN("done");

    for (i = 0, i32 = START_ADDR; i < BYTE_COUNT; ++i, ++i32) {
        PRINTF("[0x%lx] = %u\t", i32, buf[i]);
        if (i % 4 == 3) PRINTLN("");
    }
}

void writeSimpleData() {
    for (i = 0; i < 4; ++i) {
        buf[i] = i + 1;
    }
    PRINTF("writing flash...");
    extFlashWrite(START_ADDR, buf, 4);
    PRINTLN("done");
}

void readData() {
    memset(buf, 0, BYTE_COUNT);

    PRINTF("reading flash...");
    extFlashRead(START_ADDR, buf, 4);
    PRINTLN("done");

    uint32_t addr = START_ADDR;
    for (i = 0; i < 4; ++i) {
        PRINTF("[0x%lx] = %u\t", addr++, buf[i]);
    }
    PRINTLN("");
}

//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    redLedOn();

    extFlashWake();

//    eraseAllBySector();
#if 1
    // normal operation
    eraseData();

    // writeDataSpeedTest();
    // eraseData();
    writeData();

    printResults();
#else
    // minimal test
    eraseData();
    writeSimpleData();
    readData();
#endif

    redLedOff();
    blink(1000, 100);
}
