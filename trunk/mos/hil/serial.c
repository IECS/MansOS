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

#include <string.h>
#include <stdio.h>
#include <serial.h>

//===========================================================
// Data types and constants
//===========================================================

// user provided recv function callback
extern SerialCallback_t serialRecvCb[SERIAL_COUNT];

//===========================================================
// Procedures
//===========================================================

void serialSendString(uint8_t id, const char *s) {
    for (; *s; ++s) {
        serialSendByte(id, *s);
        if (*s == '\n') {
            // HACK: fix the newlines
            serialSendByte(id, '\r');
        }
    }
}

void serialSendData(uint8_t id, const uint8_t *data, uint16_t len) {
    const uint8_t *p;
    for (p = data; p < data + len; ++p) {
        serialSendByte(id, *p);
    }
}

uint_t serialSetReceiveHandle(uint8_t id, SerialCallback_t functionHandle) {
    if (id >= SERIAL_COUNT) return -1;
    serialRecvCb[id] = functionHandle;
    if (functionHandle) {
        // Enable Serial RX automatically
        serialEnableRX(id);
    }
    return 0;
}
