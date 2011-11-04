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
 * fs/block/alloc.c -- block allocation and deallocation
 */

#include <limits.h>
#include <stdint.h>

#include <hil/eeprom.h>
#include <hil/extflash.h>
#include <hil/fs/common.h>
#include <hil/fs/types.h>
#include <kernel/defines.h>
#include <kernel/mos_sem.h>
#include <lib/random.h>

#include "alloc.h"
#include "common.h"
#include "flash_access.h"

enum blockState {
    BLOCK_FREE  = 0x1, /* Block is ready for use */
    BLOCK_USED  = 0x2, /* Block is in use */
    BLOCK_AVAIL = 0x0  /* Block is free, but needs erasure */
};

/*
 * Two bits correspond to one block in the block table. This type holds as
 * many bits as the count corresponding to one segment.
 */
typedef uint8_t segment_t;
#define BLOCK_ALL_FREE  0x55
#define BLOCK_ALL_USED  0xAA
#define BLOCK_ALL_AVAIL 0x0

COMPILE_TIME_ASSERT(CHAR_BIT * sizeof(segment_t) == BLOCKS_PER_SEGMENT * 2,
                    sizecheck);

/*
 * Used to represent segment numbers. Defined as blk_t because it can't be
 * wider than this type.
 */
typedef blk_t segnum_t;
#define SEGNUM_INVAL BLOCK_INVAL

mos_mutex_t fsBlkTableMutex;

#define GETBLOCK(seg, i)     ((seg) >> 2 * (i) & 0x3)
#define SETBLOCK(seg, i, st) \
    ((seg) = ((seg) & ~((segment_t)0x3 << 2 * (i))) | (segment_t)(st) << 2 * (i))

static inline segment_t readSeg(segnum_t i)
{
    segment_t s;
    eepromRead(BLKTABLE_OFFSET + i * sizeof(segment_t), &s, sizeof(s));
    return s;
}

static inline void writeSeg(segnum_t i, segment_t s)
{
    eepromWrite(BLKTABLE_OFFSET + i * sizeof(segment_t), &s, sizeof(s));
}

static inline segnum_t segNum(blk_t block)
{
    return block / BLOCKS_PER_SEGMENT;
}

static inline blk_t segOffset(blk_t block)
{
    return block % BLOCKS_PER_SEGMENT;
}

static inline blk_t makeBlock(segnum_t seg, blk_t off)
{
    return seg * BLOCKS_PER_SEGMENT + off;
}

/* Find a free block in segment */
static blk_t findFreeBlock(segnum_t seg)
{
    /* Computing the remainder isn't the best option, but should suffice */
    blk_t     start = randomRand() % BLOCKS_PER_SEGMENT,
              i = start;
    segment_t s = readSeg(seg);

    do
    {
        if (GETBLOCK(s, i) == BLOCK_FREE)
            return makeBlock(seg, i);
        i = (i + 1) % BLOCKS_PER_SEGMENT;
    }
    while (i != start);

    return BLOCK_INVAL;
}

/*
 * This function tries to allocate blocks in such manner that blocks belonging
 * to the same file are within common segments (when such file is deleted, these
 * segments become erasable) and allocations are sufficiently random to employ
 * flash memory wear-leveling. In order to do so, blocks are searched in the
 * following sequence:
 *   - Blocks within the same segment as the previous block of this file, if
 *     applicable;
 *   - Blocks within an free segment;
 *   - Blocks within a segment that can readily be erased;
 *   - Other blocks.
 */
blk_t fsBlockAllocate(blk_t old)
{
    blk_t res = BLOCK_INVAL;

    mos_mutex_lock(&fsBlkTableMutex);

    if (old != BLOCK_INVAL)
    {
        /* If @old is valid, try to allocate a block in the same segment */
        res = findFreeBlock(segNum(old));
    }
    if (res == BLOCK_INVAL)
    {
        /*
         * Search for a free segment. During the search we also note segments
         * that are either erasable or are partially free. In case the search
         * fails, we will have gone through all segments and thus found the
         * ones of the latter kinds, if any.
         */

        segnum_t start = randomRand() % EXT_FLASH_SECTOR_COUNT,
                 i = start,
                 avail = SEGNUM_INVAL, partial = SEGNUM_INVAL;
        do
        {
            segment_t s = readSeg(i);
            if (s == BLOCK_ALL_FREE)
            {
                res = makeBlock(i, randomRand() % BLOCKS_PER_SEGMENT);
                break;
            }
            else if (!(s & BLOCK_ALL_USED))
            {
                /*
                 * This segment contains a mixture of free and available blocks
                 */
                avail = i;
            }
            else if (s & BLOCK_ALL_FREE)
            {
                /* This segment has at least one free block */
                partial = i;
            }
            i = (i + 1) % EXT_FLASH_SECTOR_COUNT;
        }
        while (i != start);

        if (res == BLOCK_INVAL) /* No free segments */
        {
            if (avail != SEGNUM_INVAL)
            {
                /* If there's an available segment, erase it */
                writeSeg(avail, BLOCK_ALL_FREE);
                blkExtFlashEraseSector(flashAddr(makeBlock(avail, 0), 0));
                res = makeBlock(avail, randomRand() % BLOCKS_PER_SEGMENT);
            }
            else if (partial != SEGNUM_INVAL)
            {
                /*
                 * Or else we arrive to the least wanted option -- share a
                 * segment with another file
                 */
                res = findFreeBlock(partial);
            }
        }
    }
    if (res == BLOCK_INVAL)
        fsSetError(FS_ERR_NOSPC);
    else
    {
        /* Mark the block as used */
        segment_t s = readSeg(segNum(res));
        SETBLOCK(s, segOffset(res), BLOCK_USED);
        writeSeg(segNum(res), s);

        /* Also set this block as next in chain */
        if (old != BLOCK_INVAL)
        {
            blkExtFlashWrite(flashAddr(old, BLOCK_SIZE - sizeof(blk_t)),
                             &res, sizeof(blk_t));
        }
    }

    mos_mutex_unlock(&fsBlkTableMutex);

    return res;
}

blk_t fsBlockGetNext(blk_t prev)
{
    blk_t res;

    /*
     * The next block number is positioned at the very end of the current block
     */
    blkExtFlashRead(flashAddr(prev, BLOCK_SIZE - sizeof(blk_t)),
                    &res, sizeof(blk_t));
    return res;
}

void fsBlockFree(blk_t block)
{
    segment_t s;

    mos_mutex_lock(&fsBlkTableMutex);

    s = readSeg(segNum(block));
    SETBLOCK(s, segOffset(block), BLOCK_AVAIL);
    writeSeg(segNum(block), s);

    mos_mutex_unlock(&fsBlkTableMutex);
}

/* This function should only be called from init */
void fsBlockFreeAll(void)
{
    segnum_t i;

    for (i = 0; i < EXT_FLASH_SECTOR_COUNT; i++)
        writeSeg(i, BLOCK_ALL_AVAIL);
}
