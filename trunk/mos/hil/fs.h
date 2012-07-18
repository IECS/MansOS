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
/*
 * fs.h -- user-level file system interface
 */

#ifndef _MANSOS_FS_H_
#define _MANSOS_FS_H_

#include <stdbool.h>
#include <stdint.h>

#include <kernel/defines.h>
#include <kernel/stdtypes.h>

#include <hil/fs/types.h>

/*
 * Get information about a file.
 *
 * path   File path
 * buf    User-supplied buffer
 *
 * Returns: true on success or false on error.
 */
bool fsStat(const char * restrict path, struct fsStat * restrict buf);

/*
 * Open a file for reading or appending.
 *
 * path   File path
 * mode   Open mode: FS_READ or FS_APPEND
 *
 * Returns: file handle on success or -1 on error.
 */
int8_t fsOpen(const char *path, fsMode_t mode);

/*
 * Read up to @count bytes from file.
 *
 * fd     File handle
 * buf    User-supplied buffer
 * count  Buffer size
 *
 * Returns: number of bytes read, 0 on EOF or -1 on error.
 */
ssize_t fsRead(int8_t fd, void *buf, size_t count);

/*
 * Get current read position in a file.
 *
 * fd     File handle
 *
 * Returns: file offset in bytes.
 */
fsOff_t fsTell(int8_t fd);

/*
 * Set current read position in a file. File must be open for reading.
 *
 * fd     File handle
 * pos    Position to seek to
 */
void fsSeek(int8_t fd, fsOff_t pos);

/*
 * Write up to @count bytes to file.
 *
 * fd     File handle
 * buf    User data
 * count  Buffer size
 *
 * Returns: number of bytes written or -1 on error.
 */
ssize_t fsWrite(int8_t fd, const void *buf, size_t count);

/*
 * Write any pending data to the storage device. File must be open for writing.
 *
 * fd     File handle
 *
 * Returns: true on success or false on error.
 */
bool fsFlush(int8_t fd);

/*
 * Flush pending data and deallocate @fd.
 *
 * fd     File handle
 *
 * Returns: true on success or false on error.
 */
bool fsClose(int8_t fd);

/*
 * Remove a file.
 *
 * path   File path
 *
 * Returns: true on success or false on error.
 */
bool fsRemove(const char *path);

/*
 * Rename a file.
 *
 * old    Current file name
 * new    New file name
 *
 * Returns: true on success or false on error.
 */
bool fsRename(const char *old, const char *new);

/*
 * Query last error.
 *
 * Returns: error number.
 */
fsError_t fsLastError(void);

#endif /* _FS_H_ */
