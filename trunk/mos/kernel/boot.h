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

#ifndef MANSOS_BOOT_H
#define MANSOS_BOOT_H

#include "defines.h"
#include <hil/intflash.h>

// This structure is stored in info flash memory
struct BootParams_s {
    uint32_t extFlashAddress;    // image location on external flash
    uint16_t intFlashAddress;    // internal flash address where the current image is loaded
    uint8_t bootRetryCount;      // cleared to zero after successful booting
    uint8_t doReprogramming;     // if not zero, then replace the existing with the one in 'extFlashAddress'
} PACKED;

typedef struct BootParams_s BootParams_t;

#define MAX_RETRY_COUNT       3 // after this much unsuccessful booting attemps, "golden image" is loaded

#define GOLDEN_IMAGE_ADDRESS  0x10000ul // address in external flash

#define BOOT_PARAMS_ADDRESS   0x1000 // internal flash address = MSP430_FLASH_INFOMEM_START

// TODO: platform specific!
#define BOOTLOADER_START      INT_FLASH_START
#define MAX_BOOTLOADER_SIZE   0x1000          // 4096 bytes
#define BOOTLOADER_END        (BOOTLOADER_START + MAX_BOOTLOADER_SIZE)

#define MAX_SYSTEM_CODE_SIZE  0xA000          // 40k bytes
#define MAX_USER_CODE_SIZE \
    (INT_FLASH_SIZE - MAX_SYSTEM_CODE_SIZE - MAX_BOOTLOADER_SIZE - 64) // almost 4k bytes

// The constants used by reprogramming server
#define DEFAULT_INTERNAL_FLASH_ADDRESS  BOOTLOADER_END  // 0x5000
#define DEFAULT_EXTERNAL_FLASH_ADDRESS  GOLDEN_IMAGE_ADDRESS
// actually 48kb, but align to ext. flash sector size
#define MAX_IMAGE_SIZE                  (64 * 1024u)

#define SYSTEM_CODE_START     DEFAULT_INTERNAL_FLASH_ADDRESS             // 0x5000
#define USER_CODE_START       (SYSTEM_CODE_START + MAX_SYSTEM_CODE_SIZE) // 0xf000

void bootParamsInit(void);

#endif
