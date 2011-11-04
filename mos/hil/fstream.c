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
// Flash stream module code.
// Note that non-static 256 byte (BLOCK_SIZE) non-static 
// buffers are used in functions.
//

#include <lib/codec/crc.h>
#include <lib/dprint.h>
#include <lib/assert.h>
#include <string.h>
#include <extflash.h>

#if 0
#define FPRINTF PRINTF
#else
#define FPRINTF(...) // nothing
#endif

// -- implementation: types

// all checksum algos enumerated
enum {
    CRC_16 = 1,
    CRC_32 = 2,
    TOTAL_CRC_ALGOS
};

// filesystem's superblock
typedef struct Superblock {
    uint32_t dataEnd;     // data end address
    uint16_t blockSize;   // block size in bytes
    uint8_t checksumSize; // checksum size (per block) in bytes
    uint8_t checksumAlgo; // checksum algortihm identifier
} Superblock_t;

// -- implementation: defines & compile time configuration

#define BLOCK_SIZE 0x100 // 256 bytes
#define CRC_SIZE   0x2   // 2 bytes
#define CRC_ALGO   CRC_16

#define CHECKSUM_OFFSET (BLOCK_SIZE - CRC_SIZE)

// the offset shoud be such that each next reserved block is ain a new segment
#define SECTOR_SIZE   0x10000ul // 64k
//#define SECTOR_SIZE     0x200u // 0.5k - for RAM flash emaulation

#define RESERVED_BLOCK_OFFSET (SECTOR_SIZE * 8)

#define FLASH_DISK_SIZE (SECTOR_SIZE * 16)

#define SUPERBLOCK_ADDR         0x00000ul
#define SUPERBLOCK_BACKUP_ADDR  (SUPERBLOCK_ADDR + RESERVED_BLOCK_OFFSET)

#define DATA_START  (SUPERBLOCK_ADDR + SECTOR_SIZE)

#define FLASH_ERROR  ~0u

#define GET_BLOCK_START_ADDR(addr) ALIGN_DOWN(addr, BLOCK_SIZE)

static uint32_t flashReadAddr = DATA_START;

static uint8_t writeBuffer[BLOCK_SIZE];
static uint16_t writeBufferOffset;

// -- implementation: function definition

static bool verifyCrc(uint32_t blockStart, const void *blockData, uint16_t usedSize)
{
    uint16_t readCrc, calcCrc;
    extFlashRead(blockStart + CHECKSUM_OFFSET, &readCrc, sizeof(readCrc));
    calcCrc = crc16((const uint8_t *) blockData, usedSize);
    if (readCrc != calcCrc) {
        FPRINTF("verifyCrc: checksums do not match: 0x%x vs 0x%x\n", readCrc, calcCrc);
    }
    return readCrc == calcCrc;
}

static inline bool readAndVerifyCrc(uint32_t addr)
{
    uint8_t buffer[BLOCK_SIZE];
    uint16_t *pcrc = (uint16_t *) (buffer + CHECKSUM_OFFSET);
    uint16_t calcCrc;

    extFlashRead(GET_BLOCK_START_ADDR(addr), buffer, BLOCK_SIZE);
    calcCrc = crc16(buffer, CHECKSUM_OFFSET);
    if (calcCrc != *pcrc) {
        FPRINTF("readVerifyCrc: checksums do not match: 0x%x vs 0x%x\n", *pcrc, calcCrc);
    }
    return calcCrc == *pcrc;
}

static inline void extFlashWriteSector(uint32_t addr, void *data, uint32_t dataLen)
{
    ASSERT(addr < FLASH_DISK_SIZE);
    extFlashEraseSector(addr);
    extFlashWrite(addr, data, dataLen);
}

static inline void extFlashWriteBlock(uint32_t addr, void *data)
{
    uint16_t *crc = (uint16_t *)(data + CHECKSUM_OFFSET);
    ASSERT(addr < FLASH_DISK_SIZE);
    *crc = crc16((const uint8_t *) data, CHECKSUM_OFFSET);
    extFlashWrite(addr, data, BLOCK_SIZE);
}

//
// Try to restore superblock from backup superblock.
// The result is not checked, so the function will not abort
// even if the the first superblock is in a bad sector.
//
static void restoreSuperblock(Superblock_t *goodSblock)
{
    uint8_t buffer[BLOCK_SIZE];
    uint16_t *crc = (uint16_t *)(buffer + CHECKSUM_OFFSET);
    memcpy(buffer, goodSblock, sizeof(Superblock_t));
    *crc = crc16((const uint8_t *) goodSblock, sizeof(*goodSblock));
    extFlashWriteSector(SUPERBLOCK_ADDR, buffer, sizeof(buffer));
}

