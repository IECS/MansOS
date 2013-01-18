/*
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
#include <ctype.h>
#include <lib/algo.h>
#include <lib/byteorder.h>
#include <sdcard/sdcard.h>

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

static Cache16_t cache;
static uint32_t cacheBlockNumber;
static bool cacheDirty;
static uint32_t cacheMirrorBlock;

// ------------------------------------------------
// functions

static void printFatInfo(FatInfo_t *fi) {
#if FATFS_DEBUG
    PRINTF("sectorsPerFAT=%u\n", fi->sectorsPerFAT);
    PRINTF("clusterCount=%u\n", fi->clusterCount);  
    PRINTF("dataStartBlock=%u\n", fi->dataStartBlock);
    PRINTF("numFATs=%u\n", (uint16_t) fi->numFATs);
    PRINTF("numRootEntries=%u\n", fi->numRootEntries);
#endif
}

static void printFatEntry(DirectoryEntry_t *entry) {
#if FATFS_DEBUG
    char name[9] = {0};
    char extension[4] = {0};
    memcpy(name, entry->filename, 8);
    memcpy(extension, entry->extension, 3);
    PRINTF("entry: %s.%s\n", name, extension);
    PRINTF("  cluster %u\n", entry->startClusterLoword);
    PRINTF("  size %lu\n", entry->fileSize);
#endif
}

static bool cacheFlush(void)
{
    if (cacheDirty) {
        DPRINTF("write block %lu (rootDirStart=%lu, dataStartBlock=%lu)\n",
                cacheBlockNumber, fatInfo.rootDirStart, fatInfo.dataStartBlock);
        if (!sdcardWriteBlock(cacheBlockNumber * SDCARD_SECTOR_SIZE, cache.data)) {
            goto fail;
        }
        // mirror FAT tables
        if (cacheMirrorBlock) {
            if (!sdcardWriteBlock(cacheMirrorBlock * SDCARD_SECTOR_SIZE, cache.data)) {
                goto fail;
            }
            cacheMirrorBlock = 0;
        }
        cacheDirty = false;
    }
    return true;
    
  fail:
    DPRINTF(" write fail!\n");
    return false;
}

static bool cacheRawBlock(uint32_t blockNumber, bool makeDirty)
{
    if (cacheBlockNumber != blockNumber) {
        if (!cacheFlush()) goto fail;
        DPRINTF("read block %lu\n", blockNumber);
        if (!sdcardReadBlock(blockNumber * SDCARD_SECTOR_SIZE, cache.data)) goto fail;
        cacheBlockNumber = blockNumber;
    }
    if (makeDirty) cacheDirty = true;
    return true;

  fail:
    DPRINTF(" cache fail!\n");
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

        PartitionTable_t* p = & cache.mbr.part[partition - 1];
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

    uint32_t totalSectors;

    FatBootBlock_t *fbs = &cache.fbs;
    if (le16read(fbs->bytesPerSector) != 512
            || fbs->numFATs == 0
            || le16read(fbs->numReservedSectors) == 0
            || fbs->sectorsPerCluster == 0) {
        // not valid FAT volume
        goto fail;
    }
    fatInfo.numFATs = fbs->numFATs;
    fatInfo.sectorsPerFAT = le16read(fbs->sectorsPerFAT16);
    fatInfo.bytesPerClusterShift = 9 + log2(fbs->sectorsPerCluster);
    fatInfo.bytesPerClusterMask = (1 << fatInfo.bytesPerClusterShift) - 1;

    fatInfo.fatStartBlock = volumeStartBlock + le16read(fbs->numReservedSectors);
    // count for FAT16, zero for FAT32
    fatInfo.numRootEntries = le16read(fbs->numRootEntries);

    // directory start for FAT16, dataStart for FAT32
    fatInfo.rootDirStart = fatInfo.fatStartBlock + fbs->numFATs * fatInfo.sectorsPerFAT;

    // data start for FAT16 and FAT32
    fatInfo.dataStartBlock = fatInfo.rootDirStart
            + ALIGN_UP_U32(32ul * le16read(fbs->numRootEntries), 512) / 512;

    // total blocks for FAT16 or FAT32
    totalSectors = le16read(fbs->totalSectors16);
    if (totalSectors == 0) {
        totalSectors = le32read(fbs->totalSectors32);
    }

    // total data blocks
    fatInfo.clusterCount = totalSectors - (fatInfo.dataStartBlock - volumeStartBlock);
    // divide by cluster size to get cluster count
    fatInfo.clusterCount >>= (fatInfo.bytesPerClusterShift - 9);

    // FAT type is determined by cluster count
    if (fatInfo.clusterCount < 4085) {
        DPRINTF("FAT12 not supported!\n");
        goto fail;
    } else if (fatInfo.clusterCount < 65525) {
        // all ok - FAT16 supported
    } else {
        DPRINTF("FAT32 not supported!\n");
        goto fail;
    }

    printFatInfo(&fatInfo);

    return true;

  fail:
    DPRINTF("init part fail\n");
    errno = ENXIO;
    return false;
}

void fatFsFileRemove(const char *filename)
{
    uint16_t directoryEntry;
    DirectoryEntry_t *de = fatFsFileSearch(filename, &directoryEntry);
    if (de) {
        de->filename[0] = FILENAME_DELETED;
        cacheDirty = true;
        cacheFlush();
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

static inline bool fatNameGet(DirectoryEntry_t *entry, char *buffer)
{
    if (entry->filename[0] == FILENAME_UNUSED
            || entry->filename[0] == FILENAME_DELETED) {
        return false;
    }
    memcpy(buffer, entry->filename, 8);
    if (entry->filename[0] == FILENAME_E5) {
        buffer[0] = 0xE5;
    }
    buffer[8] = '.';
    memcpy(buffer + 9, entry->extension, 3);
    return true;
}

static inline bool fatNameMatch(DirectoryEntry_t *entry,
                                uint8_t filename[8], uint8_t extension[3])
{
    return !memcmp(filename, entry->filename, 8)
            && !memcmp(extension, entry->extension, 3);
}

static inline bool isFatCharacterAcceptable(char c)
{
    if (isalnum(c)) return true;
    if (c == ' ') return true;
    if (c >= 128) return true;
    if (c == '!' || c == '#'
            || c == '$' || c == '%'
            || c == '&' || c == '\''
            || c == '(' || c == ')'
            || c == '-' || c == '@'
            || c == '^' || c == '_'
            || c == '`' || c == '{'
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
                if (!isFatCharacterAcceptable(name[i + j])) return false;
            }
            i += j;
            break;
        }
        if (!isFatCharacterAcceptable(name[i])) return false;
    }

    // too long?
    return name[i] == 0;
}

DirectoryEntry_t *fatFsFileSearch(const char *__restrict name,
                                  uint16_t *__restrict entryIndex /* out */)
{
    uint16_t i;
    sector_t numRootSectors = fatInfo.numRootEntries * 32 / SDCARD_SECTOR_SIZE;

    uint8_t filename[9] = {0};
    uint8_t extension[4] = {0};
    if (!fatNameSet(name, filename, extension)) {
        goto fail;
    }

    *entryIndex = 0;
    for (i = 0; i < numRootSectors; ++i) {
        if (!cacheRawBlock(fatInfo.rootDirStart + i, false)) {
            goto fail;
        }
        uint16_t j;
        for (j = 0; j < SDCARD_SECTOR_SIZE / 32; ++j) {
            DirectoryEntry_t *e = &cache.entries[j];
            DPRINTF("check entry %u\n", j);
            printFatEntry(e);
            if (fatNameMatch(e, filename, extension)) {
                DPRINTF("fatFsFileOpen: matched %s!\n", name);
                return e;
            }
            (*entryIndex)++;
        }
        break;
    }
    DPRINTF("fatFsFileOpen: %s not found!\n", name);

  fail:
    return NULL;
}

