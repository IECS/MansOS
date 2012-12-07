/**
 * Copyright (c) 2012 the MansOS team. All rights reserved.
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

#include "fatfs.h"
#include "posix-stdio.h"
#include "structures.h"
#include <errors.h>
#include <sdcard/sdcard.h>
#include <lib/byteorder.h>
#include <lib/algo.h>
#include <ctype.h>

#if DEBUG
#define FATFS_DEBUG 1
#endif

#if FATFS_DEBUG
#include <lib/dprint.h>
#define DPRINTF(...) PRINTF(__VA_ARGS__)
#else
#define DPRINTF(...) do {} while (0)
#endif

// ------------------------------------------------
// variables

FatInfo_t fatInfo;

static uint8_t cacheBuffer[SDCARD_SECTOR_SIZE];
static uint32_t cacheBlockNumber;
static bool cacheDirty;
static uint32_t cacheMirrorBlock;

// ------------------------------------------------
// functions

static void printFatInfo(FatInfo_t *fi) {
#if DEBUG
    PRINTF("blocksPerFat=%u\n", fatInfo.blocksPerFat);
    PRINTF("reservedSectorCount=%u\n", fbs->reservedSectorCount);
    PRINTF("dataStartBlock=%u\n", fatInfo.dataStartBlock);
    PRINTF("numFATs=%u\n", (uint16_t) fi->fatCount);
    PRINTF("numRootEntries=%u\n", (uint16_t) fi->rootDirEntryCount);
#endif
}

static void printFatEntry(DirectoryEntry_t *entry) {
#if DEBUG
    char name[9] = {0};
    char extension[4] = {0};
    memcpy(name, entry->filename, 8);
    memcpy(extension, entry->extension, 3);
    PRINTF("entry: %s.%s\n", name, extension);
#endif
}

static bool cacheFlush(void)
{
    if (cacheDirty) {
        if (!sdcardWriteBlock(cacheBlockNumber * SDCARD_SECTOR_SIZE, cacheBuffer)) {
            goto fail;
        }
        // mirror FAT tables
        if (cacheMirrorBlock) {
            if (!sdcardWriteBlock(cacheMirrorBlock * SDCARD_SECTOR_SIZE, cacheBuffer)) {
                goto fail;
            }
            cacheMirrorBlock = 0;
        }
        cacheDirty = 0;
    }
    return true;
    
  fail:
    return false;
}

static bool cacheRawBlock(uint32_t blockNumber, bool dirty)
{
    if (cacheBlockNumber != blockNumber) {
        if (!cacheFlush()) goto fail;
        if (!sdcardReadBlock(blockNumber * SDCARD_SECTOR_SIZE, cacheBuffer)) goto fail;
        cacheBlockNumber = blockNumber;
    }
    if (dirty) cacheDirty = true;
    return true;

  fail:
    return false;
}

bool fatFsInitPartition(uint8_t partition)
{
    cacheDirty = false;
    cacheMirrorBlock = 0;
    cacheBlockNumber = ~0ul;

    DPRINTF("fatFsInitPartition %d\n", partition);

    uint32_t volumeStartBlock = 0x0;

    if (partition) {
        if (partition > 4) {
            // TODO: errno
            goto fail;
        }

        if (!cacheRawBlock(0x0, false)) {
            goto fail;
        }

        PartitionTable_t* p = & ((mbr_t *)cacheBuffer)->part[partition - 1];
        if ((p->boot & 0x7F) != 0  ||
              p->totalSectors < 100 ||
              p->firstSector == 0) {
            // not a valid partition
            goto fail;
        }
        volumeStartBlock = p->firstSector;
    }
    if (!cacheRawBlock(volumeStartBlock, false)) {
        goto fail;
    }

    uint32_t totalBlocks;

    Fat32BootBlock_t *fbs = (Fat32BootBlock_t *) cacheBuffer;
    if (fbs->bytesPerSector != 512 ||
            fbs->fatCount == 0 ||
            fbs->reservedSectorCount == 0 ||
            fbs->sectorsPerCluster == 0) {
        // not valid FAT volume
        goto fail;
    }
    fatInfo.fatCount = fbs->fatCount;
    fatInfo.blocksPerFat = fbs->sectorsPerFat16 ?
            fbs->sectorsPerFat16 : fbs->sectorsPerFat32;

    fatInfo.bytesPerClusterShift = 9 + log2(fbs->sectorsPerCluster);
    fatInfo.bytesPerClusterMask = (1 << fatInfo.bytesPerClusterShift) - 1;

    fatInfo.fatStartBlock = volumeStartBlock + fbs->reservedSectorCount;
    // count for FAT16, zero for FAT32
    fatInfo.rootDirEntryCount = fbs->rootDirEntryCount;

    // directory start for FAT16, dataStart for FAT32
    fatInfo.rootDirStart = fatInfo.fatStartBlock + fbs->fatCount * fatInfo.blocksPerFat;

    // data start for FAT16 and FAT32
    fatInfo.dataStartBlock = fatInfo.rootDirStart
            + ALIGN_UP_U32(32ul * fbs->rootDirEntryCount, 512) / 512;

    // total blocks for FAT16 or FAT32
    totalBlocks = fbs->totalSectors16 ?
            fbs->totalSectors16 : fbs->totalSectors32;

    // total data blocks
    fatInfo.clusterCount = totalBlocks - (fatInfo.dataStartBlock - volumeStartBlock);
    // divide by cluster size to get cluster count
    fatInfo.clusterCount >>= (fatInfo.bytesPerClusterShift - 9);

    // FAT type is determined by cluster count
    if (fatInfo.clusterCount < 4085) {
        DPRINTF("FAT12 not supported!\n");
        goto fail;
    } else if (fatInfo.clusterCount < 65525) {
        // fatType_ = 16;
    } else {
        DPRINTF("FAT32 not supported!\n");
        goto fail;
        // rootDirStart_ = fbs->fat32RootCluster;
        // fatType_ = 32;
    }

    printFatInfo(&fatInfo);

    return true;

  fail:
    DPRINTF("fail\n");
    errno = ENXIO;
    return false;
}

uint16_t fatFsAllocateCluster(void)
{
    return 4; // right :D
}

void fatFsFileRemove(const char *filename)
{
    uint16_t directoryEntry;
    DirectoryEntry_t *de = fatFsFileSearch(filename, &directoryEntry);
    if (de) {
        de->filename[0] = FILENAME_DELETED;
        cacheDirty = true;
    }
}

static inline bool fatNameSet(const char *name, uint8_t filename[8], uint8_t extension[3])
{
    uint8_t i, j;
    if (name[0] == 0xE5) {
        filename[0] = FILENAME_E5;
    } else {
        filename[0] = toupper(name[0]);
    }
    bool copyName = true;
    bool copyExtension = false;
    for (i = 1, j = 1; j < 8; ++j) {
        if (copyName) {
            if (name[i] == '.') {
                i++;
                copyExtension = true;
                copyName = false;
            }
            if (name[i] == 0) {
                copyName = false;
            }
        }
        if (copyName) {
            filename[j] = toupper(name[i]);
            i++;
        } else {
            filename[j] = ' '; // padding spaces
        }
    }
    if (name[i] == '.') {
        i++;
        copyExtension = true;
    }
    for (j = 0; j < 3; ++j) {
        if (copyExtension) {
            extension[j] = toupper(name[i]);
            i++;
        } else {
            extension[j] = ' '; // padding spaces
        }
    }
    return name[i] == 0; // return false if too long
}

static inline bool fatNameMatch(DirectoryEntry_t *entry, 
                                uint8_t filename[8], uint8_t extension[3])
{
    return !memcmp(filename, entry->filename, 8)
            && !memcmp(extension, entry->extension, 3);
}

static inline bool fatCharacterAcceptable(char c)
{
    if (isalnum(c)) return true;
    if (c == ' ') return true;
    if (c >= 128) return true;
    if (c == '!' || c == '#'
            || c =='$' || c == '%'
            || c =='&' || c == '\''
            || c =='(' || c ==')'
            || c == '-' || c == '@'
            || c == '^' || c == '_'
            || c == '`' || c =='{'
            || c == '}' || c == '~') {
        return true;
    }
    return false;
}

bool fatNameAcceptable(const char *name)
{
    if (*name == 0) return false;

    uint8_t i, j;
    // filename
    for (i = 0; i < 8; ++i) {
        if (name[i] == 0) return true;
        if (name[i] == '.') {
            i++;
            // extension
            for (j = 0; j < 3; ++j) {
                if (name[i + j] == 0) return true;
                if (!fatCharacterAcceptable(name[i + j])) return false;
            }
            i += j;
            break;
        }
        if (!fatCharacterAcceptable(name[i])) return false;
    }

    // too long?
    return name[i] == 0;
}

DirectoryEntry_t *fatFsFileSearch(const char *__restrict name, uint16_t *__restrict entryIndex /* out */)
{
    uint16_t i;
    sector_t numRootSectors = fatInfo.rootDirEntryCount * 32 / SDCARD_SECTOR_SIZE;

    uint8_t filename[9] = {0};
    uint8_t extension[4] = {0};
    if (!fatNameSet(name, filename, extension)) {
        goto fail;
    }

    for (i = 0; i < numRootSectors; ++i) {
        if (!cacheRawBlock(fatInfo.rootDirStart + i, false)) {
            goto fail;
        }
        uint16_t j;
        *entryIndex = 0;
        DirectoryEntry_t *entries = (DirectoryEntry_t *) cacheBuffer;
        for (j = 0; j < SDCARD_SECTOR_SIZE / 32; ++j) {
            (*entryIndex)++;
            // DPRINTF("check entry %u\n", j);
            printFatEntry(&entries[j]);
            if (fatNameMatch(&entries[j], filename, extension)) {
                // DPRINTF("fatFsFileOpen: matched!\n", name);
                return &entries[j];
            }
        }
        break;
    }
    DPRINTF("fatFsFileOpen: %s not found!\n", name);

  fail:
    return NULL;
}

