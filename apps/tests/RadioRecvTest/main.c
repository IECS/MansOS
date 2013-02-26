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

//
// This application demonstrates raw radio packet
// reception when thread support is enabled.
//
// Warning: do not use this for real-world applications!
// The delay is going to be very high due to explicit sleeping for 1 second.
//

#include "stdmansos.h"
#include "net/radio_packet_buffer.h"

// define the size of a radio packet buffer
#ifndef RADIO_BUFFER_SIZE
#define RADIO_BUFFER_SIZE RADIO_MAX_PACKET
#endif

// define our buffer structure
static struct RadioPacketBufferReal_s {
    uint8_t bufferLength;     // length of the buffer
    int8_t receivedLength;    // length of data stored in the packet, or error code if negative
    uint8_t buffer[RADIO_BUFFER_SIZE]; // a buffer where the packet is stored
} realBuf = {RADIO_BUFFER_SIZE, 0, {0}};

extern RadioPacketBuffer_t *radioPacketBuffer;

// Application entry point
void appMain(void)
{
    // setup our radio packet buffer
    radioPacketBuffer = (RadioPacketBuffer_t *) &realBuf;
    // turn on radio listening
    radioOn();

    for (;;) {
        if (isRadioPacketError()) {
            // error occured last time the packet was received
            PRINTF("radio receive failed, error code %u\n",
                    (uint16_t) -radioPacketBuffer->receivedLength);
        }
        else if (isRadioPacketReceived()) {
            // a packet is received from the radio
            PRINTF("received %u byte packet\n", (uint16_t) radioPacketBuffer->receivedLength);
            // dump the contents of the buffer
            debugHexdump(radioPacketBuffer->buffer, radioPacketBuffer->receivedLength);
        }
        // reset our radio buffer in any case
        radioBufferReset();

        // do nothing for a while
        msleep(1000);
    }
}
