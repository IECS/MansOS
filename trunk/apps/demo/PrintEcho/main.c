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

//-------------------------------------------
//  Serial port echo applicatio.
//  Reads input from serial port (terminate with newline!),
//  echoes it back to serial port.
//-------------------------------------------

#include "stdmansos.h"

#define PROMPT "$ "

void serialReceive(uint8_t byte) {
    static uint16_t bytesReceived;
    static char buffer[255];

#if !PLATFORM_PC
    // echo the byte back
    serialSendByte(PRINTF_SERIAL_ID, byte);
#endif

    // store it in the buffer
    buffer[bytesReceived++] = byte;

    if (byte == '\n' || byte == '\r' || bytesReceived == sizeof(buffer) - 1) {
        buffer[bytesReceived] = 0;
        if (byte == '\r') serialSendByte(PRINTF_SERIAL_ID, '\n');
        PRINTF("%s", buffer);
        if (byte != '\n') {
            // print newline as well
            PRINTF("\n");
        }
        bytesReceived = 0; // reset reception
        PRINTF(PROMPT);
    }
}

void appMain(void)
{
    serialSetReceiveHandle(PRINTF_SERIAL_ID, serialReceive);

    PRINTF("Type your text here, press enter, and the mote will echo it back\n");
    PRINTF(PROMPT);
}
