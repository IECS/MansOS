/**
 * Copyright (c) 2008-2012 the MansOS team. All rights reserved.
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

/**
 *  FATLib.c:       implementation of the FATLib class.
 *  class FATLib:   a portable FAT decoder class which is hardware independent.
 *          All hardware specific operations are abstracted with the
 *          class HALayer.  The FATLib class operates with only the buffer
 *          which it passes to the class HALayer
 *
 *  Author: Ivan Sham
 *  Date: July 1, 2004
 *  Version: 2.0
 *  Note: Developed for William Hue and Pete Rizun
 *
 **/

#include "fatlib.h"
#include "halayer.h"
#include <kernel/defines.h>
#include <hil/udelay.h>
#include <lib/byteorder.h>
#include <lib/dprint.h>
#include <string.h>

//------------------
// private member variables:
//------------------

/**
 *  defines the structure of a opened file
 **/
typedef struct
{
    cluster_t currentCluster;
    uint32_t byteCount; 
    cluster_t firstCluster;    
    uint32_t fileSize;
    uint8_t sectorCount;    
    file_handle_t fileHandle;
    boolean updateDir;
} file;

// note: the variables should be 32 bit long in some cases to automatically
// promote correct variable size in arithemtical expressins later


/**
 *  array of size BUFFER_SIZE of pointers to nodes of
 *  struct file opened for reading
 **/
static file openedRead[BUFFER_SIZE];

/**
 *  array of size BUFFER_SIZE of pointers to nodes of
 *  struct file opened for writing
 **/
static file openedWrite[BUFFER_SIZE];

/**
 *  error code of the last error.
 **/
uint16_t fatLibLastError;

/**
 *  sectors offset from absolute sector 0 in the MMC/SD.
 *  All operations with the MMC/SD card are offseted with
 *  this variable in a microcontroller application
 *  ****this equals zero in Windows application
 **/
sector_t sectorZero;

/**
 *  the sector in the MMC/SD card where data starts.
 *  This is right after the root directory sectors in FAT16
 **/
static sector_t dataStarts;

/**
 *  the first sector where entries in the root directory is
 *  stored in FAT16.
 **/
static sector_t rootDirectory;

/**
 *  the number of sectors per File Allocation Table (FAT)
 **/
static sector_t sectorsPerFAT;

/**
 *  the number of sectors per cluster for the MMC/SD card
 **/
static sector_t sectorsPerCluster;

/**
 *  the number of reserved sectors in the MMC/SD card
 **/
static sector_t reservedSectors;

/**
 *  the number of File Allocation Table (FAT) on the MMC/SD.
 *  this is usually 2
 **/
static uint16_t numOfFATs;

/**
 *  the number of sectors in the section for root directory
 *  entries.
 **/
static sector_t rootSectors;

/**
 *  the file system of which the MMC/SD card is formatted in
 *
 *  fileSys == 0  -> Unknown file system
 *  fileSys == 1  -> FAT16
 *  fileSys == 2  -> FAT12
 **/
static uint8_t fileSys;

/**
 *  number of files opened for reading
 **/
static uint8_t filesOpenedRead;

/**
 *  number of files opened for writing
 **/
static uint8_t filesOpenedWrite;

#if FAT16_SUPPORT
/**
 *  FAT sector number last searched for an empty cluster
 **/
static uint16_t last_FAT_sector_searched_for_empty;
#endif // FAT16_SUPPORT


/* Local Prototypes */
static void FATLibInit();
//static uint8_t identifyFileSystem(unsigned char *buf);
//static signed char readFileSystemInfo(unsigned char *buf);
static cluster_t findEmptyCluster(unsigned char *buf);
static cluster_t getNextFAT(cluster_t cluster, unsigned char *buf);
static void updateFAT(cluster_t oldCluster, cluster_t newCluster, unsigned char *buf);
static signed char getFileHandle();
static cluster_t getFirstCluster(const char *pathname, unsigned char *buf, uint32_t *fileSize);
static cluster_t createNewEntry(const char *entryName, unsigned char *buf);
static signed char findFileIndex(file_handle_t handle);
static result_t openedCheck(cluster_t cluster);
static void updateDirectory(cluster_t firstCluster, uint32_t filesize, unsigned char *buf);


/**
 *  default constructor
 **/
static void FATLibInit()
{
    unsigned char i;
    for(i = 0; i < BUFFER_SIZE;i++)
    {
        openedRead[i].fileHandle = -1;
        openedWrite[i].fileHandle = -1;
    }
}

/**
 *  identifies the file system used by the MMC/SD card
 *
 *  @param  buf         the buffer to be used to access the MMC/SD card
 *
 *  @return 0           for unknow file system
 *  @return 1           for FAT16
 **/
