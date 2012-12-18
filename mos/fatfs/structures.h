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

#ifndef MANSOS_FATFS_STRUCTURES_H
#define MANSOS_FATFS_STRUCTURES_H

#include <kernel/defines.h>

// struct FatBootBlock_s {
//     uint8_t bootstrapProg[3]; // unused
//     uint8_t oemDescription[8]; // unused
//     uint8_t bytesPerBlock[2]; // only 512 supported
//     uint8_t blocksPerAllocationUnit; // usually 1
//     uint8_t numReservedBlocks[2]; // usually 1
//     uint8_t numFATs;  // FAT count
//     // root directory entry count, including unused ones. zero for FAT32
//     uint8_t numRootEntries[2];
//     // total blocks in disk. set to zero if > 65535
//     uint8_t numTotalBlocks16[2];
//     uint8_t mediaDescriptor; // unused
//     uint8_t blocksPerFAT[2]; // usually 1
//     uint8_t blocksPerTrack[2]; // for "bootstrap prog"
//     uint8_t numHeads[2]; // for "bootstrap prog"
//     uint8_t numHiddenBlocks[4]; // unused
//     // total blocks in disk. same as numTotalBlocks16, if <= 65535
//     uint8_t numTotalBlocks32[4];
//     uint8_t physicalDriveNumber[2]; // unused
//     uint8_t bootRecordSignature; // unsued
//     uint8_t volumeSerialNumber[4]; // unique ID for this disk
//     uint8_t volumeLabel[11]; // human-readable name
//     uint8_t fileSystemIdentifier[8]; // FAT16, FAT12, FAT32, ...
// } PACKED;
// typedef struct FatBootBlock_s FatBootBlock_t;

/**
 * \struct fat_boot
 *
 * \brief Boot sector for a FAT12/FAT16 volume.
 *
 */
struct fat_boot_s {
         /**
          * The first three bytes of the boot sector must be valid,
          * executable x 86-based CPU instructions. This includes a
          * jump instruction that skips the next nonexecutable bytes.
          */
  uint8_t jump[3];
         /**
          * This is typically a string of characters that identifies
          * the operating system that formatted the volume.
          */
  char    oemId[8];
          /**
           * The size of a hardware sector. Valid decimal values for this
           * field are 512, 1024, 2048, and 4096. For most disks used in
           * the United States, the value of this field is 512.
           */
  uint16_t bytesPerSector;
          /**
           * Number of sectors per allocation unit. This value must be a
           * power of 2 that is greater than 0. The legal values are
           * 1, 2, 4, 8, 16, 32, 64, and 128.  128 should be avoided.
           */
  uint8_t  sectorsPerCluster;
          /**
           * The number of sectors preceding the start of the first FAT,
           * including the boot sector. The value of this field is always 1.
           */
  uint16_t reservedSectorCount;
          /**
           * The number of copies of the FAT on the volume.
           * The value of this field is always 2.
           */
  uint8_t  fatCount;
          /**
           * For FAT12 and FAT16 volumes, this field contains the count of
           * 32-byte directory entries in the root directory. For FAT32 volumes,
           * this field must be set to 0. For FAT12 and FAT16 volumes, this
           * value should always specify a count that when multiplied by 32
           * results in a multiple of bytesPerSector.  FAT16 volumes should
           * use the value 512.
           */
  uint16_t rootDirEntryCount;
          /**
           * This field is the old 16-bit total count of sectors on the volume.
           * This count includes the count of all sectors in all four regions
           * of the volume. This field can be 0; if it is 0, then totalSectors32
           * must be nonzero.  For FAT32 volumes, this field must be 0. For
           * FAT12 and FAT16 volumes, this field contains the sector count, and
           * totalSectors32 is 0 if the total sector count fits
           * (is less than 0x10000).
           */
  uint16_t totalSectors16;
          /**
           * This dates back to the old MS-DOS 1.x media determination and is
           * no longer usually used for anything.  0xF8 is the standard value
           * for fixed (nonremovable) media. For removable media, 0xF0 is
           * frequently used. Legal values are 0xF0 or 0xF8-0xFF.
           */
  uint8_t  mediaType;
          /**
           * Count of sectors occupied by one FAT on FAT12/FAT16 volumes.
           * On FAT32 volumes this field must be 0, and sectorsPerFat32
           * contains the FAT size count.
           */
  uint16_t sectorsPerFat16;
           /** Sectors per track for interrupt 0x13. Not used otherwise. */
  uint16_t sectorsPerTrack;
           /** Number of heads for interrupt 0x13.  Not used otherwise. */
  uint16_t headCount;
          /**
           * Count of hidden sectors preceding the partition that contains this
           * FAT volume. This field is generally only relevant for media
           * visible on interrupt 0x13.
           */
  uint32_t hidddenSectors;
          /**
           * This field is the new 32-bit total count of sectors on the volume.
           * This count includes the count of all sectors in all four regions
           * of the volume.  This field can be 0; if it is 0, then
           * totalSectors16 must be nonzero.
           */
  uint32_t totalSectors32;
           /**
            * Related to the BIOS physical drive number. Floppy drives are
            * identified as 0x00 and physical hard disks are identified as
            * 0x80, regardless of the number of physical disk drives.
            * Typically, this value is set prior to issuing an INT 13h BIOS
            * call to specify the device to access. The value is only
            * relevant if the device is a boot device.
            */
  uint8_t  driveNumber;
           /** used by Windows NT - should be zero for FAT */
  uint8_t  reserved1;
           /** 0X29 if next three fields are valid */
  uint8_t  bootSignature;
           /**
            * A random serial number created when formatting a disk,
            * which helps to distinguish between disks.
            * Usually generated by combining date and time.
            */
  uint32_t volumeSerialNumber;
           /**
            * A field once used to store the volume label. The volume label
            * is now stored as a special file in the root directory.
            */
  char     volumeLabel[11];
           /**
            * A field with a value of either FAT, FAT12 or FAT16,
            * depending on the disk format.
            */
  char     fileSystemType[8];
           /** X86 boot code */
  uint8_t  bootCode[448];
           /** must be 0X55 */
  uint8_t  bootSectorSig0;
           /** must be 0XAA */
  uint8_t  bootSectorSig1;
} PACKED;
/** Type name for FAT Boot Sector */
typedef struct fat_boot_s FatBootBlock_t;
//------------------------------------------------------------------------------
/**
 * \struct fat32_boot
 *
 * \brief Boot sector for a FAT32 volume.
 *
 */
