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
 *
 * fs/dev/null.c -- null device file
 */

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include <kernel/stdtypes.h>

#include "dev.h"
#include "null.h"

/* File operations */
static bool    nullOpen(void);
static ssize_t nullRead(void *buf, size_t count);
static ssize_t nullWrite(const void *buf, size_t count);
static bool    nullClose(void);

const struct fsDevOperations fsDevNullOps = {
    .open  = nullOpen,
    .read  = nullRead,
    .write = nullWrite,
    .close = nullClose
};

/* The functions */

static bool nullOpen(void)
{
    return true;
}

static ssize_t nullRead(void *buf, size_t count)
{
    memset(buf, 0, count);

    return count;
}

static ssize_t nullWrite(const void *buf, size_t count)
{
    (void)buf;

    return count;
}

static bool nullClose(void)
{
    return true;
}