uint16_t fatFsGetFiles(char *p, uint16_t bufferSize)
{
    const char *end = p + bufferSize;
    uint16_t i;
    sector_t numRootSectors = fatInfo.numRootEntries * 32 / SDCARD_SECTOR_SIZE;
    uint16_t length = 0;

    if (bufferSize == 0) goto fail;
    for (i = 0; i < numRootSectors; ++i) {
        if (!cacheRawBlock(fatInfo.rootDirStart + i, false)) {
            goto fail;
        }
        uint16_t j;
        for (j = 0; j < SDCARD_SECTOR_SIZE / 32; ++j) {
            DirectoryEntry_t *e = &cache.entries[j];
            if (end - p >= 13) {
                if (fatNameGet(e, p)) {
                    p += 12;
                    *p++ = '\n';
                    length += 13;
                }
            }
        }
        break;
    }
    *(p - 1) = '\0';

  fail:
    return length;
}

DirectoryEntry_t *fatFsFileCreate(const char *__restrict name,
                                  uint16_t *__restrict entryIndex /* out */)
{
    DPRINTF("fatFsFileCreate... %s\n", name);

    uint16_t i;
    sector_t numRootSectors = fatInfo.numRootEntries * 32 / SDCARD_SECTOR_SIZE;

    uint8_t filename[9] = {0};
    uint8_t extension[4] = {0};
    if (!fatNameSet(name, filename, extension)) {
        goto fail;
    }

    *entryIndex = 0;

    for (i = 0; i < numRootSectors; ++i) {
        if (!cacheRawBlock(fatInfo.rootDirStart + i, false)) {
            DPRINTF("cacheRawBlock failed\n");
            goto fail;
        }
        uint16_t j;
        for (j = 0; j < SDCARD_SECTOR_SIZE / 32; ++j) {
            DirectoryEntry_t *entry = &cache.entries[j];
            if (entry->filename[0] == FILENAME_UNUSED
                    || entry->filename[0] == FILENAME_DELETED) {
                memcpy(entry->filename, filename, 8);
                memcpy(entry->extension, extension, 3);
                memset(entry + 11, 0, sizeof(*entry) - 11);
                cacheDirty = true;
                return entry;
            }
            (*entryIndex)++;
        }
    }

  fail:
    return NULL;
}

