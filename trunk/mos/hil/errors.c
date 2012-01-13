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

#include "errors.h"
#include <stdio.h>

#ifndef PLATFORM_PC

int_t errno;

char *strerror(int_t errno)
{
    switch (errno) {
    case 0: return "Success";
    case EPERM: return "Operation not permitted";
    case ENOENT: return "No such entity";
    case EIO: return "Input/output error";
    case ENOMEM: return "Not enough memory";
    case EACCES: return "Permission denied";
    case EFAULT: return "Bad address";
    case EBUSY: return "Device busy";
    case EEXIST: return "File exists";
    case EINVAL: return "Invalid argument";
    case ENOSPC: return "No space left on device";
    case EAGAIN: return "Resource temporarily unavailable";
    case EMSGSIZE: return "Message too long";
    case EOPNOTSUPP: return "Operation not supported";
    case ENOSYS: return "Function not implemented";
    case EBADMSG: return "Bad message";
    case EMULTIHOP: return "Multihop attempted";
    case ENOLINK: return "No link";
    case EPROTO: return "Protocol error";
    default: {
        static char buffer[4];
        sprintf(buffer, "%d", errno);
        return buffer;
    }
    }
}

#endif // PLATFORM_PC
