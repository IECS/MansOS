/*
 * Copyright (c) 2008-2013 the MansOS team. All rights reserved.
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

#ifndef MANSOS_EXTFLASH_H
#define MANSOS_EXTFLASH_H

/// \file
/// External flash chip driver
///

///
/// List all supported external flash models here (before including extflash_hal.h)
///
#define FLASH_CHIP_M25P80  1
#define FLASH_CHIP_AT25DF  2
#define FLASH_CHIP_SDCARD  3 // not really a flash chip, but...


//! Enter low power mode
void extFlashSleep();
//! Wake up from low power mode
void extFlashWake();
//! Read len bytes from flash starting at address addr into buf
void extFlashRead(uint32_t addr, uint8_t *buf, uint16_t len);
//! Write len bytes from buf to flash starting at address addr
void extFlashWrite(uint32_t addr, uint8_t *buf, uint16_t len);
//! Erase the whole flash memory
void extFlashBulkErase();
///
/// Erase one sector at address
///
/// Address is not the sector number, rather an address inside the sector
///
void extFlashEraseSector(uint32_t address);

// init flash (including SPI bus). for kernel only
extern inline void extFlashInit(void); 

#include "extflash_hal.h"


#ifndef EXT_FLASH_SECTOR_SIZE
#warning External flash constants not defined for this platform!
#define EXT_FLASH_SECTOR_SIZE   0
#define EXT_FLASH_SECTOR_COUNT  0
#define EXT_FLASH_PAGE_SIZE     0
#endif

//! The size of the extrernal flash memory in bytes
#define EXT_FLASH_SIZE \
    ((unsigned long)EXT_FLASH_SECTOR_SIZE * EXT_FLASH_SECTOR_COUNT)


#endif