bool sdCardIdentifyFs(unsigned char *buf)
{
    char status;

    fileSys = UNKNOWN;

    status = readSector(0, buf);
    if (status != MMC_SUCCESS) {
        fatLibLastError = 3;
        return false;
    }

    // PRINTF("fs=%d %d %d %d %d\n",
    //        buf[54], buf[55], buf[56], buf[57], buf[58]);

    if ((buf[54] == 'F') &&
            (buf[55] == 'A') &&
            (buf[56] == 'T') &&
            (buf[57] == '1')) {
        if (buf[58] == '6')
            fileSys = FAT16;
        else if (buf[58] == '2')
            fileSys = FAT12;
    }
#if !FAT16_SUPPORT
    if (fileSys == FAT16) fileSys = UNKNOWN;
#endif
    return fileSys != UNKNOWN;
}

/**
 *  initialize variables for file system
 *
 *  @param  buf     the buffer to be used to access the MMC/SD card
 *
 *  @return 0           variables sucessfully initialized
 *  @return -1          unknown file system
 *  @return -2          error reading from MMC/SD card 
 **/
bool sdCardReadFsInfo(unsigned char *buf)
{
    unsigned int maxRootEntry = 0;

    sectorsPerCluster = buf[13];
    reservedSectors = le16read(&buf[14]);
    numOfFATs = buf[16];

    maxRootEntry = le16read(&buf[17]);
    sectorsPerFAT = le16read(&buf[22]);

    rootDirectory = reservedSectors + sectorsPerFAT * numOfFATs;
    rootSectors = (maxRootEntry / SECTOR_SIZE) * 32;
    dataStarts = rootDirectory + rootSectors;

    return true;
}

/**
 *  initialize the system
 *
 *  @return 0           UNKNOWN file system
 *  @return 1           FAT16 file system
 *  @return 2           FAT12 file system
 *  @return 3           could not set block length
 *  @return 4           could not initialize memory card
 **/
bool fat_initialize(void)
{
    char status;
    unsigned char buf[SECTOR_SIZE];
    uint16_t timeout;

    FATLibInit();

    // Initialisation of the MMC/SD-card
#define MAX_TRIES 150
    for (timeout = 0; timeout < MAX_TRIES; timeout++) {
        status = mmcInit();
        if (status == MMC_SUCCESS) break;
    }
    if (timeout >= MAX_TRIES) {
        fatLibLastError = 1;
        return false;
    }

    while (mmcPing() != MMC_SUCCESS);  // Wait till card is inserted

    status = mmcSetBlockLength(SECTOR_SIZE);
    if (status != MMC_SUCCESS) {
        fatLibLastError = 2;
        return false;
    }

    mdelay(100);
    sectorZero = sdCardGetPartitionOffset(buf);
    PRINTF("sectorZero=%lu\n", sectorZero);
    mdelay(100);
    status = sdCardIdentifyFs(buf);
    if (status) status = sdCardReadFsInfo(buf);
    return status;
}

#if FAT16_SUPPORT
/**
 *  finds an unoccupied cluster in the FAT and returns the location.
 *  The previously empty location in the FAT will be changed to a
 *  end of file marker
 *
 *  @pre    file system is properly initialized and fileSys != UNKNOWN
 *
 *  @param  buf         the buffer to be used to access the MMC/SD card
 *
 *  @return 0           no more clusters available (disk full)
 *  @return ...         location of empty cluster
 **/
static cluster_t findEmptyClusterFAT16(unsigned char *buf)
{
    sector_t sector, end_sector;
    cluster_t returnCluster;
    unsigned int i;
    unsigned char j;

    if (last_FAT_sector_searched_for_empty == 0) {
        sector = reservedSectors;
    } else {
        sector = last_FAT_sector_searched_for_empty;
    }
    end_sector = reservedSectors + sectorsPerFAT;

    // 256 entries in a 512-byte FAT sector
    returnCluster = (sector - reservedSectors) * 256;

    while (TRUE)
    {
        while (sector < end_sector)
        {
            readSector(sector, buf);
            for (i = 0; i < SECTOR_SIZE; i += 2) //i += 2*fileSys)
            {
                if (buf[i] == 0x00 && buf[i+1] == 0)                   
                {
                    // This cluster is unused.
                    buf[i] = 0xFF;
                    buf[i + 1] = 0xFF;

                    for (j = 0; j < numOfFATs; j++) {
                        writeSector(sector + (j * sectorsPerFAT), buf);
                    }

                    // Remember where we found it:
                    last_FAT_sector_searched_for_empty = sector;
                    return returnCluster;
                }
                returnCluster++;
            }
            sector++;
        }

        // We reached the end of the FAT.  Should we wrap around and search from
        // the beginning?
        if (last_FAT_sector_searched_for_empty > reservedSectors)
        {
            // Yes.
            sector = reservedSectors;
            end_sector = last_FAT_sector_searched_for_empty;
            last_FAT_sector_searched_for_empty = 0;
            returnCluster = 0;
            // We will take the else block next time if we don't find anything.
        }
        else
        {
            return 0; 
        }
    }
}
#endif // FAT16_SUPPORT

