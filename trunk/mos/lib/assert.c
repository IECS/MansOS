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
 * lib/assert.c -- handle program logic errors
 */

#include <stdlib.h> /* For abort() */

#include <kernel/defines.h>
#include <lib/dprint.h>
#include <hil/leds.h>

#include "assert.h"

#ifdef PLATFORM_PC
#  define ASSERT_PRINT printf
#else
#  ifdef USE_PRINT
#    define ASSERT_PRINT debugPrintf
#  else
#    define ASSERT_PRINT(...)
#  endif /* USE_PRINT */
#endif /* PLATFORM_PC */

void panic(void)
{
#ifdef PLATFORM_PC
    abort();
#else
    //make LEDs blink like linux kernel on panic
    uint16_t x = 0;
    DISABLE_INTS();
    for (;;)
    {
        ledsSet(x ^= ~0u);
        volatile uint32_t t = 100000;
        while (--t);
    }
#endif
}

void assertionFailed(const char * restrict msg, const char * restrict file,
                     int line)
{
    ASSERT_PRINT("ASSERTION FAILED:\r\n%s:%d: %s\r\n", file, line, msg);

    panic();
}
