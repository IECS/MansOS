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

#include "../common.h"
#include <stdio.h>
#include <serial_number.h>
#include <kernel/expthreads/radio.h>

enum {
    IEEE154_FCF_FRAME_TYPE = 1 << 0,
    IEEE154_FCF_SECURITY_ENABLED = 1 << 3,
    IEEE154_FCF_FRAME_PENDING = 1 << 4,
    IEEE154_FCF_ACK_REQ = 1 << 5,
    IEEE154_FCF_INTRAPAN = 1 << 6,  // (pan ID compression)
    IEEE154_FCF_RSVD = 1 << 7,
    IEEE154_FCF_DEST_ADDR_MODE = 1 << 10,
    IEEE154_FCF_FRAME_VERSION = 1 << 12,
    IEEE154_FCF_SRC_ADDR_MODE = 1 << 14,
};

#define IEEE154_BEACON_FRAME       0x0
#define IEEE154_DATA_FRAME         0x1
#define IEEE154_ACK_FRAME          0x2
#define IEEE154_MAC_COMMAND_FRAME  0x3

// 41 c8 <- (IEEE154_DATA_FRAME | IEEE154_FCF_INTRAPAN | IEEE154_FCF_DEST_ADDR_MODE=10 | IEEE154_FCF_SRC_ADDR_MODE=11)
// ee <- seqnum
// 32 33 <- PAN ID
// ff ff <- dest address
// 8f e7 71 40 00 a2 13 00 <- src address
// 03 <- seqnum
// 00 52 01 23 00
// 56 78  <- "Vx"
// 54 65 73 74 20 6d 65 73 73 61 67 65 21  <- data
uint8_t myData[] = {
    0x41, 0xc8, // fcf
    0x00, // seqnum
    0x32, 0x33, // PAN ID
    0xff, 0xff, // dest address
    0x01, 0x02, 0x71, 0x40, 0x00, 0xa2, 0x13, 0x00, // src address
    0x03, // seqnum
    0x00, 0x52, 0x01, 0x23, 0x00,
    0x56, 0x78, // "Vx"
    0x54, 0x65, 0x73, 0x74, 0x20, 0x6d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x21  // data
};

// 41 c8
// 6e
// 32 33
// ff ff
// 8f e7 71 40 00 a2 13 00
// 1b
// 00 52 01 23 00
// 56 78 54 65 73 74 20 6d 65 73 73 61 67 65 21 

struct Ieee802_15_4_Packet_s {
    uint8_t fcf[2];
    uint8_t seqnum;
    uint8_t panId[2];
    uint8_t dstAddress[2];
    uint8_t srcAddress[8];
    uint8_t seqnum2;
    uint8_t something1;
    uint8_t packetId;
    uint8_t something2;
    uint8_t something3;
    uint8_t typeSourceId;
    uint8_t networkAddressOrigin[2];
    uint8_t data[24];
} PACKED;

struct Ieee802_15_4_Packet_Head_s {
    uint8_t fcf[2];
    uint8_t seqnum;
    uint8_t panId[2];
    uint8_t dstAddress[2];
    uint8_t srcAddress[8];
    uint8_t seqnum2;
    uint8_t something1;
    uint8_t packetId;
    uint8_t something2;
    uint8_t something3;
    uint8_t typeSourceId;
    uint8_t networkAddressOrigin[2];
    uint8_t data[];
} PACKED;


typedef struct Ieee802_15_4_Packet_s Ieee802_15_4_Packet_t;
typedef struct Ieee802_15_4_Packet_Head_s Ieee802_15_4_Packet_Head_t;

Ieee802_15_4_Packet_t myPacket = 
{
    .fcf = {0x41, 0xc8},
    .panId = {0x33, 0x66},
    .dstAddress = {0xff, 0xff},
    .something1 = 0xab,
    .packetId = SAD_DATA_ID,
    .something2 = 0x01,
    .something3 = 0x23,
    .typeSourceId = 0x00,
    .networkAddressOrigin = {0x56, 0x79}, // network address origin (but only in case typeSourceId == 0)
};

DataPacket_t data =
{
    .timestamp = 0xDEADBEEF,
    .islLight = 0x9999,
    .apdsLight0 = 0x8888,
    .apdsLight1 = 0x7777,
    .sq100Light = 0x6666,
    .internalVoltage = 0x5555,
    .internalTemperature = 0x4444,
    .sht75Humidity = 0x3333,
    .sht75Temperature = 0x2222,
};

uint8_t mySeqnum;

void appMain(void)
{
    // initialize radio
    RADIO_PACKET_BUFFER(radioBuffer, RADIO_MAX_PACKET);
    radioOn();
    radioSetChannel(15);

    halGetSerialNumber(myPacket.srcAddress);
    PRINTF("src MAC=%#02x %#02x %#02x %#02x %#02x %#02x %#02x %#02x\n",
            myPacket.srcAddress[0], myPacket.srcAddress[1], myPacket.srcAddress[2], myPacket.srcAddress[3],
            myPacket.srcAddress[4], myPacket.srcAddress[5], myPacket.srcAddress[6], myPacket.srcAddress[7]);

    data.sourceAddress = localAddress;

    for (;;) {
        PRINT("in appMain...\n");

        if (isRadioPacketReceived(radioBuffer)) {
            PRINTF("received %u bytes, rssi=%d, lqi=%u\n",
                    radioBuffer.receivedLength, radioGetLastRSSI(), radioGetLastLQI());
            debugHexdump(radioBuffer.buffer, radioBuffer.receivedLength);
            if (radioBuffer.receivedLength >= sizeof(Ieee802_15_4_Packet_Head_t)) {
                Ieee802_15_4_Packet_Head_t p;
                memcpy(&p, radioBuffer.buffer, sizeof(p));
                PRINTF("  packet ID=%#x\n", p.packetId);
                PRINTF("  data length=%d\n", radioBuffer.receivedLength - sizeof(p));
            }
        }
        else if (isRadioPacketError(radioBuffer)) {
            PRINTF("got an error from radio: %s\n",
                    strerror(-radioBuffer.receivedLength));
        }
        radioBufferReset(radioBuffer);
        data.timestamp = (uint32_t)(getJiffies()); // TODO: use seconds!
        data.dataSeqnum = ++mySeqnum;
        data.crc = crc16((uint8_t *) &data, sizeof(DataPacket_t) - 2);
        memcpy(&myPacket.data, &data, sizeof(DataPacket_t));
        myPacket.seqnum = myPacket.seqnum2 = mySeqnum;
        PRINTF("\n\n");
        debugHexdump(&data,sizeof(data));
        PRINTF("\n\n");
        radioSend(&myPacket, sizeof(myPacket));

        toggleRedLed();
        mdelay(5000);
    }
}
