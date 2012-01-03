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
#include <kernel/threads/radio.h>

uint8_t data[] = "hello world";

void appMain(void)
{
    // declare our packet buffer
    RADIO_PACKET_BUFFER(radioBuffer, RADIO_MAX_PACKET);

    // turn on radio listening
    radioOn();

    for (;;) {
        PRINT("in appMain...\n");

        if (isRadioPacketReceived(radioBuffer)) {
            PRINTF("got a packet from radio, size=%u, first bytes=0x%02x 0x%02x 0x%02x 0x%02x\n",
                    radioBuffer.receivedLength, radioBuffer.buffer[0], radioBuffer.buffer[1],
                    radioBuffer.buffer[2], radioBuffer.buffer[3]);
        }
        else if (isRadioPacketError(radioBuffer)) {
            PRINTF("got an error from radio: %s\n",
                    strerror(-radioBuffer.receivedLength));
        }
        radioBufferReset(radioBuffer);

        // send out own packet
        radioSend(data, sizeof(data));

        redLedToggle();
        mdelay(1000);
    }
}
