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

#include <serial.h>

#include <lib/dprint.h>

//===========================================================
// Data types and constants
//===========================================================
static uint8_t *packetBuffer = NULL;
static uint16_t bytesReceived = 0;
static uint16_t bufferSize = 0;
static SerialCallback_t packetHandler;

//===========================================================
// Procedures
//===========================================================

static void recvByte(uint8_t byte)
{
    if (packetBuffer) {
        // store byte
        packetBuffer[bytesReceived] = byte;
        ++bytesReceived;
        if (byte == '\n' || byte == '\r' || bytesReceived == bufferSize) {
            packetBuffer[bytesReceived] = 0;
            packetHandler(bytesReceived);
            bytesReceived = 0; // reset reception
        }
    }
}


uint_t serialSetPacketReceiveHandle(uint8_t id, SerialCallback_t cb,
                                    void *buffer, uint16_t len)
{
    if (id >= SERIAL_COUNT) return -1;
    if (!cb || !buffer) {
        // stop USART reception
        serialDisableRX(id);
        packetBuffer = NULL;
        return -1;
    }
    serialEnableRX(id);
    packetBuffer = (uint8_t *) buffer;
    bufferSize = len - 1;
    packetHandler = cb;
    bytesReceived = 0;
    serialSetReceiveHandle(id, recvByte);

    return 0;
}
