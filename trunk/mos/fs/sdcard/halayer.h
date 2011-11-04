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

/**
 *  HALayer.h:      implementation of the HALayer.
 *  class HALayer:  defines the hardware specific portions
 *                  of the MMC/SD card FAT library.
 *
 *  Author:    Ivan Sham
 *  Date: June 26, 2004
 *  Version: 2.0
 *  Note: Developed for William Hue and Pete Rizun
 *
 **/

#ifndef MANSOS_FATLIB_HALAYER_H
#define MANSOS_FATLIB_HALAYER_H

#include <kernel/defines.h>
#include "mmc.h"

// ---------------  types and defines required by FATlib

#define TRUE true
#define FALSE false

typedef bool boolean;

// redefine these in case support for larger disks is needed
typedef uint16_t cluster_t;
#if FAT16_SUPPORT
typedef uint32_t sector_t;
#else
typedef uint16_t sector_t;
#endif

// ---------------   variables

/**
 *  sectors offset from absolute sector 0 in the MMC/SD.
 *  All operations with the MMC/SD card are offseted with
 *  this variable in a microcontroller application
 *  ****this equals zero in Windows application
 **/
extern sector_t sectorZero;


// ---------------  

/* Memory card sector size */
#define SECTOR_SIZE 512


/**
 *    checks if there is a memory card in the slot
 *
 *    @return    FALSE        card not detected
 *    @return TRUE        car detected
 **/
boolean detectCard(void);


/**
 *    locates the offset of the partition table from the absolute zero sector
 *
 *    return ...             the offset of the partition table
 **/
uint32_t sdCardGetPartitionOffset(uint8_t *buf);


/**
 *	read content of the sector indicated by the input and place it in the buffer
 *
 *	@param	sector		sector to be read
 *	@param	*buf		pointer to the buffer
 *
 *	@return	...			number of bytes read
 **/
char readSector(sector_t sector, uint8_t *buf);


/**
 *	read partial content of the sector indicated by the input and place it in the buffer
 *
 *	@param	sector		sector to be read
 *	@paran	*buf		pointer to the buffer
 *	@param	bytesOffset	starting point in the sector for reading
 *	@param	bytesToRead	number of bytes to read
 *
 *	@return	...			number of bytes read
 **/
char readPartialSector(sector_t sector,
                       uint16_t byteOffset,
                       uint16_t bytesToRead,
                       unsigned char *buf);


/**
 *	read partial content at the end of the sector1 indicated by the input and
 *	the begging of sector 2 and place it in the buffer
 *
 *	@param	sector1		first sector to be read
 *	@param	sector2		second sector to be read
 *	@paran	*buf		pointer to the buffer
 *	@param	bytesOffset	starting point in the sector for reading
 *	@param	bytesToRead	number of bytes to read
 *
 *	@return	...			number of bytes read
 **/
char readPartialMultiSector(sector_t sector1,
                            sector_t sector2,
                            uint16_t byteOffset,
                            uint16_t bytesToRead,
                            uint8_t *buf);


/**
 *	writes content of the buffer to the sector indicated by the input
 *
 *	@param	sector		sector to be written
 *	@param	*buf		pointer to the buffer
 *
 *	@return	...			number of bytes written
 **/
char writeSector(sector_t sector, const uint8_t *buf);


/**
 *	write the content of the buffer indicated by the input to the MMC/SD card location
 *	indicated by the input sector.
 *
 *	@param	sector		sector to be written to
 *	@paran	*buf		pointer to the buffer
 *	@param	bytesOffset	starting point in the sector for writing
 *	@param	bytesToRead	number of bytes to write
 *
 *	@return	...			number of bytes written
 **/
char writePartialSector(sector_t sector,
                        uint16_t byteOffset,
                        uint16_t bytesToWrite,
                        const uint8_t *buf);


/**
 *	write the content of the buffer to the end of the sector1
 *	and the begging of sector 2
 *
 *	@param	sector1			first sector to be written to
 *	@param	sector2			second sector to be written to
 *	@paran	*buf			pointer to the buffer
 *	@param	bytesOffset		starting point in the sector for reading
 *	@param	bytesToWrite	number of bytes to write
 *
 *	@return	...				number of bytes written
 **/
char writePartialMultiSector(sector_t sector1,
                             sector_t sector2,
                             uint16_t byteOffset,
                             uint16_t bytesToWrite,
                             const uint8_t *buf);

#endif