static cluster_t findEmptyClusterFAT12(unsigned char *buf)
{
    sector_t sector = reservedSectors;
    sector_t end_sector = reservedSectors + sectorsPerFAT;

    cluster_t cluster = 0;
    uint8_t ignoreBytes = 0; // how much bytes to ignore at the start of sector

    for (; sector < end_sector; ++sector) {
        readSector(sector, buf);

        // XXX: this algorithm will fail to use those sectors
        // where the cluster number is stored righ on the sector boundary.
        unsigned char *p = buf + ignoreBytes;
        unsigned char *end = buf + SECTOR_SIZE;
        bool foundUnused = false;
        for (;p + 2 < end; p += 3, cluster += 2) {
            // always in little endian byte order
            uint16_t cluster1Bytes = le16read(p);
            uint16_t cluster2Bytes = le16read(p + 1);

            cluster_t cluster1 = cluster1Bytes & FAT12_MASK;
            cluster_t cluster2 = cluster2Bytes >> 4;

            // is some of the clusters unused?
            if (cluster1 == UNUSED) {
                cluster1Bytes |= (0xFFFF & FAT12_MASK);
                le16write(p, cluster1Bytes);
                foundUnused = true;
                break;
            }
            if (cluster2 == UNUSED) {
                cluster2Bytes |= (0xFFFF & FAT12_MASK) << 4;
                le16write(p + 1, cluster2Bytes);
                cluster += 1;
                foundUnused = true;
                break;
            }
        }

        if (foundUnused) {
            uint8_t j;
            for(j = 0; j < numOfFATs; j++) {
                writeSector(sector + (j * sectorsPerFAT), buf);
            }
            return cluster;
        }

        ignoreBytes++;
        if (ignoreBytes == 3) ignoreBytes = 0;
        else cluster += 2; // 2 unused clusters
    }

    return 0;
}

static inline cluster_t findEmptyCluster(unsigned char *buf)
{
#if FAT16_SUPPORT
    if (fileSys == FAT16) return findEmptyClusterFAT16(buf);
#endif
    return findEmptyClusterFAT12(buf);
}

/**
 *  gets the next cluster in the FAT chain given the current cluster
 *
 *  @param  cluster     the current cluster
 *  @param  buf         the buffer to be used to access the MMC/SD card
 *
 *  @return ...         the location of the next cluster or end of file marker 
 **/
static cluster_t getNextFAT(cluster_t cluster, unsigned char *buf)
{
    uint32_t byteOffset;
    sector_t sectorOffset;
    cluster_t nextCluster;

    if (fileSys == FAT16) {
        byteOffset = cluster * 2;
    } else { // FAT12
        byteOffset = cluster * 3 / 2;
    }
    sectorOffset = byteOffset / SECTOR_SIZE;
    sectorOffset += reservedSectors;
    byteOffset &= 0x01FF;
    readSector(sectorOffset, buf);
    if (fileSys == FAT16) {
        nextCluster = le16read(&buf[byteOffset]);
    } else { // FAT12
        nextCluster = le16read(&buf[byteOffset]);
        if (cluster & 0x1) {
            nextCluster >>= 4;
        }
        nextCluster &= FAT12_MASK;
    }

    return nextCluster;
}

/**
 *  updates the FAT from in location of oldCluster and place the newCluster in 
 *  that location
 *
 *  @param  buf         the buffer to be used to access the MMC/SD card
 *  @param  oldCluster  the location of the cluster to be replaced 
 *  @param  newCluster  the new location
 **/
static void updateFAT(uint16_t cluster, uint16_t newCluster, unsigned char *buf)
{
    // TODO: merge this function wih getNextFAT?
    sector_t sectorOffset;
    uint32_t byteOffset;
    unsigned char j;

    if (fileSys == FAT16) {
        byteOffset = cluster * 2;
    } else {
        byteOffset = cluster * 3 / 2;
    }

    sectorOffset = byteOffset / SECTOR_SIZE;
    sectorOffset += reservedSectors;
    byteOffset &= 0x01FF;
    readSector(sectorOffset, buf);
    if (fileSys == FAT16) {
        le16write(&buf[byteOffset], newCluster);
    } else { // FAT12
        uint16_t newClusterBytes = le16read(&buf[byteOffset]);
        if (cluster & 0x1) {
            newClusterBytes &= 0x000f;
            newClusterBytes |= newCluster << 4;
        } else {
            newClusterBytes &= 0xf000;
            newClusterBytes |= newCluster & FAT12_MASK;
        }
        le16write(&buf[byteOffset], newClusterBytes);
    }

    for(j = 0; j < numOfFATs; j++) {
        writeSector(sectorOffset + (j * sectorsPerFAT), buf);
    }
}

