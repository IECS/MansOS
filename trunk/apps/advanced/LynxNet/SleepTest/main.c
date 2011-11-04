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

#include <kernel/mansos.h>
#include <kernel/udelay.h>
#include <lib/dprint.h>
#include <lib/assert.h>
#include <hil/leds.h>
#include <hil/timers.h>
#include <hil/radio.h>
#include <hil/sleep.h>
#include <string.h>
#include <platform_hpl.h>

typedef struct Packet {
    // originator's ID
    uint16_t originator;
    // sensor data
    uint16_t light;
    uint16_t temperature;
    uint16_t activity;
    // GPS data
    uint32_t gpsLat;
    uint32_t gpsLon;
} Packet_t;

//
// Fill the packet with sensor data
//
bool readSensorData(Packet_t *packet)
{
    PRINTF("readSensorData\n");
    memset(packet, 0, sizeof(*packet));

    return true;
}

//
// do some MAC protol stuff for some (possible long) time
//
void lynxNetMacListen()
{
    PRINTF("macListen\n");

    radioOn();
    mdelay(500);
    radioOff();
}

void lynxNetRcvRadio(void)
{
    static uint8_t buffer[130];
    uint16_t len;
    len = radioRecv(buffer, sizeof(buffer));

    PRINTF("radioRecv, len=%u\n", len);
}


//
// send data using mac MAC protocol
//
void lynxNetMacSend(void *data, uint16_t dataSize)
{
    PRINTF("macSend\n");

    radioSend(data, dataSize);

    mdelay(10);
}

// --------------------------- appMain

void appMain(void)
{
    radioSetReceiveHandle(lynxNetRcvRadio);
    radioOff();

    for (;;) {
        PRINTF("time=%lu\n", getRealTime());
        lynxNetMacListen();

        Packet_t packet;
        if (readSensorData(&packet)) {
            lynxNetMacSend(&packet, sizeof(packet));
        }

        redLedOn();
        mdelay(500);
        redLedOff();

        sleep(2);
    }
}
