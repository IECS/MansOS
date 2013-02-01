/*
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

#ifndef MANSOS_FS_H
#define MANSOS_FS_H

/// \file
/// fs.h -- user-level MansOS flash file system interface
///

#include <defines.h>
#include <fs/types.h>

/**
 * Get information about a file.
 *
 * @param path   File path
 * @param buf    User-supplied buffer
 *
 * @return true on success or false on error.
 */
bool fsStat(const char * restrict path, struct fsStat * restrict buf);

/**
 * Open a file for reading or appending.
 *
 * @param path   File path
 * @param mode   Open mode: FS_READ or FS_APPEND
 *
 * @return file handle on success or -1 on error.
 */
int8_t fsOpen(const char *path, fsMode_t mode);

/**
 * Read up to count bytes from file.
 *
 * @param fd     File handle
 * @param buf    User-supplied buffer
 * @param count  Buffer size
 *
 * @return number of bytes read, 0 on EOF or -1 on error.
 */
ssize_t fsRead(int8_t fd, void *buf, size_t count);

/**
 * Get current read position in a file.
 *
 * @param fd     File handle
 *
 * @return file offset in bytes.
 */
fsOff_t fsTell(int8_t fd);

/**
 * Set current read position in a file. File must be open for reading.
 *
 * @param fd     File handle
 * @param pos    Position to seek to
 */
void fsSeek(int8_t fd, fsOff_t pos);

/**
 * Write up to count bytes to file.
 *
 * @param fd     File handle
 * @param buf    User data
 * @param count  Buffer size
 *
 * @return number of bytes written or -1 on error.
 */
ssize_t fsWrite(int8_t fd, const void *buf, size_t count);

/**
 * Write any pending data to the storage device. File must be open for writing.
 *
 * @param fd     File handle
 *
 * @return true on success or false on error.
 */
bool fsFlush(int8_t fd);

/**
 * Flush pending data and deallocate fd.
 *
 * @param fd     File handle
 *
 * @return true on success or false on error.
 */
bool fsClose(int8_t fd);

/**
 * Remove a file.
 *
 * @param path   File path
 *
 * @return true on success or false on error.
 */
bool fsRemove(const char *path);

/**
 * Rename a file.
 *
 * @param old    Current file name
 * @param new    New file name
 *
 * @return true on success or false on error.
 */
bool fsRename(const char *old, const char *new);

/**
 * Query last error.
 *
 * @return error number.
 */
fsError_t fsLastError(void);

#endif