DirectoryEntry_t *fatFsFileCreate(const char *name)
{
    DPRINTF("fatFsFileCreate... %s\n", name);

    uint16_t i;
    sector_t numRootSectors = fatInfo.rootDirEntryCount * 32 / SDCARD_SECTOR_SIZE;

    uint8_t filename[9] = {0};
    uint8_t extension[4] = {0};
    if (!fatNameSet(name, filename, extension)) {
        goto fail;
    }

    for (i = 0; i < numRootSectors; ++i) {
        if (!cacheRawBlock(fatInfo.rootDirStart + i, false)) {
            DPRINTF("cacheRawBlock failed\n");
            goto fail;
        }
        uint16_t j;
        DirectoryEntry_t *entries = (DirectoryEntry_t *) cacheBuffer;
        for (j = 0; j < SDCARD_SECTOR_SIZE / 32; ++j) {
            DirectoryEntry_t *entry = &entries[j];
            if (entry->filename[0] == FILENAME_UNUSED
                    || entry->filename[0] == FILENAME_DELETED) {
                memcpy(entry->filename, filename, 8);
                memcpy(entry->extension, extension, 3);
                memset(entry + 11, 0, sizeof(*entry) - 11);
                cacheDirty = true;
                return entry;
            }
        }
    }

  fail:
    return NULL;
}

