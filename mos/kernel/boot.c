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

#include "mansos.h"
#include "boot.h"

// if reprogramming code is included, make sure bootloader is present
#if USE_REPROGRAMMING
#define INCLUDE_BOOTLOADER 1
#else
#define INCLUDE_BOOTLOADER 0
#endif

#ifdef INCLUDE_BOOTLOADER

uint8_t bootloaderCode[] = {
#include "bl.hex"
};

static inline bool bootloaderIsPresent(void)
{
    uint8_t firstBytes[2];
    intFlashRead(BOOTLOADER_START, firstBytes, sizeof(firstBytes));

    // PRINTF("firstBytes=0x%x 0x%x (@0x%x)\n", firstBytes[0], firstBytes[1], BOOTLOADER_START);

    // TODO FIXME: platform specific!
    // Code for call instruction on msp430 is: 0xb0 0x12
    if (firstBytes[0] == 0xb0 && firstBytes[1] == 0x12) {
        // this means only "start" code from reprogramming.c is present, not a bootloader
        return false;
    }
    return true;
}

bool fixBootloader(void)
{
    if (bootloaderIsPresent()) return false;

    // PRINTF("rewrite bootloader, addr=0x%x\n", BOOTLOADER_START);
    intFlashErase(BOOTLOADER_START, sizeof(bootloaderCode));
    intFlashWrite(BOOTLOADER_START, bootloaderCode, sizeof(bootloaderCode));

    return true;
}

#else // bootloader not included

#define fixBootloader() false

#endif

//
// The purpose of this function is to reset the boot retry counter
//
void bootParamsInit(void)
{
    BootParams_t bootParams;
    intFlashRead(BOOT_PARAMS_ADDRESS, &bootParams, sizeof(bootParams));

    if (fixBootloader()) {
        // write default boot parameters
        bootParams.extFlashAddress = GOLDEN_IMAGE_ADDRESS;
        bootParams.intFlashAddress = SYSTEM_CODE_START;
    }
    // reset retry and reprogramming counter
    bootParams.bootRetryCount = 0;
    bootParams.doReprogramming = 0;

    // PRINTF("rewrite boot params\n");
    intFlashErase(BOOT_PARAMS_ADDRESS, sizeof(bootParams));
    intFlashWrite(BOOT_PARAMS_ADDRESS, &bootParams, sizeof(bootParams));
    (void) bootParams; // make compiler happy
}
