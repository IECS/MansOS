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

//
// BaseStation: forwards light and voltage readings to serial port
//


#include "mansos.h"
#include "leds.h"
#include "radio.h"
#include "usart.h"
#include "dprint.h"
#include "../sense.h"

void parsePacket(SadPacket_t *p) {
    PRINTF("%u,",p->address);
    Measurement_t *data = &p->data;
    uint16_t i;
    for (i = 0; i < RETRIES; ++i) {
        PRINTF("%u,", data->light[i]);
    }
    for (i = 0; i < RETRIES; ++i) {
        PRINTF("%u,", data->psaLight[i]); 
    }
    PRINTF("%u\n", p->data.voltage);   
}

static void bsRecvRadio()
{
    static uint8_t buffer[130];
    uint16_t len;

    len = radioRecv(buffer, sizeof(buffer));
    toggleGreenLed();
    if (len < sizeof(SadPacket_t)) {
        PRINTF("bsRecvRadio: len = %i\n", len);
        return;
    }
#if 1  // check CRC on base station?
    {
        SadPacket_t *p;
        uint16_t calcCrc;
        p = (SadPacket_t *) buffer;
        calcCrc = 0; // XXX
        if (p->crc != calcCrc) {
            PRINTF("bsRecvRadio: bad CRC 0x%04x, expected 0x%04x\n",
                    p->crc, calcCrc);
            return;
        }

        parsePacket(p);
    }
#endif
//    debugHexdump(buffer, len);

    // forward to serial...
    USARTSendData(1, buffer, len);
}

void appMain(void)
{
    radioOn();
    radioSetReceiveHandle(bsRecvRadio);

    PRINT("Base Station started\n");
}