void fatFsFileFlush(FILE *handle)
{
    // TODO: something smarter?
    cacheFlush();
}

void fatFsFileClose(FILE *handle)
{
    fatFsFileFlush(handle);
}

uint16_t fatFsRead(FILE *handle, void *buffer, uint16_t maxLength)
{
    uint16_t offsetInCluster = handle->position & fatInfo.bytesPerClusterMask;
    uint16_t offsetInBlock = offsetInCluster & (SDCARD_SECTOR_SIZE - 1);
    uint16_t blockNumber = fatInfo.dataStartBlock + clusterToBlock(handle->currentCluster - 2);

    // check the file size and do not allow to read too much
    if (handle->position + maxLength > handle->fileSize) {
        maxLength = handle->fileSize - handle->position;
    }

    cluster_t nextCluster;
    if ((offsetInCluster + maxLength) >= (1 << fatInfo.bytesPerClusterShift)) {
        // will use two clusters
        nextCluster = 0;
    } else {
        // will use just one cluster
        nextCluster = 0;
    }

    uint16_t maxLength2 = offsetInBlock + maxLength;
    if (maxLength2 > SDCARD_SECTOR_SIZE) {
        // will use two reads
        maxLength2 -= SDCARD_SECTOR_SIZE;
        maxLength -= maxLength2;
    } else {
        // one read will do
        maxLength2 = 0;
    }

    DPRINTF("read from block %d\n", blockNumber);
    if (!cacheRawBlock(blockNumber, false)) {
        DPRINTF("fatFsRead: raw read failed\n");
        return 0;
    }
    memcpy(buffer, cacheBuffer + offsetInBlock, maxLength);
    handle->position += maxLength;

    if (maxLength2) {
        if (nextCluster) {
            // TODO: set to the first block in the next cluster
            blockNumber = 0;
        } else {
            // simply advance the block number by one
            blockNumber += 1;
        }
        if (!cacheRawBlock(blockNumber, false)) {
            DPRINTF("fatFsRead: raw read (2) failed\n");
            return 0;
        }
        memcpy(buffer + maxLength, cacheBuffer, maxLength2);
        handle->position += maxLength2;
    }

    if (nextCluster) {
        // advance the cluster
        handle->currentCluster = nextCluster;
    }
    return maxLength + maxLength2;
}