static uint8_t fatGet(fat_t cluster, fat_t* value) {
    if (cluster > (fatInfo.clusterCount + 1)) return false;
    uint32_t lba = fatInfo.fatStartBlock + (cluster >> 8);
    if (lba != cacheBlockNumber) {
        if (!cacheRawBlock(lba, false)) return false;
    }
    *value = cache.fat[cluster & 0XFF];
    return true;
}

static bool fatPut(fat_t cluster, fat_t value) {
    DPRINTF("fatPut %u: %u\n", cluster, value);
    if (cluster < INITIAL_CLUSTER) return false;
    if (cluster > (fatInfo.clusterCount + 1)) return false;
    uint32_t lba = fatInfo.fatStartBlock + (cluster >> 8);
    if (lba != cacheBlockNumber) {
        if (!cacheRawBlock(lba, false)) return false;
    }
    cache.fat[cluster & 0XFF] = value;
    cacheDirty = true;
    // mirror second FAT
    if (fatInfo.numFATs > 1) cacheMirrorBlock = lba + fatInfo.sectorsPerFAT;
    return true;
}

static bool fatFsAllocateCluster(FILE *handle) {
    DPRINTF("fatFsAllocateCluster\n");

    fat_t freeCluster = handle->currentCluster ? handle->currentCluster : 1;
    uint16_t i;
    for (i = 0; ; i++) {
        if (i >= fatInfo.clusterCount) {
            DPRINTF("no free clusters (%lu total)\n", fatInfo.clusterCount);
            return false;
        }

        if (freeCluster > fatInfo.clusterCount) freeCluster = 1;
        freeCluster++;

        fat_t value;
        if (!fatGet(freeCluster, &value)) {
            DPRINTF("fatGet failed\n");
            return false;
        }
        if (value == 0) break;
    }

    if (!fatPut(freeCluster, FAT16_END)) {
        DPRINTF("fatPut (1) failed\n");
        return false;
    }

    if (handle->currentCluster != 0) {
        if (!fatPut(handle->currentCluster, freeCluster)) {
            DPRINTF("fatPut (2) failed\n");
            return false;
        }
    } else {
        handle->dirEntryDirty = true;
        handle->firstCluster = freeCluster;
    }
    handle->currentCluster = freeCluster;
    return true;
}

