/**
 * Copyright (c) 2008-2011 the MansOS team. All rights reserved.
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
 *
 * fs/block/flash_access.h -- flash memory interfaces
 */

#ifndef _FS_BLOCK_FLASH_ACCESS_H
#define _FS_BLOCK_FLASH_ACCESS_H

#include <stddef.h>
#include <stdint.h>

#include <alarms.h>
#include <extflash.h>
#include <fs/types.h>

#include "common.h"

/* Convert a block number and an offset to a flash address */
static inline uint32_t flashAddr(blk_t block, fsOff_t offset)
{
    return (uint32_t)block * BLOCK_SIZE + offset;
}

/* Calculate the physical address of logical @offset */
static inline uint32_t chunkAddr(blk_t block, fsOff_t offset)
{
    return flashAddr(block, (offset % BLOCK_DATA_SIZE) / CHUNK_DATA_SIZE
                            * CHUNK_SIZE + offset % CHUNK_DATA_SIZE);
}

/* Address of service information for chunk containing @offset */
static inline uint32_t serviceAddr(blk_t block, fsOff_t offset)
{
    return flashAddr(block, (offset % BLOCK_DATA_SIZE) / CHUNK_DATA_SIZE
                            * CHUNK_SIZE + CHUNK_DATA_SIZE);
}

/* How long to wait before entering low-power mode (ms) */
#define FS_FLASH_IDLE_TIME  100

extern Alarm_t fsBlockFlashAlarm;

#if 0 // TODO: broken for now with the new alarms

/* Begin working with flash */
static inline void enableFlash(void)
{
    if (!alarmRemove(&fsBlockFlashAlarm))
    {
        /* If the flash has already been put to sleep */
        extFlashWake();
    }
}

/* End working with flash */
static inline void disableFlash(void)
{
    alarmRegister(&fsBlockFlashAlarm);
}

#else /* Alternative variants */

static inline void enableFlash(void)
{
    extFlashWake();
}

static inline void disableFlash(void)
{
    extFlashSleep();
}

#endif

/* Flash memory access wrappers */

static inline void blkExtFlashRead(uint32_t addr, void *buf, size_t len)
{
    enableFlash();
    extFlashRead(addr, buf, len);
    disableFlash();
}

static inline void blkExtFlashWrite(uint32_t addr, const void *buf, size_t len)
{
    enableFlash();
    extFlashWrite(addr, buf, len);
    disableFlash();
}

static inline void blkExtFlashEraseSector(uint32_t addr)
{
    enableFlash();
    extFlashEraseSector(addr);
    disableFlash();
}

#endif /* _FS_BLOCK_FLASH_ACCESS_H */