/**
 *  gets the next available file handle 
 *  Note:  all files have unique handle (no overlap between read and write)
 *
 *  @return -10         no more file handles available
 *  @return ...         file handle
 **/
static signed char getFileHandle()
{
    signed char i = 0;
    signed char newHandle = 0;
    while (newHandle < 2 * BUFFER_SIZE)
    {
        i = 0;
        while ((i < BUFFER_SIZE) && (i >= 0))
        {
            if (openedRead[i].fileHandle != -1)
            {               
                if(openedRead[i].fileHandle==newHandle)
                {
                    i = -128;
                }
            }
            if ((openedWrite[i].fileHandle != -1)&&(i>=0))
            {
                if(openedWrite[i].fileHandle==newHandle)
                {
                    i = -128;
                }
            }
            i++;
        }
        if(i == BUFFER_SIZE)
        {
            return newHandle;
        }
        newHandle++;
    }
    return -10;
}

/**
 *  closes the file indicated by the input
 *  
 *  @param  fileHandle  handle of file to be closed
 *  
 *  @return 0           file sucessfully closed
 *  @return -1          invalid file handle
 *  @return -2          invalid file system
 **/
signed char fat_close(file_handle_t fileHandle)
{
    unsigned char i;

    if (fileSys == UNKNOWN) {
        return -2;
    }

    if (detectCard() == FALSE) {
        return -10;
    }

    fat_flush();

    for (i = 0; i < filesOpenedRead; i++)
    {
        if(openedRead[i].fileHandle == fileHandle)
        {
            filesOpenedRead--;
            openedRead[i].fileHandle = -1;
            return 0;
        }
    }
    for (i = 0; i < filesOpenedWrite; i++)
    {
        if (openedWrite[i].fileHandle == fileHandle)
        {
            filesOpenedWrite--;
            openedWrite[i].fileHandle = -1;
            return 0;
        }
    }
    return -1;
}

/**
 *  case insensitive compare of the 2 inputs
 *  returns true if and only if c1 = c2 or if
 *  c1 and c2 are the same character with different
 *  capitialization
 *
 *  @param  c1          input 1
 *  @param  c2          input 2
 **/
static inline boolean charEquals(char c1, char c2)
{
    if (c1 >= 'A' && c1 <= 'Z') c1 |= 0x20; // to lowercase
    if (c2 >= 'A' && c2 <= 'Z') c2 |= 0x20; // to lowercase
    return c1 == c2;
}

/**
 *  determines the location of the cluster of the file or directory
 *  pointed to by the input pathname.  Only the first token in 
 *  the string pointed to by pathname is used (anything before '\').
 *
 *  @param  buf         the buffer to be used to access the MMC/SD card
 *  @param  pathname    pointer to the string of the pathname
 *  @param  fileSize    stores the size of the file being seeked
 *
 *  @return 0           if the directory or file does not exist
 *  @return ...         the first cluster of the directory or file being seeked
 **/
static cluster_t getFirstCluster(const char *pathname, unsigned char *buf, uint32_t *fileSize)
{
    int offset = 0;
    uint16_t sectorCount;
    sector_t sector;
    unsigned char i, j;
    unsigned char tokenLength = 0;  // path name contails tokes less than 255 characters long
    const char *tempPathname;
    boolean matchingNames;
    boolean targetFound;

    sector = rootDirectory;
    tempPathname = pathname;
    while ((*tempPathname != 0x5C)&&(*tempPathname))
    {
        tokenLength++;
        tempPathname++;
    }

    targetFound = FALSE;
    sectorCount = 0;
    i = 0;
    PRINTF("GFC: readSector %u\n", sector);
    PRINTF("GHC find: %s\n", pathname);
    readSector(sector, buf);
    if (tokenLength < 13) {
        while ((targetFound == FALSE)&&(sector > 0)) {
            offset = i * 32;
            tempPathname = pathname;
            matchingNames = TRUE;

            if (buf[offset] == '\0') {
                // End of entries.
                break;
            }

            // Check attributes except for ARCHIVE or READONLY, and for deleted entries:
            if (((buf[11+offset] & 0xDE) == 0x00)&&((buf[offset] & 0xFF) != 0xE5)) {
                // debug
                char tmpfbuf[8 + 3 + 2];
                memcpy(tmpfbuf, buf + offset, 8 + 3 + 1);
                tmpfbuf[sizeof(tmpfbuf) - 1] = 0;
                PRINTF("GHC: %s\n", tmpfbuf);

                for (j = 0; j < 8; j++) {
                    if (*tempPathname != '.' && *tempPathname != '\0') {
                        // Before end of filename, characters must match:
                        if (!charEquals(buf[offset+j], *tempPathname)) {
                            matchingNames = FALSE;
                        }

                        tempPathname++;
                    }
                    else if ((buf[offset+j]) != ' ') {
                        // After end of filename, must have blanks.
                        matchingNames = FALSE;
                    }
                }

                if (*tempPathname != '\0') {
                    tempPathname++;
                }

                for (j = 8; j < 11; j++) {
                    if (*tempPathname != '\0') {
                        // Before end of extension, characters must match:
                        if (!charEquals(buf[offset+j], *tempPathname)) {
                            matchingNames = FALSE;
                        }

                        tempPathname++;
                    }
                    else if ((buf[j+offset]) != ' ') {
                        // After end of extension, must have blanks.
                        matchingNames = FALSE;
                    }
                }

                if (matchingNames) {
                    targetFound = TRUE;
                }
            }

            i++;
            if (i == 16) {
                // 16 = directory entries per sector (fixed number)
                i = 0;
                sectorCount++;
                if (sectorCount < rootSectors) {
                    sector++;
                } else {
                    sector = 0;
                }

                readSector(sector, buf);
            }
        }
    }

    if (targetFound == FALSE) {
        PRINTF("GFC: not found!\n");
        mdelay(100);
        return 0;
    }

    cluster_t cluster;
    cluster = le16read(&buf[26+offset]);
    *fileSize = le32read(&buf[28+offset]);
    PRINTF("GFC: cluster=%u\n", cluster);
    mdelay(100);
    return cluster;
}


