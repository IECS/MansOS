/*
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
 * fs/prefix.c -- file subsystem management
 */

#include <stddef.h>
#include <string.h>

#include <kernel/defines.h>

#include "common.h"
#include "prefix.h"
#include "subsystems.h"
#include "types.h"

/* One entry in the prefix table */
struct fsPrefixEntry {
    const char * restrict                 prefix;
    const struct fsOperations * restrict  ops;
    void                                (*init)(void);
};

#define FS_SUBSYSTEM(Prefix, Ops, Init) \
    { .prefix = Prefix, .ops = Ops, .init = Init },
static const struct fsPrefixEntry prefixEntries[] = { FS_SUBSYSTEM_LIST };

/* Initialize subsystems */
void fsInitSubsystems(void)
{
    size_t i;

    for (i = 0; i < sizeof(prefixEntries) / sizeof(struct fsPrefixEntry); i++)
        prefixEntries[i].init();
}

/* Find entry in the prefix table */
static const struct fsPrefixEntry *fsFindPrefixEntry(const char *path)
{
    size_t i;

    for (i = 0; i < sizeof(prefixEntries) / sizeof(struct fsPrefixEntry); i++)
    {
        size_t prefixlen = strlen(prefixEntries[i].prefix);

        if (strlen(path) > prefixlen && !strncmp(path, prefixEntries[i].prefix,
            prefixlen))
        {
            return prefixEntries + i;
        }
    }

    fsSetError(FS_ERR_NOENT);
    return NULL;
}

/* Find entry in the prefix table, strip prefix from @path */
const struct fsOperations *fsPrefixLookup(const char * restrict *path)
{
    const struct fsPrefixEntry *entry = fsFindPrefixEntry(*path);

    if (!entry)
        return NULL;
    else
    {
        *path += strlen(entry->prefix);
        return entry->ops;
    }
}
