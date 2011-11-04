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
 *  FATLib.h:   interface for the FATLib.
 *  class FATLib:   a portable FAT decoder class which is hardware independent.
 *          All hardware specific operations are abstracted with the
 *          class HALayer.  The FATLib class operates with only the buffer
 *          which it passes to the class HALayer
 *
 *  Author: Ivan Sham
 *  Date: JUly 1, 2004
 *  Version: 2.0
 *  Note: Developed for William Hue and Pete Rizun
 *
 **/

#ifndef MANSOS_FATLIB_H
#define MANSOS_FATLIB_H

#include "halayer.h"
#include "sdcard_hal.h"


#define UNKNOWN 0
#define FAT16 1
#define FAT12 2

#define UNUSED 0

#define FAT12_MASK  0x0fff
#define FAT16_MASK  0xffff

#define DIRECTORY TRUE
#define FILE FALSE

#define READ TRUE
#define WRITE FALSE

// this is also the max number of files opened for reading or writing simulaneously
// (.e.g there can be 2 opened for reading and 2 more - for writing)
#define BUFFER_SIZE 2

#define abs(x)  (((x) > 0) ? (x) : (-(x)))

//typedef bool boolean;

typedef int8_t result_t;
typedef int8_t file_handle_t;

//------------------
// member functions:
//------------------

/**
 *  initialize the system
 *
 *  @return 0           UNKNOWN file system
 *  @return 1           FAT16 file system
 *  @return 2           FAT12 file system
 *  @return 3           could not set block length
 *  @return 4           could not initialize memory card
 **/
bool fat_initialize(void);

/**
 *  closes the file indicated by the input
 *
 *  @param  fileHandle  handle of file to be closed
 *
 *  @return 0           file sucessfully closed
 *  @return -1          invalid file handle
 *  @return -2          invalid file system
 **/
result_t fat_close(file_handle_t fileHandle);

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
result_t fat_openRead(const char *pathname);

/**
 *  opens the file indicated by the input path name.  If the pathname
 *  points to a valid path, the file is created and added to the list of
 *  currently opened files for writing and the unique file handle is returned.
 *
 *  @param  pathname    a pointer to the location of the file to be opened
 *
 *  @return -1          invalid pathname
 *  @return -2          file already exist
 *  @return -3          file already opened for writing
 *  @return -4          no directory entries left
 *  @return -10         no handles available
 *  @return -20         memory card error
 *  @return -128        other error
 *  @return (non-negative)  file handle of sucessfully opened file
 **/
result_t fat_openWrite(const char *pathname);

/**
 *  reads the content of the file identified by the input handle.  It reads from
 *  where the last read operation on the same file ended.  If it's the first time
 *  the file is being read, it starts from the beginning of the file.
 *
 *  @pre    nByte < SECTOR_SIZE
 *
 *  @param  buf         the buffer to be used to access the MMC/SD card
 *  @param  handle      handle of file to be read
 *  @param  nByte       number of bytes to read
 *
 *  @return -10         memory card error
 *  @return -1          invalid handle
 *  @return ...         number of bytes read
 **/
int16_t fat_read(file_handle_t handle, uint8_t *buf, uint32_t nByte);

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
int16_t fat_write(file_handle_t handle, uint8_t *buf, uint32_t nByte);

/**
 *  updates the file size in the directory table for all files with the update flag set
 **/
void fat_flush(void);

/**
 *  error code of the last error.
 **/
uint16_t fatGetLastError();

/**
 *  get current file system number
 **/
uint8_t fatGetFileSys();

#endif
