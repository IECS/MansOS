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

#include "stdmansos.h"
#include "socket.h"
#include "config.h"

//-----------------
// constants
//-----------------
enum {
    SLEEP_TIME_MS = 1000,
    COUNTER_PORT = 123
};

// receive counter and display it on leds
void listenForCounter() {
    uint8_t receivedCounter;
    MosSocket_t *socket;
    int16_t ret;

    ret = socketCreate(&socket, &receivedCounter, sizeof(receivedCounter));
    if (ret) return;

    socketBind(socket, COUNTER_PORT);

    PRINTF("listening for counter on port %d\n", COUNTER_PORT);

    while (1) {
        ret = socketRecv(socket);
        PRINTF("received counter %u from 0x%04x\n",
                receivedCounter, socket->recvSrcAddr.shortAddr);
        setLeds(receivedCounter);
    }
}

void sendCounter() {
    uint8_t counter = 0;
    MosSocket_t *socket;
    int16_t ret;

    greenLedOn();

    ret = socketCreate(&socket, NULL, 0);
    if (ret) {
        PRINTLN("socketCreate failed!");
        return;
    }

    while(1) {
        PRINTF("sending counter %i\n", counter);

        socketSend(socket, NULL, COUNTER_PORT, &counter, sizeof(counter));

        threadSleep(SLEEP_TIME_MS);
        ++counter;
    }
}

void appMain(void)
{
    PRINT_INIT(128);

    // custom config for now...
    /* config.macTdmaSuperfameInterval = 3*1000; */
    /* config.macTdmaIsMaster = CFG_MAC_TDMA_IS_MASTER; */
    /* config.macTdmaIsBaseStation = CFG_MAC_TDMA_IS_BASE_STATION; */
    /* config.macTdmaSlaveNumber = CFG_MAC_TDMA_SLAVE_NUMBER; */
    /* config.systemLocation = CFG_SYSTEM_LOCATION; */

    initSockets(MAC_GLOBAL_TDMA, CFG_MAC_TDMA_IS_BASE_STATION ? 0 : MAC_TDMA);

    threadSleep(1000);

    if (CFG_MAC_TDMA_IS_BASE_STATION) {
        // set correct local address
        localAddress = CFG_BASE_STATION_ADDR;

        sendCounter();
    } else if (CFG_MAC_TDMA_IS_MASTER) {
        // set local address from location
        localAddress = MAKE_U16(CFG_SYSTEM_LOCATION, 0xff);
    } else {
        // set local address from location and slave number
        localAddress = MAKE_U16(CFG_SYSTEM_LOCATION, CFG_MAC_TDMA_SLAVE_NUMBER);

        listenForCounter();
    }
}
