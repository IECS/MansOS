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

#ifndef EXT_FLASH_HAL_H
#define EXT_FLASH_HAL_H

// let the platform define the chip it uses
#include <platform.h>

// #ifndef EXT_FLASH_CHIP
// #define EXT_FLASH_CHIP FLASH_CHIP_M25P80
// #endif

#if EXT_FLASH_CHIP == FLASH_CHIP_M25P80

#include <m25p80/m25p80.h>

#define EXT_FLASH_SECTOR_SIZE   M25P80_SECTOR_SIZE
#define EXT_FLASH_SECTOR_COUNT  M25P80_SECTOR_COUNT
#define EXT_FLASH_PAGE_SIZE     M25P80_PAGE_SIZE

#define extFlashInit() m25p80_init()
#define extFlashSleep() m25p80_sleep()
#define extFlashWake() m25p80_wake()
#define extFlashRead(addr, buf, len) m25p80_read(addr, buf, len)
#define extFlashWrite(addr, buf, len) m25p80_write(addr, buf, len)
#define extFlashBulkErase() m25p80_bulkErase()
#define extFlashEraseSector(addr) m25p80_eraseSector(addr)

#elif EXT_FLASH_CHIP == FLASH_CHIP_AT25DF

#include <at25df/at25df.h>

#define EXT_FLASH_SECTOR_SIZE   AT25DF_SECTOR_SIZE
#define EXT_FLASH_SECTOR_COUNT  AT25DF_SECTOR_COUNT
#define EXT_FLASH_PAGE_SIZE     AT25DF_PAGE_SIZE

#define extFlashInit() at25df_init()
#define extFlashSleep() at25df_sleep()
#define extFlashWake() at25df_wake()
#define extFlashRead(addr, buf, len) at25df_read(addr, buf, len)
#define extFlashWrite(addr, buf, len) at25df_write(addr, buf, len)
#define extFlashBulkErase() at25df_bulkErase()
#define extFlashEraseSector(addr) at25df_eraseSector(addr)

#elif EXT_FLASH_CHIP == FLASH_CHIP_SDCARD

#include <sdcard/sdcard.h>

#define EXT_FLASH_SECTOR_SIZE   SDCARD_SECTOR_SIZE
#define EXT_FLASH_SECTOR_COUNT  SDCARD_SECTOR_COUNT
#define EXT_FLASH_PAGE_SIZE     SDCARD_PAGE_SIZE

#define extFlashInit() sdcardInit()
#define extFlashSleep() sdcardSleep()
#define extFlashWake() sdcardWake()
#define extFlashRead(addr, buf, len) sdcardRead(addr, buf, len)
#define extFlashWrite(addr, buf, len) sdcardWrite(addr, buf, len)
#define extFlashBulkErase() sdcardBulkErase()
#define extFlashEraseSector(addr) sdcardEraseSector(addr)

#else
#warning External flash chip not defined for this platform!
#define extFlashInit()
#define extFlashSleep()
#define extFlashWake()
#define extFlashRead(addr, buf, len) (0)
#define extFlashWrite(addr, buf, len)
#define extFlashBulkErase()
#define extFlashEraseSector(addr)
#endif


#endif