//
// Read superblock, verify it, get offset from it
//
static uint32_t getDataWritePos(uint32_t sblockAddress)
{
    Superblock_t superblock;
    extFlashRead(sblockAddress, &superblock, sizeof(superblock));

    if (superblock.dataEnd < DATA_START
            || !verifyCrc(sblockAddress, &superblock, sizeof(superblock))) {
        if (sblockAddress == SUPERBLOCK_ADDR) {
            uint32_t result = getDataWritePos(SUPERBLOCK_BACKUP_ADDR);
            if (result != FLASH_ERROR) restoreSuperblock(&superblock);
            return result;
        }
        return FLASH_ERROR;
    }

    return superblock.dataEnd;
}

//
// Update both superblocks with new data end position.
//
static bool updateSuperblocks(uint32_t writePos)
{
    uint8_t buffer[BLOCK_SIZE];
    Superblock_t *superblock = (Superblock_t *) buffer;
    uint16_t *pcrc = (uint16_t *) (buffer + CHECKSUM_OFFSET);
    uint16_t calcCrc;

    memset(buffer, 0, BLOCK_SIZE);

    // write metainfo
    superblock->dataEnd = writePos;
    superblock->blockSize = BLOCK_SIZE;
    superblock->checksumSize = CRC_SIZE;
    superblock->checksumAlgo = CRC_ALGO;

    // write crc
    calcCrc = crc16((uint8_t *) superblock, sizeof(*superblock));
    *pcrc = calcCrc;

    extFlashWriteSector(SUPERBLOCK_ADDR, buffer, BLOCK_SIZE);

    extFlashWriteSector(SUPERBLOCK_BACKUP_ADDR, buffer, BLOCK_SIZE);

    memset(buffer, 0, sizeof(buffer));
    // verify superblocks (at least one of them must be OK)
    extFlashRead(SUPERBLOCK_ADDR, buffer, BLOCK_SIZE);
    if (superblock->dataEnd == writePos
            && superblock->blockSize == BLOCK_SIZE
            && superblock->checksumSize == CRC_SIZE
            && superblock->checksumAlgo == CRC_ALGO
            && *pcrc == calcCrc) {
        return true;
    }

    extFlashRead(SUPERBLOCK_BACKUP_ADDR, buffer, BLOCK_SIZE);
    if (superblock->dataEnd == writePos
            && superblock->blockSize == BLOCK_SIZE
            && superblock->checksumSize == CRC_SIZE
            && superblock->checksumAlgo == CRC_ALGO
            && *pcrc == calcCrc) return true;

    return false;
}

void flashStreamClear()
{ 
    extFlashBulkErase();

    updateSuperblocks(DATA_START);
}

void flashStreamSeek(uint32_t addr)
{
    // XXX: this does not make much sense at the moment from users PoV
    flashReadAddr = addr;
    if (flashReadAddr < DATA_START) flashReadAddr = DATA_START;
}

void flashStreamSeekToNewBlock(void)
{
    flashReadAddr = GET_BLOCK_START_ADDR(flashReadAddr)
            + BLOCK_SIZE;
}

static bool flashStreamVerifyBlockChecksum(uint32_t blockAddr, uint16_t sizeInBlock)
{
    uint8_t buffer[BLOCK_SIZE];
    uint16_t *pcrc = (uint16_t *) (buffer + CHECKSUM_OFFSET);
    bool blockStatus;

    extFlashRead(blockAddr, buffer, BLOCK_SIZE);

    if (IS_ALIGNED(blockAddr, RESERVED_BLOCK_OFFSET)) {
        if (blockAddr == SUPERBLOCK_ADDR || blockAddr == SUPERBLOCK_BACKUP_ADDR) {
            blockStatus = (crc16(buffer, sizeof(Superblock_t)) == *pcrc);
            FPRINTF("superblock: %s\n", (blockStatus ? "ok" : "error"));
        } else {
            blockStatus = true;
            FPRINTF("reserved-block: ok\n");
        }
    } else {
        blockStatus = (crc16(buffer, sizeInBlock) == *pcrc);
        FPRINTF("block 0x%lx: %s\n", blockAddr, (blockStatus ? "ok" : "error"));
    }
    return blockStatus;
}

bool flashStreamVerifyChecksums()
{
    bool result = true;
    uint32_t dataEnd = getDataWritePos(SUPERBLOCK_ADDR);
    uint32_t readPos = SUPERBLOCK_ADDR;

    FPRINTF("flashStreamVerifyChecksums: dataEnd=0x%lx\n", dataEnd);

    uint32_t endBlockStart = GET_BLOCK_START_ADDR(dataEnd);

    while (readPos < endBlockStart) {
        uint16_t sizeInBlock = CHECKSUM_OFFSET;
        if (readPos + BLOCK_SIZE > dataEnd) {
            sizeInBlock = dataEnd - readPos;
        }
        if (!flashStreamVerifyBlockChecksum(readPos, sizeInBlock)) {
            result = false;
        }
        readPos += IS_ALIGNED(readPos, RESERVED_BLOCK_OFFSET) ?
                SECTOR_SIZE : BLOCK_SIZE;
    }
    return result;
}

