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

#ifndef MANSOS_FATFS_STRUCTURES_H
#define MANSOS_FATFS_STRUCTURES_H

#include <kernel/defines.h>

// File-system sector and cluster types
typedef uint16_t cluster_t;
#if FAT32_SUPPORT
typedef uint32_t sector_t;
#else
typedef uint16_t sector_t;
#endif

struct FatBootBlock_s {
    uint8_t bootstrapProg[3];
    uint8_t oemDescription[8];
    uint8_t bytesPerSector[2]; // only 512 supported
    uint8_t sectorsPerCluster; // 1 to 128
    uint8_t numReservedSectors[2];
    uint8_t numFATs;  // FAT count
    // root directory entry count, including unused ones. zero for FAT32
    uint8_t numRootEntries[2];
    // total blocks in disk. set to zero if > 65535
    uint8_t totalSectors16[2];
    uint8_t mediaDescriptor; // unused
    uint8_t sectorsPerFAT16[2];

    uint8_t sectorsPerTrack[2]; // for "bootstrap prog"
    uint8_t numHeads[2]; // for "bootstrap prog"
    uint8_t numHiddenBlocks[4]; // unused
    // total blocks in disk. same as totalSectors16, if <= 65535
    uint8_t totalSectors32[4];
    uint8_t physicalDriveNumber[2]; // unused
    uint8_t bootRecordSignature; // unsued
    uint8_t volumeSerialNumber[4]; // unique ID for this disk
    uint8_t volumeLabel[11]; // human-readable name
    uint8_t fileSystemIdentifier[8]; // FAT16, FAT12, FAT32, ...
    uint8_t bootCode[448];
    uint8_t bootSectorSig0;  // always 0x55
    uint8_t bootSectorSig1;  // always 0xAA
} PACKED;
typedef struct FatBootBlock_s FatBootBlock_t;


// value of bytes 510-511 for MBR
#define BOOT_BLOCK_SIGNATURE_1 0x55
#define BOOT_BLOCK_SIGNATURE_2 0xAA

// for FAT/FAT32 boot setcor
#define EXTENDED_BOOT_SIGNATURE  0x29

// historical value for hard disks
#define MEDIA_DESCRIPTOR 0xF0


/**
 * \struct partitionTable
 * \brief MBR partition table entry
 *
 * A partition table entry for a MBR formatted storage device.
 * The MBR partition table has four entries.
 */
struct PartitionTable_s {
          /**
           * Boot Indicator . Indicates whether the volume is the active
           * partition.  Legal values include: 0X00. Do not use for booting.
           * 0X80 Active partition.
           */
    uint8_t  boot;
          /**
            * Head part of Cylinder-head-sector address of the first block in
            * the partition. Legal values are 0-255. Only used in old PC BIOS.
            */
    uint8_t  beginHead;
          /**
           * Sector part of Cylinder-head-sector address of the first block in
           * the partition. Legal values are 1-63. Only used in old PC BIOS.
           */
    unsigned beginSector : 6;
           /** High bits cylinder for first block in partition. */
    unsigned beginCylinderHigh : 2;
          /**
           * Combine beginCylinderLow with beginCylinderHigh. Legal values
           * are 0-1023.  Only used in old PC BIOS.
           */
    uint8_t  beginCylinderLow;
          /**
           * Partition type. See defines that begin with PART_TYPE_ for
           * some Microsoft partition types.
           */
    uint8_t  type;
          /**
           * head part of cylinder-head-sector address of the last sector in the
           * partition.  Legal values are 0-255. Only used in old PC BIOS.
           */
    uint8_t  endHead;
          /**
           * Sector part of cylinder-head-sector address of the last sector in
           * the partition.  Legal values are 1-63. Only used in old PC BIOS.
           */
    unsigned endSector : 6;
           /** High bits of end cylinder */
    unsigned endCylinderHigh : 2;
          /**
           * Combine endCylinderLow with endCylinderHigh. Legal values
           * are 0-1023.  Only used in old PC BIOS.
           */
    uint8_t  endCylinderLow;
           /** Logical block address of the first block in the partition. */
    uint32_t firstSector;
           /** Length of the partition, in blocks. */
    uint32_t totalSectors;
} PACKED;
/** Type name for PartitionTable */
typedef struct PartitionTable_s PartitionTable_t;


