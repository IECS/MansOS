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
//#include <extflash.h>
#include "mmc.h"
#include "sdcard_hal.h"

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
} Superblock_t PACKED;

// -- implementation: defines & compile time configuration

#define BLOCK_SIZE 0x200 // 512 bytes
#define CRC_SIZE   0x2   // 2 bytes
#define CRC_ALGO   CRC_16

#define CHECKSUM_OFFSET (BLOCK_SIZE - CRC_SIZE)

// the offset shoud be such that each next reserved block is in a new segment
#undef SECTOR_SIZE
#define SECTOR_SIZE   0x10000ul // 64k
//#define SECTOR_SIZE     0x200u // 0.5k - for RAM sdcard emaulation

#define RESERVED_BLOCK_OFFSET (SECTOR_SIZE * 8)

//#define SDCARD_DISK_SIZE (SECTOR_SIZE * 16)
#define SDCARD_DISK_SIZE (SECTOR_SIZE * 256) // 16 MB

#define SUPERBLOCK_ADDR         0x00000ul
#define SUPERBLOCK_BACKUP_ADDR  (SUPERBLOCK_ADDR + RESERVED_BLOCK_OFFSET)

#define DATA_START  (SUPERBLOCK_ADDR + SECTOR_SIZE)

#define SDCARD_ERROR  ~0u

// works for powers of 2
#define IS_ALIGNED(addr, al) (!((addr) & (al - 1)))
#define GET_BLOCK_START_ADDR(addr) ((addr) & ~(BLOCK_SIZE - 1))

static uint32_t sdcardReadAddr = DATA_START;

static uint8_t writeBuffer[BLOCK_SIZE];
static uint16_t writeBufferOffset;

static uint8_t tmpBuffer[BLOCK_SIZE];

// -- implementation: function definitions

void extSdcardRead(unsigned long address, void *buffer, unsigned long bufferSize)
{
    if (bufferSize == BLOCK_SIZE) {
        mmcReadBlock(address, BLOCK_SIZE, buffer);
    } else {
        mmcReadBlock(address, BLOCK_SIZE, tmpBuffer);
        memcpy(buffer, tmpBuffer, bufferSize);
    }
}

void extSdcardWrite(unsigned long address, void *buffer, unsigned long bufferSize)
{
    if (bufferSize == BLOCK_SIZE) {
        mmcWriteBlock(address, BLOCK_SIZE, buffer);
    } else {
        memset(tmpBuffer, 0, sizeof(tmpBuffer));
        memcpy(tmpBuffer, buffer, bufferSize);
        mmcWriteBlock(address, BLOCK_SIZE, tmpBuffer);
    }
}

// -- implementation: function definitions

static bool verifyCrc(uint32_t blockStart, const void *blockData, uint16_t usedSize)
{
    uint16_t readCrc, calcCrc;
    extSdcardRead(blockStart + CHECKSUM_OFFSET, &readCrc, sizeof(readCrc));
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

    extSdcardRead(GET_BLOCK_START_ADDR(addr), buffer, BLOCK_SIZE);
    calcCrc = crc16(buffer, CHECKSUM_OFFSET);
    if (calcCrc != *pcrc) {
        FPRINTF("readVerifyCrc: checksums do not match: 0x%x vs 0x%x\n", *pcrc, calcCrc);
    }
    return calcCrc == *pcrc;
}

static inline void extSdcardWriteSector(uint32_t addr, void *data, uint32_t dataLen)
{
    ASSERT(addr < SDCARD_DISK_SIZE);
//    extSdcardEraseSector(addr);
    extSdcardWrite(addr, data, dataLen);
}

static inline void extSdcardWriteBlock(uint32_t addr, void *data)
{
    uint16_t *crc = (uint16_t *)(data + CHECKSUM_OFFSET);
    ASSERT(addr < SDCARD_DISK_SIZE);
    *crc = crc16((const uint8_t *) data, CHECKSUM_OFFSET);
    extSdcardWrite(addr, data, BLOCK_SIZE);
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
    extSdcardWriteSector(SUPERBLOCK_ADDR, buffer, sizeof(buffer));
}

