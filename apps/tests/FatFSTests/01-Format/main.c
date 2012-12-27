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

#include "stdmansos.h"
#include <lib/byteorder.h>
#include <lib/assert.h>
#include <sdcard/sdcard.h>
#include <fatfs/structures.h>

//
// FAT16 formatting application.
// If DO_ERASE is true, filesystem image is erased (all set to zeros).
// If DO_FORMAT is true, FAT16 filesystem is cear
// If PARTITION_TABLE is true, filesystem is created in partition 1
// (with 1MB offset); otherwise whole disk is used.
//
// For quick format comment out DO_ERASE;
// for erasing without formatting comment out DO_FORMAT.
//
// The formatting is modelled after Linux 3.2 command line utilities
// "fdisk" and "mkdosfs -F 16" with the default values.
//

#define DO_ERASE 1
#define DO_FORMAT 1

#define PARTITION_TABLE 1

#define LINUX_PARTITION  0x83

//
// SD-card dependent constants.
// The values below are suitable for 2GB card.
//
#define BLOCK_SIZE                 512
#define PARTITION_OFFSET_BLOCKS    2048ul
#define PARTITION_OFFSET           (PARTITION_OFFSET_BLOCKS * BLOCK_SIZE)
#define TOTAL_SIZE                 (2ul * 1024 * 1024 * 1024)
#define TOTAL_BLOCKS               (TOTAL_SIZE / BLOCK_SIZE)

#define BLOCKS_PER_CLUSTER         64

#define FAT_COUNT                  2
#define ROOT_DIR_ENTRY_COUNT       2048
#define BLOCKS_PER_FAT             256
#define SECTORS_PER_CLUSTER        64 // depends on media size

#if PARTITION_TABLE
#define FAT1_OFFSET                ((PARTITION_OFFSET_BLOCKS + BLOCKS_PER_CLUSTER) * BLOCK_SIZE)
#define FAT2_OFFSET                ((PARTITION_OFFSET_BLOCKS + BLOCKS_PER_CLUSTER + BLOCKS_PER_FAT) * BLOCK_SIZE)
#else
#define FAT1_OFFSET                (BLOCKS_PER_CLUSTER * BLOCK_SIZE)
#define FAT2_OFFSET                ((BLOCKS_PER_CLUSTER +  BLOCKS_PER_FAT) * BLOCK_SIZE)
#endif

static uint8_t buffer[SDCARD_SECTOR_SIZE];

int fatFsFormat(void)
{
    MasterBootRecord_t *mbr;
    FatBootBlock_t *fbs;
    int ret = -1;
    
    redLedOn();

#if DO_ERASE
    sdcardBulkErase();
#endif // DO_ERASE

#if DO_FORMAT
    if (!sdcardReadBlock(0, buffer)) goto fail;

    // TODO: allow to use superblock only!

    mbr = (MasterBootRecord_t *) buffer;
    mbr->part[0].boot = 0;
    mbr->part[0].beginHead = 0;
    mbr->part[0].beginSector = 0;
    mbr->part[0].beginCylinderHigh = 0;
    mbr->part[0].beginCylinderLow = 0;
    mbr->part[0].type = LINUX_PARTITION;
    mbr->part[0].endHead = 0;
    mbr->part[0].endSector = 0;
    mbr->part[0].endCylinderHigh = 0;
    mbr->part[0].endCylinderLow = 0;
    mbr->part[0].firstSector = PARTITION_OFFSET_BLOCKS;
    mbr->part[0].totalSectors = TOTAL_BLOCKS - PARTITION_OFFSET_BLOCKS;
    mbr->mbrSig0 = 0x55;
    mbr->mbrSig1 = 0xAA;

    if (!sdcardWriteBlock(0, buffer)) goto fail;

    
    if (!sdcardReadBlock(PARTITION_OFFSET, buffer)) goto fail;

    fbs = (FatBootBlock_t *) buffer;
    // put jump insrtuction the the first three bytes
    fbs->bootstrapProg[0] = 0xEB;
    fbs->bootstrapProg[1] = 0x3C;
    fbs->bootstrapProg[2] = 0x90;
    // put "mansos" as the formatter string
    memcpy(fbs->oemDescription, "mansos\0\0", 8);
    // put constants
    le16write(fbs->bytesPerSector, BLOCK_SIZE);
    fbs->sectorsPerCluster = SECTORS_PER_CLUSTER;
    le16write(fbs->numReservedSectors, 64);
    fbs->numFATs = FAT_COUNT;
    le16write(fbs->numRootEntries, ROOT_DIR_ENTRY_COUNT);
    if (TOTAL_BLOCKS < 0x10000) {
        le16write(fbs->totalSectors16, (uint16_t) TOTAL_BLOCKS);
    } else {
        le16write(fbs->totalSectors16, 0); // set to zero and use totalSectors32 instead
    }
    fbs->mediaDescriptor = 0xf8;
    le16write(fbs->sectorsPerFAT16, BLOCKS_PER_FAT);
    le16write(fbs->sectorsPerTrack, 0);
    le16write(fbs->numHeads, 0);
    le32write(fbs->numHiddenBlocks, 0ul);
    le32write(fbs->totalSectors32, TOTAL_BLOCKS);
    le16write(fbs->physicalDriveNumber, 0);
//    fbs->reserved1 = 0;
    fbs->bootRecordSignature = 0x29; // valid serial number, volume label and FS type
    le32write(fbs->volumeSerialNumber, ((uint32_t)randomNumber() << 16) + randomNumber());
    memcpy(fbs->volumeLabel, "         ", 11);
    memcpy(fbs->fileSystemIdentifier, "FAT16   ", 8);
    fbs->bootSectorSig0 = 0x55;
    fbs->bootSectorSig1 = 0xAA;

    if (!sdcardWriteBlock(PARTITION_OFFSET, buffer)) goto fail;

    // FAT #1
    if (!sdcardReadBlock(FAT1_OFFSET, buffer)) goto fail;
    buffer[0] = 0xf8;
    buffer[1] = 0xff;
    buffer[2] = 0xff;
    buffer[3] = 0xff;
    if (!sdcardWriteBlock(FAT1_OFFSET, buffer)) goto fail;

    // FAT #2
    if (!sdcardReadBlock(FAT2_OFFSET, buffer)) goto fail;
    buffer[0] = 0xf8;
    buffer[1] = 0xff;
    buffer[2] = 0xff;
    buffer[3] = 0xff;
    if (!sdcardWriteBlock(FAT2_OFFSET, buffer)) goto fail;

#endif // DO_FORMAT

    ret = 0; // success!

  fail:
    redLedOff();
    return ret;
}

void appMain(void)
{
    COMPILE_TIME_ASSERT(BLOCK_SIZE == SDCARD_SECTOR_SIZE, sizecheck);

    int result = fatFsFormat();
    ASSERT(result == 0);

    PRINTF("done!\n");
}