bool flashStreamRead(void *buffer, uint16_t *dataLen)
{
    uint32_t dataEnd = getDataWritePos(SUPERBLOCK_ADDR);
    uint16_t dataSize;

//     FPRINTF("flashStreamRead: flashReadAddr=%lu, len=%u\n",
//             flashReadAddr, *dataLen);

    if (flashReadAddr >= dataEnd) return false;

    dataSize = *dataLen;
    if (dataSize > dataEnd - flashReadAddr) {
        dataSize = dataEnd - flashReadAddr;
        *dataLen = dataSize;
    }

    uint16_t dataToRead = dataSize;
    uint32_t lastBlockStart = GET_BLOCK_START_ADDR(flashReadAddr);
    if (flashReadAddr == lastBlockStart + CHECKSUM_OFFSET) {
        flashReadAddr += 2;
        if (IS_ALIGNED(flashReadAddr, RESERVED_BLOCK_OFFSET)) {
            flashReadAddr += SECTOR_SIZE;
        }
        lastBlockStart = flashReadAddr;
    }
    uint16_t bytesToReadInBlock = BLOCK_SIZE - CRC_SIZE - (flashReadAddr - lastBlockStart);
    uint16_t bufferOffset = 0;
    for (;;) {
        if (bytesToReadInBlock > dataToRead) bytesToReadInBlock = dataToRead;

        if (!readAndVerifyCrc(flashReadAddr)) return false;

        extFlashRead(flashReadAddr, buffer + bufferOffset, bytesToReadInBlock);

        flashReadAddr += bytesToReadInBlock;
        dataToRead -= bytesToReadInBlock;
        if (!dataToRead) break;

        bufferOffset += bytesToReadInBlock;
        bytesToReadInBlock = CHECKSUM_OFFSET;

        flashReadAddr += CRC_SIZE; // skip CRC

        ASSERT(IS_ALIGNED(flashReadAddr, BLOCK_SIZE));
        if (IS_ALIGNED(flashReadAddr, RESERVED_BLOCK_OFFSET)) {
            flashReadAddr += BLOCK_SIZE;
        }
    }

    return true;
}

bool flashStreamWrite(const uint8_t *data, uint16_t dataSize)
{
    if (dataSize + writeBufferOffset < CHECKSUM_OFFSET) {
        memcpy(writeBuffer + writeBufferOffset, data, dataSize);
        writeBufferOffset += dataSize;
        return true;
    }

    uint32_t dataOffset = getDataWritePos(SUPERBLOCK_ADDR);
    if (dataOffset == FLASH_ERROR
            || dataOffset >= FLASH_DISK_SIZE) {
        return false;
    }

    ASSERT(IS_ALIGNED(dataOffset, BLOCK_SIZE));
    ASSERT(!IS_ALIGNED(dataOffset, RESERVED_BLOCK_OFFSET));

//     FPRINTF("flashStreamWrite: dataOffset=0x%lx, dataSize=%u\n",
//             dataOffset, dataSize);

    while (dataSize + writeBufferOffset >= CHECKSUM_OFFSET) {
        uint16_t t = CHECKSUM_OFFSET - writeBufferOffset;
        memcpy(writeBuffer + writeBufferOffset, data, t);
        data += t;
        dataSize -= t;

        if (dataOffset >= FLASH_DISK_SIZE) return false;

        extFlashWriteBlock(dataOffset, writeBuffer);
        dataOffset += BLOCK_SIZE;
        if (IS_ALIGNED(dataOffset, RESERVED_BLOCK_OFFSET)) {
            dataOffset += SECTOR_SIZE;
        }
        writeBufferOffset = 0;
    }

    if (dataSize) memcpy(writeBuffer, data, dataSize);
    writeBufferOffset = dataSize;

    if (!updateSuperblocks(dataOffset)) return false;

    return true;
}

bool flashStreamFlush(void)
{
    if (!writeBufferOffset) return true;
    uint32_t dataOffset = getDataWritePos(SUPERBLOCK_ADDR);
    if (dataOffset == FLASH_ERROR
            || dataOffset >= FLASH_DISK_SIZE) {
        return false;
    }
    // fill unused block space rest with zeros
    memset(writeBuffer + writeBufferOffset, 0,
            CHECKSUM_OFFSET - writeBufferOffset);
    extFlashWriteBlock(dataOffset, writeBuffer);
    writeBufferOffset = 0;
    dataOffset += BLOCK_SIZE;
    if (IS_ALIGNED(dataOffset, RESERVED_BLOCK_OFFSET)) {
        dataOffset += SECTOR_SIZE;
    }
    if (!updateSuperblocks(dataOffset)) return false;
    return true;
}
