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

#include "stdmansos.h"
#include <net/radio_packet_buffer.h>

uint8_t data[] = "hello world";

void appMain(void)
{
    // turn on radio listening
    radioOn();

    for (;;) {
        PRINT("in appMain...\n");

        if (isRadioPacketReceived()) {
            PRINTF("got a packet from radio, size=%u, first bytes=0x%02x 0x%02x 0x%02x 0x%02x\n",
                    radioPacketBuffer->receivedLength,
                    radioPacketBuffer->buffer[0],
                    radioPacketBuffer->buffer[1],
                    radioPacketBuffer->buffer[2],
                    radioPacketBuffer->buffer[3]);
        }
        else if (isRadioPacketError()) {
            PRINTF("got an error from radio: %s\n",
                    strerror(-radioPacketBuffer->receivedLength));
        }
        radioBufferReset();

        // send out own packet
        radioSend(data, sizeof(data));

        redLedToggle();
        mdelay(1000);
    }
}