/**
 *  creates a new entry (file or directory) at the location indicated by the
 *  input cluster.  The input control determines whether a file or a
 *  directory is created
 *  *****ONLY FILES AND DIRECTORY WITH SHORT FILE NAMES ARE SUPPORTED
 *
 *  @param  buf         the buffer to be used to access the MMC/SD card 
 *  @param  entryName   pointer to the name of the new entry
 *
 *  @return 0           if an error occurs while adding a new entry
 *  @return ...         the location of the first cluster of the new entry
 **/
static cluster_t createNewEntry(const char *entryName, unsigned char *buf)
{
    uint16_t offset = 0;
    uint16_t sectorCount = 0;
    uint16_t tokenLength = 0;
    sector_t sector;
    cluster_t newCluster;
    unsigned char i;
    const char *tempEntryName;
    boolean done = FALSE;

    // Note:  findEmptyCluster() will mark the returned cluster in the FAT
    // as used.  If we fail for some other reason below, we should really
    // free the newCluster, but we don't right now.  Also, unless we are
    // creating a directory, we really shouldn't allocate a first cluster
    // until some data is written to the file.  Additionally, if we re-use a
    // deleted entry, we should re-use the cluster chain, adding clusters as
    // required, but we don't do that right now either.
    newCluster = findEmptyCluster(buf);
    if (newCluster == 0) {
        return 0;   // no more empty cluster
    }

    sector = rootDirectory;

    tempEntryName = entryName;

    while ((*tempEntryName != '.')&&(*tempEntryName)&&(*tempEntryName != 0x5C))
    {
        tokenLength++;
        tempEntryName++;
    }

    while (!done)
    {
        readSector(sector, buf);
        i = 0;
        while (i < 16)
        {
            offset = i * 32;            
            if(((buf[offset] & 0xFF) == 0x00) || ( (buf[offset] & 0xFF) == 0xE5) )
            {
                done = TRUE;
                i = 15;
            }
            i++;
        }
        if (!done)
        {
            sectorCount++;
            if(sectorCount < rootSectors) {
                sector++;
            }
            else {
                return 0;   // no more root directory
            }
        }
    }

    buf[offset+11] = 0x20;
    if (tokenLength < 9) {
        for (i = 0; i < 8; i++) {
            if (*entryName != '.') {
                if ((*entryName >= 'a')&&(*entryName <= 'z')) {
                    buf[offset+i] = (*entryName - 32);
                }
                else {
                    buf[offset+i] = *entryName ;
                }
                entryName++;
            }
            else {
                buf[offset+i] = 0x20;
            }
        }
        entryName++;
        for (i = 8; i < 11; i++) {
            if (*entryName) {
                if ((*entryName >= 'a')&&(*entryName <= 'z')) {
                    buf[offset+i] = (*entryName - 32);
                }
                else {
                    buf[offset+i] = *entryName ;
                }
                entryName++;
            }
            else {
                buf[offset+i] = 0x20;
            }
        }
    }
    else {
        // file with long file name
        return 0;
    }

    buf[offset+12] = 0x00;
    buf[offset+13] = 0x00;

    // Set the date and time to January 02, 2005 21:00:00 .
    // If a real time clock is available, we could use it.
    buf[offset+14] = (04 >> 1) & 0x1F;  // Seconds.
    buf[offset+14] |= 03 << 5;  // Part of minutes.
    buf[offset+22] = buf[offset+14];

    buf[offset+15] = (03 >> 3) & 0x07;  // More of minutes.
    buf[offset+15] |= 21 << 3;  // Hours.
    buf[offset+23] = buf[offset+15];            
    
    buf[offset+16] = 02 & 0x1F;     // Day.
    buf[offset+16] |= (01 << 5) & 0xE0;   // Part of month.
    buf[offset+18] = buf[offset+16];
    buf[offset+24] = buf[offset+16];
                
    buf[offset+17] = (01 >> 3) & 0x01;   // More of month.
    buf[offset+17] |= (((2005 - 1980) & 0xFF) << 1) & 0xFE;  // Year.
    buf[offset+19] = buf[offset+17];
    buf[offset+25] = buf[offset+17];

    buf[offset+26] = (newCluster & 0xFF);
    buf[offset+27] = (newCluster >> 8) & 0xFF;

    for(i = 28; i < 32; i++) {
        buf[offset+i] = 0x00;
    }
    writeSector(sector, buf);

    return newCluster;
}


