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

#include "dprint.h"
#include <radio.h>

//
// Hack: printInit() is both in this file and in dprint-serial.c
// to avoid discarding these files at link stage.
// This is just 6 byte overhead (code memory) by default,
// and significant savings if radio is not used.
//
void printInit(void)
{
    extern void printInitReal(void);
    printInitReal();
}

void radioPrint(const char* str)
{
#if 0
   if (!localMac) getSimpleMac()->init(NULL, false, NULL, 0);
   macSend(NULL, (uint8_t *) str, strlen(str) + 1);
#else
   // don't forget to call radioInit() somewhere!
   radioSend((uint8_t *) str, strlen(str) + 1);
   // mdelay(100); // wait a bit, to allow the radio to complete the sending
#endif
}

#if USE_NETWORK
void networkPrint(const char* str)
{
    static Socket_t socket;
    if (socket.port == 0) {
        socketOpen(&socket, NULL);
        socketBind(&socket, DPRINT_PORT);
        socketSetDstAddress(&socket, MOS_ADDR_ROOT);
    }
    socketSend(&socket, str, strlen(str) + 1);
}
#endif // USE_NETWORK
