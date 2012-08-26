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
extern USARTCallback_t usartRecvCb[USART_COUNT];

//===========================================================
// Procedures
//===========================================================

void USARTSendString(uint8_t id, char *s) {
    for (; *s; ++s) {
        USARTSendByte(id, *s);
        if (*s == '\n') {
            // HACK: fix the newlines
            USARTSendByte(id, '\r');
        }
    }
}

void USARTSendData(uint8_t id, uint8_t *data, uint16_t len) {
    uint8_t *p;
    for (p = data; p < data + len; ++p) {
        USARTSendByte(id, *p);
    }
}

uint_t USARTSetReceiveHandle(uint8_t id, USARTCallback_t functionHandle) {
    if (id >= USART_COUNT) return -1;
    usartRecvCb[id] = functionHandle;
    if (functionHandle) {
        // Enable USART RX automatically
        USARTEnableRX(id);
    }
    return 0;
}
