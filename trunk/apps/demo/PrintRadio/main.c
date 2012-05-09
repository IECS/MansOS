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

// --------------------------------------------
// PrintRadio: forwards packets between radio and serial interfaces
// --------------------------------------------

#include "stdmansos.h"

static uint8_t radioBuffer[RADIO_MAX_PACKET];
static char serialBuffer[255];

void recvRadio(void)
{
    int16_t len;

    greenLedToggle();
    len = radioRecv(radioBuffer, sizeof(radioBuffer));
    if (len < 0) {
        PRINT("radio receive failed\n");
    }
    else if (len > 0 ) {
        PRINTF("radio receive %d bytes\n", len);
        debugHexdump((uint8_t *) radioBuffer, len);
    }
}

void recvSerial(uint8_t length) {
    PRINT(serialBuffer);
    if (serialBuffer[length - 1] != '\n') {
        // print newline as well
        PRINT("\n");
    }
    radioSend(serialBuffer, length);
}

void appMain(void)
{
    USARTSetPacketReceiveHandle(PRINTF_USART_ID, recvSerial,
            (uint8_t *)serialBuffer, sizeof(serialBuffer));

    radioSetReceiveHandle(recvRadio);
    radioOn();

    PRINT("Forwarder started...\n");

    while (1) {
        radioSend("hello world", sizeof("hello world"));
        redLedToggle();
        mdelay(1000);
    }
}
