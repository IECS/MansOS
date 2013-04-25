/*
 * Copyright (c) 2013 the MansOS team. All rights reserved.
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

#ifndef MANSOS_TRACE_H
#define MANSOS_TRACE_H

/// \file
/// Tracing support
///

#include <print.h>

#define TRACE_BASE(fmt, ...)                                              \
    PRINTF("%s:%d: " fmt "\n", __func__, __LINE__, __VA_ARGS__)

#ifdef USE_TRACE

#define TRACE_MSG(s)      TRACE_BASE("%s", (s))
#define TRACE_ENTER()     TRACE_MSG("enter")
#define TRACE_LEAVE()     TRACE_MSG("leave")
#define TRACE(x)          TRACE_BASE(#x " = %ld", (long int)(x))
#define TRACE2(x, y)                                                      \
    TRACE_BASE(#x ", " #y " = %ld, %ld", (long int)(x), (long int)(y))
#define TRACE3(x, y, z)                                                   \
    TRACE_BASE(#x ", " #y ", " #z " = %ld, %ld, %ld", (long int)(x),      \
               (long int)(y), (long int)(z))
#define TRACEF(fmt, ...)  TRACE_BASE(#__VA_ARGS__ " = " fmt, __VA_ARGS__)

#else // USE_TRACE not defined

#define TRACE_MSG(s)      ((void)0)
#define TRACE_ENTER()     ((void)0)
#define TRACE_LEAVE()     ((void)0)
#define TRACE(x)          ((void)0)
#define TRACE2(x, y)      ((void)0)
#define TRACE3(x, y, z)   ((void)0)
#define TRACEF(fmt, ...)  ((void)0)

#endif // USE_TRACE

#endif // MANSOS_TRACE_H
