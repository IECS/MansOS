/*
 * Copyright (c) 2008-2013 the MansOS team. All rights reserved.
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

#ifndef MANSOS_ERRNO_H
#define MANSOS_ERRNO_H

/// \file
/// POSIX-compatible error codes
///

#include <stdtypes.h>

#ifdef PLATFORM_PC

// use error code definitions from standard headers
#include <errno.h>
#include <string.h>

#else

//===========================================================
// Data types and constants
//===========================================================

#define EPERM           1               /* Operation not permitted */
#define ENOENT          2               /* No such file or directory */
#define ESRCH           3               /* No such process */
#define EINTR           4               /* Interrupted system call */
#define EIO             5               /* Input/output error */
#define ENXIO           6               /* Device not configured */
#define E2BIG           7               /* Argument list too long */
#define ENOEXEC         8               /* Exec format error */
#define EBADF           9               /* Bad file descriptor */
#define ECHILD          10              /* No child processes */
#define EDEADLK         11              /* Resource deadlock avoided */
#define ENOMEM          12              /* Cannot allocate memory */
#define EACCES          13              /* Permission denied */
#define EFAULT          14              /* Bad address */
#define ENOTBLK         15              /* Block device required */
#define EBUSY           16              /* Device busy */
#define EEXIST          17              /* File exists */
#define EXDEV           18              /* Cross-device link */
#define ENODEV          19              /* Operation not supported by device */
#define ENOTDIR         20              /* Not a directory */
#define EISDIR          21              /* Is a directory */
#define EINVAL          22              /* Invalid argument */
#define ENFILE          23              /* Too many open files in system */
#define EMFILE          24              /* Too many open files */
#define ENOTTY          25              /* Inappropriate ioctl for device */
#define EFBIG           27              /* File too large */
#define ENOSPC          28              /* No space left on device */
#define ESPIPE          29              /* Illegal seek */
#define EROFS           30              /* Read-only filesystem */
#define EMLINK          31              /* Too many links */
#define EPIPE           32              /* Broken pipe */

/* math software */
#define EDOM            33              /* Numerical argument out of domain */
#define ERANGE          34              /* Result too large */

/* non-blocking and interrupt i/o */
#define EAGAIN          35              /* Resource temporarily unavailable */
#define EWOULDBLOCK     EAGAIN          /* Operation would block */
#define EINPROGRESS     36              /* Operation now in progress */
#define EALREADY        37              /* Operation already in progress */

/* ipc/network software -- argument errors */
#define ENOTSOCK        38              /* Socket operation on non-socket */
#define EDESTADDRREQ    39              /* Destination address required */
#define EMSGSIZE        40              /* Message too long */
#define EPROTOTYPE      41              /* Protocol wrong type for socket */
#define ENOPROTOOPT     42              /* Protocol not available */
#define EPROTONOSUPPORT 43              /* Protocol not supported */
#define ESOCKTNOSUPPORT 44              /* Socket type not supported */
#define EOPNOTSUPP      45              /* Operation not supported */
#define ENOTSUP         EOPNOTSUPP      /* Operation not supported */
#define EPFNOSUPPORT    46              /* Protocol family not supported */
#define EAFNOSUPPORT    47              /* Address family not supported by protocol family */
#define EADDRINUSE      48              /* Address already in use */
#define EADDRNOTAVAIL   49              /* Can't assign requested address */

/* ipc/network software -- operational errors */
#define ENETDOWN        50              /* Network is down */
#define ENETUNREACH     51              /* Network is unreachable */
#define ENETRESET       52              /* Network dropped connection on reset */
#define ECONNABORTED    53              /* Software caused connection abort */
#define ECONNRESET      54              /* Connection reset by peer */
#define ENOBUFS         55              /* No buffer space available */
#define EISCONN         56              /* Socket is already connected */
#define ENOTCONN        57              /* Socket is not connected */
#define ESHUTDOWN       58              /* Can't send after socket shutdown */
#define ETOOMANYREFS    59              /* Too many references: can't splice */
#define ETIMEDOUT       60              /* Operation timed out */
#define ECONNREFUSED    61              /* Connection refused */

#define ELOOP           62              /* Too many levels of symbolic links */
#define ENAMETOOLONG    63              /* File name too long */

/* should be rearranged */
#define EHOSTDOWN       64              /* Host is down */
#define EHOSTUNREACH    65              /* No route to host */
#define ENOTEMPTY       66              /* Directory not empty */

#define ENOLCK          77              /* No locks available */
#define ENOSYS          78              /* Function not implemented */

#define EBADMSG         89              /* Bad message */
#define EMULTIHOP       90              /* Multihop attempted */
#define ENOLINK         91              /* Link has been severed */
#define EPROTO          92              /* Protocol error */

#define ENOTCAPABLE     93              /* Capabilities insufficient */
#define ECAPMODE        94              /* Not permitted in capability mode */

//===========================================================
// Procedures
//===========================================================

//! Get ASCII string description of a specific error
char *strerror(int_t errno);

//===========================================================
// Variables
//===========================================================

//! The number of the last error, set by library functions
extern int_t errno;

#endif // not PLATFORM_PC

#endif
