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
// SAD Sensor application - reads sensors every k seconds, 
// broadcasts measured values and voltage every m seconds
//-----------------------------------------------------------------------------

#define MOTE_NR  4

#include "mansos.h"
#include "adc.h"
#include "radio.h"
#include "leds.h"
#include "addr.h"
#include "timers.h"
#include "../sense.h"
#include "udelay.h"
#include <lib/dprint.h>
#include <string.h>

Measurement_t buffer[MAX_RECORDS];
uint16_t currRecord;

void mdelay(uint16_t ms) {
    uint32_t end = getRealTime() + ms;
    while (timeAfter(end, getRealTime())) {
        busyWait(100);
    }
}

void readData(void) {
    // read 3 values of photo-active & whole-spectrum light
    Measurement_t *rec;
    uint16_t voltSum;
    uint16_t readRec;

    if (currRecord >= MAX_RECORDS) return; // no more space

    // read adc
    greenLedOn();
    rec = &buffer[currRecord];
  
    voltSum = 0;
    for (readRec = 0; readRec < RETRIES; ++readRec) {
        rec->light[readRec] = adcRead(ADC_LIGHT_TOTAL);
        rec->psaLight[readRec] = adcRead(ADC_LIGHT_PHOTOSYNTHETIC);
        voltSum += adcRead(ADC_INTERNAL_VOLTAGE);

        mdelay(SAMPLE_PAUSE);
    }
//    PRINTF("PAR=%d", rec->light[0]);
//    PRINTF(" TSR=%d", rec->psaLight[0]);
//    PRINTF("\r\n");
    rec->voltage = voltSum / RETRIES;
    ++currRecord;

    greenLedOff();
}

void sendData(void) {
    uint16_t i;
    SadPacket_t packet;

    redLedOn();
    packet.id = SAD_STAMP;
    packet.address = MOTE_NR;
    packet.crc = 0;

    for (i = 0; i < currRecord; ++i) {
        Measurement_t *rec = &buffer[i];
        memcpy(&packet.data, rec, sizeof(*rec));
        packet.crc = crc16_data((uint8_t *) &packet, sizeof(packet) - 2, 0);

        radioSend(&packet, sizeof(packet));
        mdelay(PACKET_PAUSE);
    }

    currRecord = 0;
    redLedOff();
    radioOff();
}

void appMain(void)
{
    uint32_t nextReadTime, nextSendTime;

    nextReadTime = getRealTime() + 1000;
    nextSendTime = getRealTime() + 5000;

    while (1) {
        uint32_t now = getRealTime();
        if (timeAfter(now, nextReadTime)) {
            readData();
            nextReadTime += READ_PERIOD_MS;
        }

        if (timeAfter(now, nextSendTime)) {
            sendData();
            nextSendTime += SEND_PERIOD_MS;
        }
        
        busyWait(10000);
    }
}