//
// Read superblock, verify it, get offset from it
//
static uint32_t getDataWritePos(uint32_t sblockAddress)
{
    Superblock_t superblock;
    extSdcardRead(sblockAddress, &superblock, sizeof(superblock));

    if (superblock.dataEnd < DATA_START
            || !verifyCrc(sblockAddress, &superblock, sizeof(superblock))) {
        if (sblockAddress == SUPERBLOCK_ADDR) {
            uint32_t result = getDataWritePos(SUPERBLOCK_BACKUP_ADDR);
            if (result != SDCARD_ERROR) restoreSuperblock(&superblock);
            return result;
        }
        return SDCARD_ERROR;
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

    extSdcardWriteSector(SUPERBLOCK_ADDR, buffer, BLOCK_SIZE);

    extSdcardWriteSector(SUPERBLOCK_BACKUP_ADDR, buffer, BLOCK_SIZE);

    memset(buffer, 0, sizeof(buffer));
    // verify superblocks (at least one of them must be OK)
    extSdcardRead(SUPERBLOCK_ADDR, buffer, BLOCK_SIZE);
    if (superblock->dataEnd == writePos
            && superblock->blockSize == BLOCK_SIZE
            && superblock->checksumSize == CRC_SIZE
            && superblock->checksumAlgo == CRC_ALGO
            && *pcrc == calcCrc) {
        return true;
    }

    extSdcardRead(SUPERBLOCK_BACKUP_ADDR, buffer, BLOCK_SIZE);
    if (superblock->dataEnd == writePos
            && superblock->blockSize == BLOCK_SIZE
            && superblock->checksumSize == CRC_SIZE
            && superblock->checksumAlgo == CRC_ALGO
            && *pcrc == calcCrc) return true;

    return false;
}

void sdcardStreamClear()
{ 
//    extSdcardBulkErase();

    updateSuperblocks(DATA_START);
}

void sdcardStreamSeek(uint32_t addr)
{
    // XXX: this does not make much sense at the moment from users PoV
    sdcardReadAddr = addr;
    if (sdcardReadAddr < DATA_START) sdcardReadAddr = DATA_START;
}

void sdcardStreamSeekToNewBlock(void)
{
    sdcardReadAddr = GET_BLOCK_START_ADDR(sdcardReadAddr)
            + BLOCK_SIZE;
}

static bool sdcardStreamVerifyBlockChecksum(uint32_t blockAddr, uint16_t sizeInBlock)
{
    uint8_t buffer[BLOCK_SIZE];
    uint16_t *pcrc = (uint16_t *) (buffer + CHECKSUM_OFFSET);
    bool blockStatus;

    extSdcardRead(blockAddr, buffer, BLOCK_SIZE);

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

bool sdcardStreamVerifyChecksums()
{
    bool result = true;
    uint32_t dataEnd = getDataWritePos(SUPERBLOCK_ADDR);
    uint32_t readPos = SUPERBLOCK_ADDR;

    FPRINTF("sdcardStreamVerifyChecksums: dataEnd=0x%lx\n", dataEnd);

    uint32_t endBlockStart = GET_BLOCK_START_ADDR(dataEnd);

    while (readPos < endBlockStart) {
        uint16_t sizeInBlock = CHECKSUM_OFFSET;
        if (readPos + BLOCK_SIZE > dataEnd) {
            sizeInBlock = dataEnd - readPos;
        }
        if (!sdcardStreamVerifyBlockChecksum(readPos, sizeInBlock)) {
            result = false;
        }
        readPos += IS_ALIGNED(readPos, RESERVED_BLOCK_OFFSET) ?
                SECTOR_SIZE : BLOCK_SIZE;
    }
    return result;
}

bool sdcardStreamRead(void *buffer, uint16_t *dataLen)
{
    uint32_t dataEnd = getDataWritePos(SUPERBLOCK_ADDR);
    uint16_t dataSize;

//     FPRINTF("sdcardStreamRead: sdcardReadAddr=%lu, len=%u\n",
//             sdcardReadAddr, *dataLen);

    if (sdcardReadAddr >= dataEnd) return false;

    dataSize = *dataLen;
    if (dataSize > dataEnd - sdcardReadAddr) {
        dataSize = dataEnd - sdcardReadAddr;
        *dataLen = dataSize;
    }

    uint16_t dataToRead = dataSize;
    uint32_t lastBlockStart = GET_BLOCK_START_ADDR(sdcardReadAddr);
    if (sdcardReadAddr == lastBlockStart + CHECKSUM_OFFSET) {
        sdcardReadAddr += 2;
        if (IS_ALIGNED(sdcardReadAddr, RESERVED_BLOCK_OFFSET)) {
            sdcardReadAddr += SECTOR_SIZE;
        }
        lastBlockStart = sdcardReadAddr;
    }
    uint16_t bytesToReadInBlock = BLOCK_SIZE - CRC_SIZE - (sdcardReadAddr - lastBlockStart);
    uint16_t bufferOffset = 0;
    for (;;) {
        if (bytesToReadInBlock > dataToRead) bytesToReadInBlock = dataToRead;

        if (!readAndVerifyCrc(sdcardReadAddr)) return false;

        extSdcardRead(sdcardReadAddr, buffer + bufferOffset, bytesToReadInBlock);

        sdcardReadAddr += bytesToReadInBlock;
        dataToRead -= bytesToReadInBlock;
        if (!dataToRead) break;

        bufferOffset += bytesToReadInBlock;
        bytesToReadInBlock = CHECKSUM_OFFSET;

        sdcardReadAddr += CRC_SIZE; // skip CRC

        ASSERT(IS_ALIGNED(sdcardReadAddr, BLOCK_SIZE));
        if (IS_ALIGNED(sdcardReadAddr, RESERVED_BLOCK_OFFSET)) {
            sdcardReadAddr += BLOCK_SIZE;
        }
    }

    return true;
}

bool sdcardStreamWrite(const uint8_t *data, uint16_t dataSize)
{
    if (dataSize + writeBufferOffset < CHECKSUM_OFFSET) {
        memcpy(writeBuffer + writeBufferOffset, data, dataSize);
        writeBufferOffset += dataSize;
        return true;
    }

    uint32_t dataOffset = getDataWritePos(SUPERBLOCK_ADDR);
    if (dataOffset == SDCARD_ERROR
            || dataOffset >= SDCARD_DISK_SIZE) {
        return false;
    }

    ASSERT(IS_ALIGNED(dataOffset, BLOCK_SIZE));
    ASSERT(!IS_ALIGNED(dataOffset, RESERVED_BLOCK_OFFSET));

//     FPRINTF("sdcardStreamWrite: dataOffset=0x%lx, dataSize=%u\n",
//             dataOffset, dataSize);

    while (dataSize + writeBufferOffset >= CHECKSUM_OFFSET) {
        uint16_t t = CHECKSUM_OFFSET - writeBufferOffset;
        memcpy(writeBuffer + writeBufferOffset, data, t);
        data += t;
        dataSize -= t;

        if (dataOffset >= SDCARD_DISK_SIZE) return false;

        extSdcardWriteBlock(dataOffset, writeBuffer);
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

bool sdcardStreamFlush(void)
{
    if (!writeBufferOffset) return true;
    uint32_t dataOffset = getDataWritePos(SUPERBLOCK_ADDR);
    if (dataOffset == SDCARD_ERROR
            || dataOffset >= SDCARD_DISK_SIZE) {
        return false;
    }
    // fill unused block space rest with zeros
    memset(writeBuffer + writeBufferOffset, 0,
            CHECKSUM_OFFSET - writeBufferOffset);
    extSdcardWriteBlock(dataOffset, writeBuffer);
    writeBufferOffset = 0;
    dataOffset += BLOCK_SIZE;
    if (IS_ALIGNED(dataOffset, RESERVED_BLOCK_OFFSET)) {
        dataOffset += SECTOR_SIZE;
    }
    if (!updateSuperblocks(dataOffset)) return false;
    return true;
}
