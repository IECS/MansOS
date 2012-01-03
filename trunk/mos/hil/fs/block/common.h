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
 * fs/block/common.h -- common definitions
 */

#ifndef _FS_BLOCK_COMMON_H_
#define _FS_BLOCK_COMMON_H_

#include <stdint.h>

#include <hil/extflash.h>
#include <hil/fs/types.h>
#include <kernel/defines.h>

/*
 * A block is the minimal addressable unit of data. A segment is the minimal
 * erasable unit of data. Block size has to be chosen reasonably in regard of
 * these arguments:
 *   - High fragmentation should be avoided;
 *   - Block table should not be large;
 *   - Even a single byte file occupies a block. Therefore there should be
 *     sufficiently many blocks to reduce space losses and support enough files.
 *
 * Blocks in the same segment are treated specially by the allocation algorithm.
 * The value of BLOCKS_PER_SEGMENT can be 4, 8 or 16 due to the implementation
 * of the algorithm. 4 seems generally fair; it could be specialized per
 * platform.
 */
#define BLOCKS_PER_SEGMENT 4
#define BLOCK_SIZE         (EXT_FLASH_SECTOR_SIZE / BLOCKS_PER_SEGMENT)

/* Block address */
typedef uint16_t blk_t;
#define BLOCK_INVAL ((blk_t)-1)

/*
 * A block is further divided in "chunks". A chunk is small enough that it
 * can be read into memory entirely, and also possibly holds the data checksum
 * at the end. Memory buffers will match the size of chunks.
 *
 * Since a block has the next block number in the end, blocks don't divide in
 * chunks well. We will use a size of 2^n - 1 to minimize space loss. Note that
 * the number of chunks in a block must then be at least the count of bytes
 * needed for block service information (next block number).
 */
#define CHUNK_SIZE      255 /* 253 for data and 2 for the checksum */
#define CHUNK_DATA_SIZE (CHUNK_SIZE - sizeof(uint16_t))

#define BLOCK_DATA_SIZE (BLOCK_SIZE / CHUNK_SIZE * CHUNK_DATA_SIZE)

COMPILE_TIME_ASSERT(BLOCK_SIZE % CHUNK_SIZE >= sizeof(blk_t), serviceSpaceCheck);

/* Open file representation in RAM */
struct fsFileControlBlock {
    mos_mutex_t  mutex;
    int8_t       id;
    uint8_t      refcount;
    blk_t        first;
    fsOff_t      size;         /* Size of data in storage */
    uint16_t     crc;
    bool         openForWrite;
};

extern mos_mutex_t fsBlkHandleMutex;

#endif /* _FS_BLOCK_COMMON_H_ */
