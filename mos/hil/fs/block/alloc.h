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
 *
 * fs/block/alloc.h -- block allocation and deallocation
 */

#ifndef _FS_BLOCK_ALLOC_H_
#define _FS_BLOCK_ALLOC_H_

#include <limits.h>

#include <hil/extflash.h>
#include <kernel/mos_sem.h>

#include "common.h"
#include "meta.h"

/* Offset to the block table in EEPROM */
#define BLKTABLE_OFFSET FS_HEADER_SIZE

/* Size of the block table. Each block occupies two bits. */
#define BLKTABLE_SIZE (EXT_FLASH_SECTOR_COUNT * BLOCKS_PER_SEGMENT * 2 / CHAR_BIT)

/* Allocate a block. @old should be the previous block in file. */
blk_t fsBlockAllocate(blk_t old);

/*
 * Get next block (does not detect errors). Caller should hold a mutex of the
 * open file to which the block belongs.
 */
blk_t fsBlockGetNext(blk_t prev);

/* Free block */
void fsBlockFree(blk_t block);

/* Free all blocks */
void fsBlockFreeAll(void);

extern mos_mutex_t fsBlkTableMutex;

#endif /* _FS_BLOCK_ALLOC_H_ */
