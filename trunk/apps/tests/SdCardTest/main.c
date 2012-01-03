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

#include "mansos.h"
#include "udelay.h"
#include "dprint.h"
#include "radio.h"
#include "sleep.h"
#include "leds.h"

#include <fs/sdcard/fatlib.h>
#include <string.h>

//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    bool status;

    mdelay(1000);

    // PRINT("FAT init...\n");
    status = fat_initialize();   // Initialise the FAT library.

//    initLeds();

    PRINTF("status=%d\n", (int) status);
    PRINTF("problem=%d\n", fatGetLastError());
    PRINTF("file sys=%d\n", (int)fatGetFileSys());

    if (status) {
        int8_t handle;
        int16_t stringSize;
        char stringBuf[100];

        PRINTF("read...\n");
        handle = fat_openRead("read.txt");
        if (handle >= 0) {
            PRINTF("opened for reading OK\n");
            // fat_read(handle, stringBuf, 100); // 0-99
            // fat_read(handle, stringBuf, 100); // 100-199
            // fat_read(handle, stringBuf, 100); // 200-299
            stringSize = fat_read(handle, (uint8_t *) stringBuf, 4);
            stringBuf[stringSize] = '\n';
            stringBuf[stringSize + 1] = '\0';
            PRINTF("stringSize=%d\n", stringSize);
            PRINTF(stringBuf);
            PRINTF("done\n");
            fat_close(handle);
        } else {
            PRINTF("open read: %d\n", handle);
        }

#if 1
        PRINTF("write...\n");
        handle = fat_openWrite("write.txt");
        if (handle >= 0) {
            PRINTF("opened for writing OK\n");
            strcpy(stringBuf, "crap");
            stringSize = strlen(stringBuf);
            fat_write(handle, (uint8_t *)stringBuf, stringSize);
            fat_flush();    // Optional.
            fat_close(handle);
        } else {
            PRINTF("open write: %d\n", handle);
        }
#endif
    }

    mdelay(1000);
    while(1) {
        msleep(3000);
        PRINT(".\n");
    }
}
