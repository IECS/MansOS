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
//  Serial port echo application.
//  Reads input from serial port (terminate with newline!),
//  echoes it back to serial port.
//-------------------------------------------

#include "stdmansos.h"

#define PROMPT "$ "

void serialReceive(uint8_t byte) {
    static uint16_t numBytesReceived;
    static char buffer[255];
    static bool hadCr = false;

    // store the byte in a buffer
    buffer[numBytesReceived++] = byte;

    if (byte == '\r') hadCr = true;

    if (byte == '\n' || numBytesReceived == sizeof(buffer) - 1) {
        buffer[numBytesReceived] = 0;
        PRINTF("%s", buffer);
        if (!hadCr) serialSendByte(PRINTF_SERIAL_ID, '\r');
	if (byte != '\n') serialSendByte(PRINTF_SERIAL_ID, '\n');
        numBytesReceived = 0; // reset counter
        hadCr = false;
        PRINTF(PROMPT);
    }
}

void appMain(void)
{
    serialSetReceiveHandle(PRINTF_SERIAL_ID, serialReceive);

    PRINTF("# Type your text, press [Enter], and the mote will echo it back\n");
    PRINTF(PROMPT);
}
