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
 */

#include "stdmansos.h"
#include <net/socket.h>
#include <net/routing.h>

//-----------------
// constants
//-----------------
enum {
    SLEEP_TIME_MS = 5000,
    COUNTER_PORT  = 123,
};

static void recvData(Socket_t *socket, uint8_t *data, uint16_t len)
{
    PRINTF("got %d bytes from 0x%04x (0x%02x) \n",
            len, socket->recvMacInfo->originalSrc.shortAddr, *data);
    redLedToggle();
}

static void sendData(Socket_t *socket)
{
    uint16_t counter = 0;
    for (;;) {
        PRINTF("sending counter %i\n", counter);
        if (socketSend(socket, &counter, sizeof(counter))) {
            PRINT("socketSend failed\n");
        }
        mdelay(SLEEP_TIME_MS);
        ++counter;
    }
}

void appMain(void)
{
    Socket_t socket;
    socketOpen(&socket, recvData);
    socketBind(&socket, COUNTER_PORT);
    socketSetDstAddress(&socket, MOS_ADDR_ROOT);

#if BASE_STATION
    (void) sendData;
    for (;;) {
        // PRINT("in app main\n");
        redLedToggle();
        mdelay(SLEEP_TIME_MS);
    }
#else
    sendData(&socket);
#endif
}
