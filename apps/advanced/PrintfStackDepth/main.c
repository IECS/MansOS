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

//------------------------------------------------
// This program demonstrates the (excessive) stack depth used by
// msp430 libC printf() functions.
// For gcc version 4.5.3 (GNU GCC patched mspgcc-20110716))
// stack usage is 62 bytes + storage room for each argument argcount
// (as args are pushed onto stack).
// Note that by default MansOS uses vsnprintf() instead of vprintf(),
// so even greater stack depth is likely to be used!
//------------------------------------------------

#include "stdmansos.h"

MemoryAddress_t spInMain;
MemoryAddress_t spInPutchar;

int putchar(int c) {
    if (!spInPutchar) {
        GET_SP(spInPutchar);
    }
    serialSendByte(PRINTF_SERIAL_ID, c);
    return c;
}

void appMain(void)
{
    GET_SP(spInMain);

    // this uses 62 bytes on GCC 4.5.3
    //PRINTF("hello world\r\n");

    // 
    // This uses:
    //    70 bytes (62 + 2 + 2 + 4) on GCC 4.5.3 (GNU GCC patched mspgcc-20110716))
    //    110 bytes on GCC version 4.6.3 20120301 
    PRINTF("Hello %s! Running MansOS build %s, uptime is %lu seconds\r\n",
            "world",
            __DATE__,
            getTimeSec());

    PRINTF("stack pointer in main: %#x\r\n", spInMain);
    PRINTF("stack pointer in putchar: %#x\r\n", spInPutchar);
    PRINTF("difference: %d\r\n", spInMain - spInPutchar);
}