uint16_t fatFsWrite(FILE *handle, const void *buffer, uint16_t length)
{
    if ((handle->flags & O_APPEND) && handle->position != handle->fileSize) {
        // TODO: handle append
        return 0;
    }

    // if currentCluster is 0, allocate space on disk!
    if (handle->currentCluster == 0) {
        handle->currentCluster = fatFsAllocateCluster();
        // TODO: set it on disk (in dir entry!)
    }

    uint16_t offsetInCluster = handle->position & fatInfo.bytesPerClusterMask;
    uint16_t offsetInBlock = offsetInCluster & (SDCARD_SECTOR_SIZE - 1);
    uint16_t blockNumber = fatInfo.dataStartBlock + clusterToBlock(handle->currentCluster - 2);

    cluster_t nextCluster;
    if ((offsetInCluster + length) >= (1 << fatInfo.bytesPerClusterShift)) {
        // will use two clusters
        nextCluster = fatFsAllocateCluster();
    } else {
        // will use just one cluster
        nextCluster = 0;
    }

    uint16_t length2 = offsetInBlock + length;
    if (length2 > SDCARD_SECTOR_SIZE) {
        // will use two writes
        length2 -= SDCARD_SECTOR_SIZE;
        length -= length2;
    } else {
        // one write will do
        length2 = 0;
    }
    DPRINTF("write in block %d\n", blockNumber);
    if (!cacheRawBlock(blockNumber, true)) {
        DPRINTF("fatFsWrite: raw read failed\n");
        return 0;
    }
    memcpy(cacheBuffer + offsetInBlock, buffer, length);
    handle->position += length;

    if (length2) {
        if (nextCluster) {
            // TODO: allocate new cluster

            // set to the first block in the next cluster
            blockNumber = 0;
        } else {
            // simply advance the block number by one
            blockNumber += 1;
        }
        if (!cacheRawBlock(blockNumber, true)) {
            DPRINTF("fatFsWrite: raw read (2) failed\n");
            return 0;
        }
        memcpy(cacheBuffer, buffer + length, length2);
        handle->position += length2;
    }

    if (nextCluster) {
        // advance the cluster
        handle->currentCluster = nextCluster;
    }
    return length;
}
