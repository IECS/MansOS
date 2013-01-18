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

#include "stdmansos.h"
#include <net/mac.h>
#include <net/net-stats.h>

uint8_t sendBuffer[100] = "X hello world";

static void macRecv(MacInfo_t *mi, uint8_t *data, uint16_t len)
{
    PRINTF("got %d bytes from 0x%04x (0x%02x) \n",
            len, mi->originalSrc.shortAddr, *data);
    redLedToggle();
}

void appMain(void)
{
    macProtocol.recvCb = macRecv;

    MosAddr dst;
    uint8_t counter = 0,i;
    uint16_t neighbor;
    // XXX: hardcoded addresses
    if (localAddress == 0x0236) {
        intToAddr(dst, 0x5b5a);
    } else {
        intToAddr(dst, 0x0236);
    }
    neighbor = addNeighbor(dst.shortAddr);
    while (true) {
        for (i=0;i<10;i++) {
            sendBuffer[0] = '0' + counter++;
            if (counter >= 10) counter = 0;

            int8_t result = macSend(&dst, sendBuffer, sizeof(sendBuffer));
            if (result < 0) {
                PRINTF("mac send failed: %s\n", strerror(-result));
            } else if (result < sizeof(sendBuffer)) {
                PRINTF("mac send: not all sent! (%u vs %u)\n", result, sizeof(sendBuffer));
            }
           mdelay(1000);
        }
        PRINTF("sent:%d akcepted:%d => %d%%\t",
                linq[neighbor].sent, linq[neighbor].sentAck,
                (100*linq[neighbor].sentAck / linq[neighbor].sent));
        PRINTF("recv:%d akcepted:%d => %d%%\n",
                linq[neighbor].recv, linq[neighbor].recvAck,
                (100*linq[neighbor].recvAck / linq[neighbor].recv));
    }
}
