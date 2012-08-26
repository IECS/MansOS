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

#ifndef MANSOS_THREADS_RADIO_H
#define MANSOS_THREADS_RADIO_H

#include <radio.h>

typedef struct RadioPacketBuffer_s {
    uint8_t bufferLength;     // length of the buffer
    int8_t receivedLength;    // length of data stored in the packet, or error code if negative
    uint8_t buffer[0];        // pointer to a buffer where the packet is stored
} RadioPacketBuffer_t;

extern RadioPacketBuffer_t *radioPacketBuffer;

// ----------------------------------------------------------------
// Kernel API
// ----------------------------------------------------------------

void radioProcess(void);

// ----------------------------------------------------------------
// User API
// ----------------------------------------------------------------

#define RADIO_PACKET_BUFFER(name, size)                                 \
    static struct RadioPacketBuffer_s_##name {                          \
        uint8_t bufferLength;                                           \
        int8_t receivedLength;                                          \
        uint8_t buffer[size];                                           \
    } name = {size, 0, {0}};                                            \
    radioPacketBuffer = (RadioPacketBuffer_t *) &name;

#define isRadioPacketEmpty(radioBuffer)         \
    ((radioBuffer).receivedLength == 0)

#define isRadioPacketReceived(radioBuffer)      \
    ((radioBuffer).receivedLength > 0)

#define isRadioPacketError(radioBuffer)         \
    ((radioBuffer).receivedLength < 0)

#define radioBufferReset(radioBuffer)           \
    (radioBuffer).receivedLength = 0;

#endif