/**
 *  finds the index of the for the array of pointers which
 *  corresponds to the input file handle.
 *
 *  @param  handle      the handle of the file being seeked
 *
 *  @return -1          invalid file handle
 *  @return ...         the index to the correct file pointer
 **/
static signed char findFileIndex(file_handle_t handle)
{
    signed char i;

    for(i = 0; i < BUFFER_SIZE; i++) {
        if (openedWrite[i].fileHandle == handle
                || openedRead[i].fileHandle == handle) {
            return i;
        }
    }
    return -1;
}

/**
 *  checks to see if the file indicated by the input cluster
 *  is already opened for either reading or writing
 *
 *  @pre    the input must be the location of the first cluster 
 *          of a file
 *  @param  cluster     first cluster of the file being checked
 *
 *  @return -1          file is already opened for reading
 *  @return -2          file is already opened for writing
 *  @return 0           file is currently not opened
 **/
static result_t openedCheck(cluster_t cluster)
{
    unsigned char i;
    for (i = 0; i < BUFFER_SIZE; i++) {
        if (openedRead[i].fileHandle != -1
                && openedRead[i].firstCluster == cluster) {
            return -1;
        }
        if (openedWrite[i].fileHandle != -1
                && openedWrite[i].firstCluster == cluster) {
            return -2;
        }
    }
    return 0;
}

/**
 *  opens the file indicated by the input path name.  If the pathname
 *  points to a valid file, the file is added to the list of currently
 *  opened files for reading and the unique file handle is returned.
 *
 *  @param  pathname    a pointer to the location of the file to be opened
 *  @param  buf         the buffer to be used to access the MMC/SD card
 *
 *  @return -1          invalid pathname
 *  @return -2          file does not exist
 *  @return -3          file already opened for writing
 *  @return -4          file already opened for reading
 *  @return -10         no handles available
 *  @return -20         memory card error
 *  @return -128        other error
 *  @return ...         file handle of sucessfully opened file
 **/
file_handle_t fat_openRead(const char *pathname)
{
    unsigned char buf[SECTOR_SIZE];
    const char *tempPathname;
    cluster_t cluster;
    uint32_t fileSize = 0;
    signed char i;
    signed char index = -1;

    if (fileSys <= 0) {
        return -1;
    }

    if (detectCard() == FALSE) {
        return -20;
    }

    if (filesOpenedRead >= BUFFER_SIZE) {
        return -10;
    }
    for (i = 0; i < BUFFER_SIZE; ++i) {
        if (openedRead[i].fileHandle == -1) {
            index = i;
            break;
        }
    }
    if (index == -1) { // impossible in normal case
        return -128;
    }

    if (*pathname == 0x5C) {
        pathname++;
    }

    tempPathname = pathname;
    while (*tempPathname) {
        if (*tempPathname == 0x5C)
            return -128;
        tempPathname++;
    }

    tempPathname = pathname;
    cluster = getFirstCluster(tempPathname, buf, &fileSize);
    if (cluster <= 0)
        return -2;

    i = openedCheck(cluster);
    if (i < 0) {
        if(i == -1)
            return -4;
        return -3;
    }

    memset(&openedRead[index], 0, sizeof(openedRead[0]));
    openedRead[index].firstCluster = cluster;
    openedRead[index].currentCluster = cluster;
    openedRead[index].fileSize = fileSize;
    openedRead[index].fileHandle = getFileHandle();
    filesOpenedRead++;
    return openedRead[index].fileHandle;
}

/**
 *  opens the file indicated by the input path name.  If the pathname
 *  points to a valid path, the file is created and added to the list of
 *  currently opened files for writing and the unique file handle is returned.
 *
 *  @param  pathname    a pointer to the location of the file to be opened
 *
 *  @return -1          invalid pathname
 *  @return -2          file already exists
 *  @return -3          file already opened for writing
 *  @return -4          no directory entries left
 *  @return -10         no handles available
 *  @return -20         memory card error
 *  @return -128        other error
 *  @return (non-negative)  file handle of sucessfully opened file
 **/
