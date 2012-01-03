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

//
// Testing resource sharing between radio and external flash
//

#include "stdmansos.h"
#include "dprint.h"
#include "hil/extflash.h"
#include "lib/assert.h"
#include "lib/random.h"
#include <string.h>

// while working with flash, radio must be turned off
#define SELECT_FLASH        \
    {                       \
        radioOff();         \
        extFlashWake();

// turn radio back on
#define UNSELECT_FLASH      \
        extFlashSleep();    \
        radioOn();          \
    }


// -------------------------------------
// Timers
// -------------------------------------

Alarm_t timer;

void onTimer(void *x)
{
    TPRINTF("onTimer\n");
    blueLedToggle();

    alarmSchedule(&timer, 1000);
}

// -------------------------------------
// Radio
// -------------------------------------

void radioRecvCb()  {
    static uint8_t buffer[128];
    int16_t len;
    redLedToggle();
    len = radioRecv(buffer, sizeof(buffer) - 1);
    if (len <= 0) {
        PRINTF("radio recv failed, len=%d\n", len);
        return;
    }
    if (len > 0) {
        buffer[len] = 0;
        PRINTF("recv: %s\n", (char *) buffer);
    }
}

// -------------------------------------
// Flash
// -------------------------------------

#define BUFFER_SIZE 64

static uint8_t buffer[BUFFER_SIZE];

void flashErase(uint32_t address)
{
    extFlashEraseSector(address);
}

void flashWrite(uint32_t address)
{
    extFlashWrite(address, buffer, BUFFER_SIZE);
}

void flashRead(uint32_t address)
{
    uint16_t i;
    uint8_t readBuffer[BUFFER_SIZE];
    extFlashRead(address, readBuffer, BUFFER_SIZE);

    for (i = 0; i < BUFFER_SIZE; ++i) {
        ASSERT(readBuffer[i] == buffer[i]);
    }
}

// -------------------------------------
// Main function
// -------------------------------------
void appMain(void)
{
    uint16_t i;

    // timers (used to get an extra interrupt context)
    alarmInit(&timer, onTimer, NULL);
    alarmSchedule(&timer, 1000);

    // radio
    radioSetReceiveHandle(radioRecvCb);
    radioOn();

    for (i = 0; i < BUFFER_SIZE; i++) {
        buffer[i] = i;
    }

    randomInit();

    // SELECT_FLASH;
    // extFlashBulkErase();
    // UNSELECT_FLASH;

    for (i = 0; ; i++) {
        uint32_t address = i * 64ul;

        SELECT_FLASH;

        if (IS_ALIGNED(address, EXT_FLASH_SECTOR_SIZE)) {
            PRINTF("erase address %lu\n", address);
            flashErase(address);
        }

        PRINTF("write address %lu\n", address);
        flashWrite(address);

        if (address > 0) {
            PRINTF("verify...\n");
            flashRead(address - 64);
        }

        UNSELECT_FLASH;

        msleep(400 + randomRand() % 600);

        PRINTF("send smth to radio...\n");
        radioSend("hello world", sizeof("hello world"));

        greenLedToggle();
    }
}