struct fat32_boot_s {
         /**
          * The first three bytes of the boot sector must be valid,
          * executable x 86-based CPU instructions. This includes a
          * jump instruction that skips the next nonexecutable bytes.
          */
  uint8_t jump[3];
         /**
          * This is typically a string of characters that identifies
          * the operating system that formatted the volume.
          */
  char    oemId[8];
          /**
           * The size of a hardware sector. Valid decimal values for this
           * field are 512, 1024, 2048, and 4096. For most disks used in
           * the United States, the value of this field is 512.
           */
  uint16_t bytesPerSector;
          /**
           * Number of sectors per allocation unit. This value must be a
           * power of 2 that is greater than 0. The legal values are
           * 1, 2, 4, 8, 16, 32, 64, and 128.  128 should be avoided.
           */
  uint8_t  sectorsPerCluster;
          /**
           * The number of sectors preceding the start of the first FAT,
           * including the boot sector. Must not be zero
           */
  uint16_t reservedSectorCount;
          /**
           * The number of copies of the FAT on the volume.
           * The value of this field is always 2.
           */
  uint8_t  fatCount;
          /**
           * FAT12/FAT16 only. For FAT32 volumes, this field must be set to 0.
           */
  uint16_t rootDirEntryCount;
          /**
           * For FAT32 volumes, this field must be 0.
           */
  uint16_t totalSectors16;
          /**
           * This dates back to the old MS-DOS 1.x media determination and is
           * no longer usually used for anything.  0xF8 is the standard value
           * for fixed (nonremovable) media. For removable media, 0xF0 is
           * frequently used. Legal values are 0xF0 or 0xF8-0xFF.
           */
  uint8_t  mediaType;
          /**
           * On FAT32 volumes this field must be 0, and sectorsPerFat32
           * contains the FAT size count.
           */
  uint16_t sectorsPerFat16;
           /** Sectors per track for interrupt 0x13. Not used otherwise. */
  uint16_t sectorsPerTrack;
           /** Number of heads for interrupt 0x13.  Not used otherwise. */
  uint16_t headCount;
          /**
           * Count of hidden sectors preceding the partition that contains this
           * FAT volume. This field is generally only relevant for media
           * visible on interrupt 0x13.
           */
  uint32_t hidddenSectors;
          /**
           * Contains the total number of sectors in the FAT32 volume.
           */
  uint32_t totalSectors32;
         /**
           * Count of sectors occupied by one FAT on FAT32 volumes.
           */
  uint32_t sectorsPerFat32;
          /**
           * This field is only defined for FAT32 media and does not exist on
           * FAT12 and FAT16 media.
           * Bits 0-3 -- Zero-based number of active FAT.
           *             Only valid if mirroring is disabled.
           * Bits 4-6 -- Reserved.
           * Bit 7  -- 0 means the FAT is mirrored at runtime into all FATs.
           *        -- 1 means only one FAT is active; it is the one referenced
           *             in bits 0-3.
           * Bits 8-15 -- Reserved.
           */
  uint16_t fat32Flags;
          /**
           * FAT32 version. High byte is major revision number.
           * Low byte is minor revision number. Only 0.0 define.
           */
  uint16_t fat32Version;
          /**
           * Cluster number of the first cluster of the root directory for FAT32.
           * This usually 2 but not required to be 2.
           */
  uint32_t fat32RootCluster;
          /**
           * Sector number of FSINFO structure in the reserved area of the
           * FAT32 volume. Usually 1.
           */
  uint16_t fat32FSInfo;
          /**
           * If nonzero, indicates the sector number in the reserved area
           * of the volume of a copy of the boot record. Usually 6.
           * No value other than 6 is recommended.
           */
  uint16_t fat32BackBootBlock;
          /**
           * Reserved for future expansion. Code that formats FAT32 volumes
           * should always set all of the bytes of this field to 0.
           */
  uint8_t  fat32Reserved[12];
           /**
            * Related to the BIOS physical drive number. Floppy drives are
            * identified as 0x00 and physical hard disks are identified as
            * 0x80, regardless of the number of physical disk drives.
            * Typically, this value is set prior to issuing an INT 13h BIOS
            * call to specify the device to access. The value is only
            * relevant if the device is a boot device.
            */
  uint8_t  driveNumber;
           /** used by Windows NT - should be zero for FAT */
  uint8_t  reserved1;
           /** 0X29 if next three fields are valid */
  uint8_t  bootSignature;
           /**
            * A random serial number created when formatting a disk,
            * which helps to distinguish between disks.
            * Usually generated by combining date and time.
            */
  uint32_t volumeSerialNumber;
           /**
            * A field once used to store the volume label. The volume label
            * is now stored as a special file in the root directory.
            */
  char     volumeLabel[11];
           /**
            * A text field with a value of FAT32.
            */
  char     fileSystemType[8];
           /** X86 boot code */
  uint8_t  bootCode[420];
           /** must be 0X55 */
  uint8_t  bootSectorSig0;
           /** must be 0XAA */
  uint8_t  bootSectorSig1;
} PACKED;
/** Type name for FAT32 Boot Sector */
typedef struct fat32_boot_s Fat32BootBlock_t;


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
typedef struct MasterBootRecord_s mbr_t;


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
  mbr_t   mbr;
          /** Used to access to a cached FAT16 boot sector. */
  Fat32BootBlock_t fbs;
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
#define LAST_CLUSTER    0xffff


typedef struct FatInfo_s {
    uint32_t blocksPerFat;       // FAT size in blocks
    uint32_t clusterCount;       // clusters in one FAT

    uint8_t bytesPerClusterShift; // by shifting with this, can convert between bytes and clusters
    uint16_t bytesPerClusterMask; // by masking out with this, can get the offset in cluster

    uint32_t dataStartBlock;     // first data block number
    uint8_t fatCount;            // number of FATs on volume
    uint32_t fatStartBlock;      // start block for first FAT
    uint16_t rootDirEntryCount;  // number of entries in FAT16 root dir
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