file_handle_t fat_openWrite(const char *pathname)
{
    unsigned char buf[SECTOR_SIZE];
    const char *tempPathname;
    cluster_t cluster;
    uint32_t fileSize = 0;
    signed char i;
    signed char index = -1;

    if (fileSys <= 0) {
        return -1;
    }

    if (detectCard() == FALSE) {
        return -20;
    }

    if (filesOpenedWrite >= BUFFER_SIZE) {
        return -10;
    }
    for (i = 0; i < BUFFER_SIZE; i++) {
        if (openedWrite[i].fileHandle == -1) {
            index = i;
            break;
        }
    }
    if (index == -1) { // impossible in normal case
        return -128;
    }

    
    if (*pathname == 0x5C) {
        pathname++;
    }
    tempPathname = pathname;
    while (*tempPathname) {
        if(*tempPathname == 0x5C)
            return -128;
        tempPathname++;
    }   

    tempPathname = pathname;

    cluster = getFirstCluster(tempPathname, buf, &fileSize);
    if (cluster != 0) {
        return -2;
    }

    cluster = createNewEntry(tempPathname, buf);
    if (cluster == 0)
        return -4;

    memset(&openedWrite[index], 0, sizeof(openedWrite[0]));
    openedWrite[index].currentCluster = cluster;
    openedWrite[index].fileHandle = getFileHandle();
    openedWrite[index].firstCluster = cluster;
    filesOpenedWrite++;
    return openedWrite[index].fileHandle;
}

/**
 *  reads the content of the file identified by the input handle.  It reads from
 *  where the last read operation on the same file ended.  If it's the first time
 *  the file is being read, it starts from the begining of the file.
 *
 *  @pre    nByte < SECTOR_SIZE
 *
 *  @param  buf         the buffer to be used to access the MMC/SD card
 *  @param  handle      handle of file to be closed
 *  @param  nByte       number of bytes to read
 *
 *  @return -10         memory card error
 *  @return -1          invalid handle
 *  @return ...         number of bytes read
 **/
int16_t fat_read(file_handle_t handle, unsigned char *buf, uint32_t nByte)
{
    sector_t sectorToRead;
    sector_t sectorToRead2;
    signed char index;
    unsigned char tempBuf[SECTOR_SIZE];

    if (detectCard() == FALSE) {
        return -10;
    }

    index = findFileIndex(handle);
    if (index == -1) {
        return -1;       // invalid handle
    }

    cluster_t lastCluster;
    if (fileSys == FAT16) {
        lastCluster = 0xFFF6;
    } else { // FAT12
        lastCluster = 0xFF7;
    }

    if ((openedRead[index].currentCluster > lastCluster)
            || (openedRead[index].currentCluster < 2)) {
        return -32768;   // already reached the end of file
    }

    PRINTF("read, fs=%u\n",
            openedRead[index].fileSize);
    PRINTF("read, bc=%u\n",
            openedRead[index].byteCount);

    sectorToRead = (openedRead[index].currentCluster - 2) * sectorsPerCluster + dataStarts;
    sectorToRead += openedRead[index].sectorCount;

    if (openedRead[index].fileSize < nByte) {
        nByte = openedRead[index].fileSize;
    }
    if ((openedRead[index].byteCount + nByte) > SECTOR_SIZE)
    {
        if ((openedRead[index].sectorCount + 1) == sectorsPerCluster)
        {
            // cross cluster read
            openedRead[index].currentCluster = getNextFAT(openedRead[index].currentCluster, tempBuf);
            sectorToRead2 = (openedRead[index].currentCluster - 2) * sectorsPerCluster + dataStarts;
            openedRead[index].sectorCount = 0;
        }
        else
        {
            // cross sector read
            sectorToRead2 = sectorToRead + 1;
            openedRead[index].sectorCount++;
        }
        readPartialMultiSector(sectorToRead, sectorToRead2, openedRead[index].byteCount, nByte, buf);
        openedRead[index].byteCount += nByte;
        openedRead[index].byteCount -= SECTOR_SIZE;
    }
    else
    {
        // single sector read
        readPartialSector(sectorToRead, openedRead[index].byteCount, nByte, buf);
        openedRead[index].byteCount += nByte;
    }
    if (nByte > openedRead[index].fileSize)
        openedRead[index].fileSize = 0;
    else
        openedRead[index].fileSize -= nByte;
    return nByte;
}


/**
 *  writes the content in the buffer to the file identified by the input handle.  It writes
 *  to where the last write operation on the same file ended.  If it's the first time
 *  the file is being written to, it starts from the beginning of the file.
 *
 *  @pre    nByte < SECTOR_SIZE
 *
 *  @param  buf         the buffer to be used to access the MMC/SD card
 *  @param  handle      handle of file to be written to
 *  @param  nByte       number of bytes to write
 *
 *  @return -10         memory card error
 *  @return -1          invalid handle
 *  @return -2          memory card is full
 *  @return ...         number of bytes written
 **/