static DirectoryEntry_t* cacheDirEntry(uint16_t index, bool makeDirty)
{
    DPRINTF("cacheDirEntry, index=%u\n", index);
    if (index >= fatInfo.numRootEntries) return NULL;
    if (!cacheRawBlock(fatInfo.rootDirStart + (index >> 4), makeDirty)) {
        return NULL;
    }
    return &cache.entries[index & 0xF];
}

void fatFsFileFlush(FILE *handle)
{
    if (handle->dirEntryDirty) {
        // cache directory entry
        DirectoryEntry_t* d = cacheDirEntry(handle->directoryEntry, true);
        if (!d) return;

        // update file size and first cluster
        d->fileSize = handle->fileSize;
        d->startClusterLoword = handle->firstCluster;

        // TODO: set modification time

        handle->dirEntryDirty = false;
    }
    cacheFlush();
}

void fatFsFileClose(FILE *handle)
{
    fatFsFileFlush(handle);
}

uint16_t fatFsRead(FILE *handle, void *buffer, uint16_t maxLength)
{
    if (!(handle->flags = O_RDWR))  {
        // TODO: report error
        return 0;
    }

    // if the file is empty, return 0
    if (handle->currentCluster == 0) return 0;
    // if end of file is reached, also return 0
    // if (handle->currentCluster >= FAT16_END_MIN) return 0;
    if (handle->position == handle->fileSize) return 0;

    uint16_t offsetInCluster = handle->position & fatInfo.bytesPerClusterMask;
    uint16_t offsetInBlock = offsetInCluster & (SDCARD_SECTOR_SIZE - 1);
    uint16_t blockNumber = fatInfo.dataStartBlock + clusterToBlock(handle->currentCluster);
    blockNumber += offsetInCluster / SDCARD_SECTOR_SIZE;

    // check the file size and do not allow to read too much
    if (handle->position + maxLength > handle->fileSize) {
        maxLength = handle->fileSize - handle->position;
    }

    bool useTwoClusters;
    if ((offsetInCluster + maxLength) >= (1 << fatInfo.bytesPerClusterShift)) {
        // will use two clusters
        useTwoClusters = true;
    } else {
        // will use just one cluster
        useTwoClusters = false;
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
    memcpy(buffer, cache.data + offsetInBlock, maxLength);
    handle->position += maxLength;

    if (maxLength2) {
        if (useTwoClusters) {
            // set handle->currentCluster to the next cluster
            fatGet(handle->currentCluster, &handle->currentCluster);
            // set to the first block in the next cluster
            blockNumber = fatInfo.dataStartBlock + clusterToBlock(handle->currentCluster);
        } else {
            // simply advance the block number by one
            blockNumber += 1;
        }
        if (!cacheRawBlock(blockNumber, false)) {
            DPRINTF("fatFsRead: raw read (2) failed\n");
            return 0;
        }
        memcpy(buffer + maxLength, cache.data, maxLength2);
        handle->position += maxLength2;
    }

    return maxLength + maxLength2;
}

uint16_t fatFsWrite(FILE *handle, const void *buffer, uint16_t length)
{
    if (!(handle->flags & O_WRITE))  {
        // TODO: report error
        return 0;
    }

    if ((handle->flags & O_APPEND) && handle->position != handle->fileSize) {
        // handle append (move to the end of the file)
        if (!fatfsGoToPosition(handle, handle->fileSize)) return 0;
    }

    // if currentCluster is 0, allocate space on disk!
    if (handle->currentCluster == 0) {
        if (!fatFsAllocateCluster(handle)) return 0;
    }
    DPRINTF("  currentCluster=%u\n", handle->currentCluster);

    uint16_t offsetInCluster = handle->position & fatInfo.bytesPerClusterMask;
    uint16_t offsetInBlock = offsetInCluster & (SDCARD_SECTOR_SIZE - 1);
    uint16_t blockNumber = fatInfo.dataStartBlock + clusterToBlock(handle->currentCluster);
    blockNumber += offsetInCluster / SDCARD_SECTOR_SIZE;

    uint16_t beforeEnd = handle->fileSize - handle->position;
    if (beforeEnd < length) {
        // the file is going to grow
        handle->fileSize += length - beforeEnd;
    }
    handle->position += length;
    handle->dirEntryDirty = true;
    DPRINTF("  pos=%lu fileSize=%lu off-in-block=%u\n",
            handle->position, handle->fileSize, offsetInBlock);

    bool useTwoClusters;
    if ((offsetInCluster + length) >= (1 << fatInfo.bytesPerClusterShift)) {
        // will use two clusters
        useTwoClusters = true;
    } else {
        // will use just one cluster
        useTwoClusters = false;
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
        DPRINTF("fatFsWrite: raw write failed\n");
        return 0;
    }
    memcpy(cache.data + offsetInBlock, buffer, length);

    if (length2) {
        if (useTwoClusters) {
            fat_t nextCluster;
            fatGet(handle->currentCluster, &nextCluster);
            if (nextCluster >= FAT16_END_MIN) {
                // end reached; allocate new cluster
                DPRINTF("!!! alloc new cluster\n");
                if (!fatFsAllocateCluster(handle)) {
                    return length; // 'length' bytes were already written
                }
            }
            else {
                // next cluster is already allocated to the file; use it
                handle->currentCluster = nextCluster;
                DPRINTF("use existing next cluster\n");
            }
            // set to the first block in the next cluster
            blockNumber = fatInfo.dataStartBlock + clusterToBlock(handle->currentCluster);
        } else {
            // simply advance the block number by one
            blockNumber += 1;
        }
        if (!cacheRawBlock(blockNumber, true)) {
            DPRINTF("fatFsWrite: raw write (2) failed\n");
            return 0;
        }
        memcpy(cache.data, buffer + length, length2);
    }
    else if (useTwoClusters) {
        // next read will be exactly at the start of the next cluster
        // fatGet(handle->currentCluster, &handle->currentCluster);
        fat_t nextCluster;
        fatGet(handle->currentCluster, &nextCluster);
        if (nextCluster >= FAT16_END_MIN) {
            // end reached; allocate new cluster
            DPRINTF("!!! alloc new cluster\n");
            fatFsAllocateCluster(handle);
        }
        else {
            // next cluster is already allocated to the file; use it
            handle->currentCluster = nextCluster;
            DPRINTF("use existing next cluster\n");
        }
    }

    // flush it (also writes changes to dir entry table)
    fatFsFileFlush(handle);

    return length + length2;
}

bool fatfsGoToPosition(FILE *handle, uint32_t newPosition)
{
    cluster_t clusterIndex = newPosition >> fatInfo.bytesPerClusterShift;
    fat_t nextCluster = handle->firstCluster;
    while (clusterIndex) {
        if (!fatGet(nextCluster, &nextCluster)) return false;
        clusterIndex--;
    }
    handle->currentCluster = nextCluster;
    handle->position = newPosition;
    return true;
}
