/*
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

//-----------------------------------------------------------------------------
//  Counter exchange application
//  Mote broadcasts its counter which gets incremented every second.
//  When receiving a counter value, mote displays it on leds.
//-----------------------------------------------------------------------------

#include "stdmansos.h"
#include <random.h>
#include "amb8420/amb8420.h"
#include <string.h>

#define RECV 1

void sendCounter(void);
void recvCounter(void);
//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    // ------------------------- serial number
    PRINTF("%s %#04x starting...\n",
            RECV ? "Receiver" : "Sender", localAddress);

    uint8_t *p = malloc(2);
    p[0] = 1;
    p[1] = 0;
#if RECV
    radioSetReceiveHandle(recvCounter);
    //radioSetReceiveHandle(usartReceive);
    radioOn();

    AMB8420_SWITCH_TO_COMMAND_MODE();
    PRINTF("\nChanged mode...\n");

    //GET WOR timeout
    amb8420Get(66,2);
    //SET WOR timeout to 1s sleep(1s recv)
    amb8420Set(2,p,66);
    //GET WOR timeout
    amb8420Get(66,2);
    //Back to transparent mode & enable WOR
    AMB8420_SWITCH_TO_TRANSPARENT_MODE();
    /* Note: Not all messages will be recieved!*/
    AMB8420_ENTER_WOR_MODE();

#else
//  radioSetReceiveHandle(radioDiscard);
//  radioOn();
    sendCounter();
#endif
}

// receive counter and display it on leds
void recvCounter(void)
{
    static uint8_t buffer[128];
    int16_t len;

    redLedToggle();
    len = radioRecv(buffer, sizeof(buffer));
    if (len < 0) {
        PRINTF("radio recv failed\n");
        return;
    }
    if (len == 0) {
        PRINTF("0 len!\n");
    }
    //debugHexdump(buffer, len);
    if (len > 0) {
        PRINTF("Received %d bytes, counter = %d\n", len, buffer[0]);
    }
}

void sendCounter(void) {
    // static uint8_t sendBuffer[120];
    static uint8_t sendBuffer[20] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
        10, 11, 12, 13, 14, 15, 16, 17, 18, 19
    };
    uint8_t *counter = &sendBuffer[0];
    // memcpy(sendBuffer + 1, &localAddress, 2);
    while (1) {
        PRINTF("0x%04x: sending counter %i\n", localAddress, *counter);
        redLedToggle();
        radioSend(sendBuffer, sizeof(sendBuffer));
        mdelay(2000);
        ++(*counter);
    }
}