int16_t fat_write(file_handle_t handle, unsigned char *buf, uint32_t nByte)
{
    sector_t sectorToWrite;
    sector_t sectorToWrite2;
    signed char index;
    unsigned char tempBuf[SECTOR_SIZE];

    if (detectCard() == FALSE) {
        return -10;
    }

    index = findFileIndex(handle);
    if (index == -1) {
        return -1; // invalid handle
    }
    sectorToWrite = (openedWrite[index].currentCluster - 2) * sectorsPerCluster + dataStarts;
    sectorToWrite += openedWrite[index].sectorCount;

    if ((openedWrite[index].byteCount + nByte) > SECTOR_SIZE)
    {
        if ((openedWrite[index].sectorCount + 1) == sectorsPerCluster)
        {
            // cross cluster write
            sectorToWrite2 = openedWrite[index].currentCluster;
            openedWrite[index].currentCluster = findEmptyCluster(tempBuf);
            if (openedWrite[index].currentCluster > 0)
            {
                updateFAT(sectorToWrite2, openedWrite[index].currentCluster, tempBuf);
                sectorToWrite2 = (openedWrite[index].currentCluster - 2) * sectorsPerCluster + dataStarts;
                openedWrite[index].sectorCount = 0;
            }
            else {
                return -2; // memory card full
            }
        }
        else
        {
            // cross sector write
            sectorToWrite2 = sectorToWrite + 1;
            openedWrite[index].sectorCount++;
        }
        writePartialMultiSector(sectorToWrite, sectorToWrite2, openedWrite[index].byteCount, nByte, buf);
        openedWrite[index].byteCount += nByte;
        openedWrite[index].byteCount -= SECTOR_SIZE;
    }
    else
    {
        // single sector write
        writePartialSector(sectorToWrite, openedWrite[index].byteCount, nByte, buf);
        openedWrite[index].byteCount += nByte;
    }

    openedWrite[index].fileSize += nByte;
    // Uncomment next line if you want FAT constantly updated, instead of using fat_flush():
    //updateDirectory(openedWrite[index].dirLocation, openedWrite[index].firstCluster, openedWrite[index].fileSize, tempBuf);
    openedWrite[index].updateDir = TRUE;
    return nByte;
}


/**
 *  updates the file size in the directory table for all files with the update flag set
 **/
void fat_flush(void)
{
    int i;
    unsigned char tempBuf[SECTOR_SIZE];
    for (i = 0; i < filesOpenedWrite; i++)
    {
        if (openedWrite[i].updateDir == TRUE)
        {
            updateDirectory(openedWrite[i].firstCluster, openedWrite[i].fileSize, tempBuf);
            openedWrite[i].updateDir = FALSE;
        }
    }
}


/**
 *  updates the file size and date/time in the directory table of the file identified by the input first cluster.
 *
 *  @param  buf             the buffer to be used to access the MMC/SD card
 *  @param  firstCluster    the location of the first cluster of the file being updated
 **/
static void updateDirectory(cluster_t firstCluster, uint32_t filesize, unsigned char *buf)
{
    sector_t sector;
    unsigned char sectorCount = 0;
    unsigned char i;
    unsigned int offset;
    cluster_t cluster;
    boolean done = FALSE;

    sector = rootDirectory;

    while ((!done) && (sectorCount < sectorsPerCluster)) {
        readSector(sector, buf);
        for (i = 0; i < 16; i++) {
            offset = i * 32;
            cluster = le16read(&buf[26+offset]);
            
            if (cluster == firstCluster) {   
                done = TRUE;

                buf[28+offset] = (filesize) & 0x000000FF;
                buf[29+offset] = (filesize >> 8) & 0x000000FF;
                buf[30+offset] = (filesize >> 16) & 0x000000FF;
                buf[31+offset] = (filesize >> 24) & 0x000000FF;

                /** Set access date to March 04, 2005 23:15:18 . **/
                // If a real time clock is available, we could use it.
                buf[offset+22] = (18 >> 1) & 0x1F;  // Seconds.
                buf[offset+22] |= 15 << 5;  // Part of minutes.
                buf[offset+23] = (15 >> 3) & 0x07;  // More of inutes.
                buf[offset+23] |= 23 << 3;  // Hours.
                buf[offset+18] = 04 & 0x1F; // Day.
                buf[offset+18] |= (03 << 5) & 0xE0; // Part of month.
                buf[offset+24] = buf[offset+18];
                buf[offset+19] = (03 >> 3) & 0x01;    // More of month.
                buf[offset+19] |= (((2005 - 1980) & 0xFF) << 1) & 0xFE; // Year.
                buf[offset+25] = buf[offset+19];

                writeSector(sector, buf);
            }
        }
        sector++;
        sectorCount++;
    }
}

uint8_t fatGetFileSys()
{
    return fileSys;
}

uint16_t fatGetLastError()
{
    return fatLibLastError;
}
