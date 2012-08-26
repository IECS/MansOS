/**
 * Copyright (c) 2008-2011 the MansOS team. All rights reserved.
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
 *
 * fs/prefix.h -- file subsystem management
 */

#ifndef _FS_PREFIX_H_
#define _FS_PREFIX_H_

#include <stdbool.h>

#include <kernel/defines.h>
#include <kernel/stdtypes.h>

#include "types.h"

/*
 * File operations that a file subsystem has to implement.
 *
 * Open files are identified by a unique data pointer associated with them,
 * which will typically point to some file control block structure. The core
 * takes care of mapping file handles to an operations/unique pointer pair.
 * NULL return value will be treated as an error by the upper layer.
 */
struct fsOperations {
    bool     (*stat)(const char * restrict path, struct fsStat * restrict buf);
    void    *(*open)(const char *path, fsMode_t mode);
    ssize_t  (*read)(void * restrict id, void * restrict buf, size_t count);
    fsOff_t  (*tell)(void *id);
    void     (*seek)(void *id, fsOff_t pos);
    ssize_t  (*write)(void * restrict id, const void * restrict buf,
                      size_t count);
    bool     (*flush)(void *id);
    bool     (*close)(void *id);
    bool     (*remove)(const char *path);
    bool     (*rename)(const char *old, const char *new);
};

/* Initialize subsystems */
void fsInitSubsystems(void);

/* Find entry in the prefix table, strip prefix from @path */
const struct fsOperations *fsPrefixLookup(const char * restrict *path);

#endif /* _FS_MOUNT_H_ */