/**
 * \struct masterBootRecord
 *
 * \brief Master Boot Record
 *
 * The first block of a storage device that is formatted with a MBR.
 */
struct MasterBootRecord_s {
           /** Code Area for master boot program. */
  uint8_t  codeArea[440];
           /** Optional Windows NT disk signature. May contain boot code. */
  uint32_t diskSignature;
           /** Usually zero but may be more boot code. */
  uint16_t usuallyZero;
           /** Partition tables. */
  PartitionTable_t   part[4];
           /** First MBR signature byte. Must be 0X55 */
  uint8_t  mbrSig0;
           /** Second MBR signature byte. Must be 0XAA */
  uint8_t  mbrSig1;
} PACKED;
/** Type name for masterBootRecord */
typedef struct MasterBootRecord_s MasterBootRecord_t;


struct DirectoryEntry_s {
    uint8_t filename[8];
    uint8_t extension[3];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t timeCreatedSplitSeconds;
    uint16_t timeCreated;
    uint16_t dateCreated;
    uint16_t dateAccessed;
    uint16_t startClusterHiword;
    uint16_t timeWritten;
    uint16_t dateWritten;
    uint16_t startClusterLoword;
    uint32_t fileSize;
} PACKED;
typedef struct DirectoryEntry_s DirectoryEntry_t;

typedef uint16_t fat_t;

typedef union Cache16_u {
          /** Used to access cached file data blocks. */
    uint8_t data[512];
          /** Used to access cached FAT entries. */
    fat_t   fat[256];
          /** Used to access cached directory entries. */
    DirectoryEntry_t entries[16];
          /** Used to access a cached Master Boot Record. */
    MasterBootRecord_t  mbr;
          /** Used to access to a cached FAT16 boot sector. */
    FatBootBlock_t fbs;
} Cache16_t;


// constants used in filename entries
#define FILENAME_UNUSED    0x00
#define FILENAME_DELETED   0xE5
#define FILENAME_E5        0x05
#define FILENAME_DIRECTORY 0x2E

// File attributes (unused; no need for any at the moment)
#define FILE_ATTR_READ_ONLY         1 << 0
#define FILE_ATTR_HIDDEN            1 << 1
#define FILE_ATTR_SYSTEM            1 << 2
#define FILE_ATTR_DISK_VOLUME_LABEL 1 << 3
#define FILE_ATTR_SUBDIRECTORY      1 << 4
#define FILE_ATTR_ARCHIVED          1 << 5


#define INITIAL_CLUSTER 2

#define FAT16_END     0xFFFF
#define FAT16_END_MIN 0xFFF8

typedef struct FatInfo_s {
    uint16_t sectorsPerFAT;       // FAT size in sectors
    uint32_t clusterCount;        // clusters in one FAT

    uint8_t bytesPerClusterShift; // by shifting with this, can convert between bytes and clusters
    uint16_t bytesPerClusterMask; // by masking out with this, can get the offset in cluster

    uint32_t dataStartBlock;     // first data block number
    uint8_t numFATs;             // number of FATs on volume
    uint32_t fatStartBlock;      // start block for first FAT
    uint16_t numRootEntries;     // number of entries in FAT16 root dir
    uint32_t rootDirStart;       // root start block for FAT16, cluster for FAT32
} FatInfo_t;

extern FatInfo_t fatInfo;

static inline uint32_t clusterToBytes(cluster_t cluster) {
    return (uint32_t) cluster << fatInfo.bytesPerClusterShift;
}

static inline sector_t clusterToBlock(cluster_t cluster) {
    cluster -= 2; // cluster numbering starts from 2
    return (sector_t) cluster << (fatInfo.bytesPerClusterShift - 9);
}

#endif
