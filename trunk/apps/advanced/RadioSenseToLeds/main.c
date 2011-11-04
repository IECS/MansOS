/**
 * Copyright (c) 2008-2010 Leo Selavo and the contributors. All rights reserved.
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
//  Sensor value exchange application
//  Mote reads light, broadcasts it
//  When sensed value received over radio, mote displays it on leds
//-----------------------------------------------------------------------------

#include "mansos.h"
#include "leds.h"
#include "light.h"
#include "comm.h"
#include "dprint.h"
#include "socket.h"
#include "scheduler.h"
#include "byteorder.h"

//-----------------
// constants
//-----------------
enum {
    SLEEP_TIME_MS = 1000, // sense and send every second
    SENSE_PORT = 45 // use "port" 145 (like AMType in TinyOS)
};

//-----------------
// two threads: send and receive
//-----------------
static void senseAndSend(void);
static void listenForData(void);

//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    PRINT_INIT(128); // for debug output to serial
    initSockets(MAC_SIMPLE, 0); // use simple CSMA-type MAC
    defaultThreadCreate(listenForData); // spawn thread for listening
    senseAndSend(); // call sending loop
}

// receive light reading and display it on leds
static void listenForData(void) {
    uint16_t receivedLight;
    MosSocket_t *socket;
    int16_t ret;

    ret = socketCreate(&socket, (uint8_t *) &receivedLight, sizeof(receivedLight));
    if (ret) {
        PRINTLN("failed to create socket");
        return;
    }

    socketBind(socket, SENSE_PORT);

    while (1) {
        ret = socketRecv(socket);
        if (ret < 0) {
            PRINTLN("socketRecv failed");
            continue;
        }
        if (socket->recvBuffer.length == 0) {
            PRINTLN("socketRecv: no data");
            continue;
        }
        if (socket->recvBuffer.length < sizeof(receivedLight)) {
            PRINTLN("socketRecv: too short");
            continue;
        }

        receivedLight = ntohs(receivedLight);
        PRINTF("received sensor value %u from 0x%04x\n",
                receivedLight, socket->recvSrcAddr.shortAddr);
        setLeds(receivedLight);
    }
}

// periodically read light and broadcast it over radio
static void senseAndSend(void) {
    MosSocket_t *socket;
    int16_t ret;

    ret = socketCreate(&socket, NULL, 0);
    if (ret) {
        PRINTLN("failed to create socket");
        return;
    }

    while(1)
    {
        uint16_t light = readLight();
        PRINTF("sending value %u\n", light);
        light = htons(light);
        ret = socketSend(socket, NULL, SENSE_PORT,
                         (uint8_t *) &light, sizeof(light));
        if (ret) {
            PRINTLN("socketSend failed");
        }
        threadSleep(SLEEP_TIME_MS);
    }
}

