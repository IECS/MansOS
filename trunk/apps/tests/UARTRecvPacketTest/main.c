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

//------------------------------------------------------------
// USART Send & Receive demo:
// * Send counter every second
// * Receive data packets, get first byte as an ASCII digit,
//   set it on leds and send back
//------------------------------------------------------------

#include "stdmansos.h"
#include "usart.h"
#include "dprint.h"
#include "udelay.h"

/**
 * Our callback
 */
void usartPacketReceived(uint8_t bytes);

/**
 * Buffer size, in bytes
 */
enum {
    BUF_SIZE = 10
};

/**
 * Buffer, in which the received data will be stored
 */
static uint8_t buf[BUF_SIZE];


//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    PRINT_INIT(128);
    // Set packet reception handler (callback function)
    USARTSetPacketReceiveHandle(1, usartPacketReceived, buf, BUF_SIZE);
    ledOff();

    // Send Ping every second
    static uint_t counter = 1;
    while (1) {
        PRINTF("Ping #%i\n", counter++);
        mdelay(1000);
    }
}

void usartPacketReceived(uint8_t bytes) {
    // get ASCII digit from the first byte
    uint8_t c = buf[0] - '0';

    ledsSet(c);
    PRINTF("Pong %i %i\n", c, bytes);
}
