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
#include <kernel/expthreads/radio.h>
#include <hil/i2c_soft.h>
#include <hil/snum.h>
#include <hil/extflash.h>
#include <lib/codec/crc.h>
#include <lib/assert.h>
#include <isl29003/isl29003.h>
#include <apds9300/apds9300.h>
#include <kernel/expthreads/alarms.h>

#define WRITE_TO_FLASH 1
#define SEND_TO_RADIO  1
#define ISL_SENSOR     1
//#define PRINT_PACKET   1

bool writeAdsRegister(uint8_t reg, uint16_t val);
bool readAdsRegister(uint8_t reg, uint16_t *val);

uint32_t extFlashAddress;

uint16_t mySeqnum;

int32_t rootClockDelta; // from routing/dv.c

// --------------------------------------------

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
    uint8_t data[sizeof(DataPacket_t)];
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

// --------------------------------------------

void sendDataPacket(DataPacket_t *packet)
{
    static Ieee802_15_4_Packet_t myPacket = {
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

    PRINT("send sensor data...\n");
    memcpy(&myPacket.data, packet, sizeof(DataPacket_t));
    myPacket.seqnum = myPacket.seqnum2 = (uint8_t) mySeqnum;

    int ret = radioSend(&myPacket, sizeof(myPacket));
    if (ret < 0) {
        PRINTF("radioSend failed: %s\n", strerror(-ret));
    }
}

// --------------------------------------------

#if WRITE_TO_FLASH
void prepareExtFlash(void)
{
    extFlashWake();

    // check if old records exist
    DataPacket_t packet;
    bool prevMissed = false;

    extFlashAddress = EXT_FLASH_RESERVED;
    while (extFlashAddress < EXT_FLASH_SIZE) {
        extFlashRead(extFlashAddress, &packet, sizeof(packet));
        if (packet.crc == crc16((uint8_t *)&packet, sizeof(packet) - sizeof(uint16_t))) {
            prevMissed = false;
        } else {
            // XXX: this is supposed to find first non-packet, but it will find first two invalid packets!!
            if (!prevMissed) {
                prevMissed = true;
            } else {
                extFlashAddress -= sizeof(packet);
                if (extFlashAddress > EXT_FLASH_RESERVED) {
                    // write special zero packet to mark reboot
                    memset(&packet, 0, sizeof(packet)); // zero crc
                    extFlashWrite(extFlashAddress, &packet, sizeof(packet));
                    extFlashAddress += sizeof(packet);
                }
                break;
            }
        }
        extFlashAddress += sizeof(packet);
    }
    PRINTF("flash packet offset=%lu\n", extFlashAddress - EXT_FLASH_RESERVED);
}
#endif

void rxRoutingPacket(uint8_t *data)
{
    static RoutingInfoPacket_t myRoutingInfo;

    RoutingInfoPacket_t ri;
    memcpy(&ri, data, sizeof(ri));
    if (seqnumAfter(myRoutingInfo.seqnum, ri.seqnum) && seqnumAfter(ri.seqnum, myRoutingInfo.seqnum - 5)) {
        PRINT("Routing info, but already seen!\n");
        return;
    }

    PRINT("rx routing info:\n");
    debugHexdump(&ri, sizeof(ri));

    memcpy(&myRoutingInfo, &ri, sizeof(ri));
    // this is the only parameter we use at the moment
    rootClockDelta = (int32_t)(ri.rootClock - getJiffies());
    PRINTF("  ri.rootClock=%lu, delta=%ld\n", ri.rootClock, rootClockDelta);
}

void radioTryRx(void)
{
    if (isRadioPacketReceived(*radioPacketBuffer)) {
//        PRINTF("received %u bytes, rssi=%d, lqi=%u\n",
//                radioPacketBuffer->receivedLength, radioGetLastRSSI(), radioGetLastLQI());
//        debugHexdump(radioPacketBuffer->buffer, radioPacketBuffer->receivedLength);
        uint16_t len = radioPacketBuffer->receivedLength;
        if (len >= sizeof(Ieee802_15_4_Packet_Head_t)) {
            Ieee802_15_4_Packet_Head_t p;
            memcpy(&p, radioPacketBuffer->buffer, sizeof(p));
            PRINTF("  packet ID=%#x\n", p.packetId);
            PRINTF("  data length=%d\n", len - sizeof(p));
            if (p.packetId == SAD_ROUTING_ID) {
                rxRoutingPacket(radioPacketBuffer->buffer + len - sizeof(SadRoutingInfoPacket_t));
                // PRINT("ROUTING!\n");
                // debugHexdump(radioPacketBuffer->buffer, radioPacketBuffer->receivedLength);
                // PRINT("\n");
            }
        }
    }
    else if (isRadioPacketError(*radioPacketBuffer)) {
        PRINTF("got an error from radio: %s\n",
                strerror(-radioPacketBuffer->receivedLength));
    }
    radioBufferReset(*radioPacketBuffer);
}


// --------------------------------------------

void readSensors(DataPacket_t *packet)
{
//    PRINTF("reading sensors...\n");

    ledOn();

    // humidityOn();
    // mdelay(SHT11_RESPONSE_TIME * 1000);

    packet->timestamp = (uint32_t)(getJiffies() + rootClockDelta);
    packet->sourceAddress = localAddress;
    packet->dataSeqnum = ++mySeqnum;
#if ISL_SENSOR
    if (!islRead(&packet->islLight, true)) {
        PRINT("islRead failed\n");
        packet->islLight = 0xffff;
    }
    packet->apdsLight0 = 0xffff;
    packet->apdsLight1= 0xffff;
#else
    if (apdsReadWord(COMMAND | DATA0LOW_REG, &packet->apdsLight0) != 0) {
        PRINT("apdsReadWord 0 failed\n");
        packet->apdsLight0 = 0xffff; // error value
    }
    if (apdsReadWord(COMMAND | DATA1LOW_REG, &packet->apdsLight1) != 0) {
        PRINT("apdsReadWord 1 failed\n");
        packet->apdsLight1 = 0xffff; // error value
    }
    packet->islLight = 0xffff;
#endif
    if (!readAdsRegister(ADS_CONVERSION_REGISTER, &packet->sq100Light)) {
        PRINT("readAdsRegister failed\n");
        packet->sq100Light = 0xffff;
    }
    packet->internalVoltage = adcRead(ADC_INTERNAL_VOLTAGE);
    packet->internalTemperature = adcRead(ADC_INTERNAL_TEMPERATURE);
    packet->sht75Humidity = readHumidity();
    packet->sht75Temperature = readHTemperature();
    packet->crc = crc16((uint8_t *) packet, sizeof(*packet) - 2);

#if WRITE_TO_FLASH
    radioOff();
    extFlashWrite(extFlashAddress, packet, sizeof(*packet));
    DataPacket_t verifyRecord;
    memset(&verifyRecord, 0, sizeof(verifyRecord));
    extFlashRead(extFlashAddress, &verifyRecord, sizeof(verifyRecord));
    if (memcmp(packet, &verifyRecord, sizeof(verifyRecord))) {
        ASSERT("writing in flash failed!" && false);
    }
    extFlashAddress += sizeof(verifyRecord);
    radioOn();
#endif

    // humidityOff();
    ledOff();
}

// --------------------------------------------

#if PRINT_PACKET
void printPacket(DataPacket_t *packet)
{
    PRINTF("dataSeqnum=%#x\n", packet->dataSeqnum);
    PRINTF("islLight=%#x\n", packet->islLight);
    PRINTF("apdsLight=%#x/%#x\n", packet->apdsLight0, packet->apdsLight1);
    PRINTF("sq100Light=%#x\n", packet->sq100Light);
    PRINTF("internalVoltage=%u\n", packet->internalVoltage);
    PRINTF("internalTemperature=%u\n", packet->internalTemperature);
    PRINTF("sht75Humidity=%#x\n", packet->sht75Humidity);
    PRINTF("sht75Temperature=%#x\n", packet->sht75Temperature);
}
#endif

// --------------------------------------------

Alarm_t alarm;

#define ALARM_INTERVAL (30 * 1000)

void alarmCallback(void *param)
{
    PRINT("alarmCallback\n");
    radioOff();
    mdelay(100);
    radioOn();
    // reschedule the alarm
    alarmSchedule(&alarm, ALARM_INTERVAL);
}

void appMain(void)
{
    uint16_t i;

    // ------------------------- serial number
    PRINTF("Mote %#04x starting...\n", localAddress);

    // ------------------------- external flash
#if WRITE_TO_FLASH
    prepareExtFlash();
#endif

    // ------------------------- light sensor
#if ISL_SENSOR
    islInit();
    islOn();
//    configureIsl(2, 0, 3, 3);
#else
    apdsInit();
    apdsOn();
    // disable interrupt generation
    apdsCommand(INTERRUPT_REG, DISABLE_INTERRUPT);
    // set normal integration mode (maximal sensitivity)
    apdsCommand(TIMING_REG, INTEGRATION_TIME_NORMAL);
#endif

    // ------------------------- 16 bit ADC
    // i2cInit(); -- not needed because islInit() or apdsInit() calls this as well
    writeAdsRegister(ADS_CONFIG_REGISTER, 0x8483);

    // ------------------------- networking
#if SEND_TO_RADIO
    // initialize radio
    RADIO_PACKET_BUFFER(radioBuffer, RADIO_MAX_PACKET);
    radioOn();
    radioSetChannel(15);
#else
    radioOff();
#endif

    // alarms (try to reset radio stack in this way...)
    alarmInit(&alarm, alarmCallback, NULL);
    alarmSchedule(&alarm, ALARM_INTERVAL);


    // ------------------------- main loop
    for (i = 0; i < 6; ++i) {
        toggleRedLed();
        mdelay(100);
    }
    ledOff();

    uint32_t nextDataReadTime = 0;
    for (;;) {
        // ledOn();
#if SEND_TO_RADIO
        radioOn();
        yield();
        radioTryRx();
#endif

        uint32_t now = getRealTime();
        if (timeAfter32(now, nextDataReadTime)) {
            if (getJiffies() < 300 * 1000ul ) {
                nextDataReadTime = now + DATA_INTERVAL_SMALL;
            } else {
                nextDataReadTime = now + DATA_INTERVAL;
            }
            DataPacket_t packet;
            readSensors(&packet);
#if PRINT_PACKET
            printPacket(&packet);
#endif
#if SEND_TO_RADIO
            sendDataPacket(&packet);
#endif
        }
#if SEND_TO_RADIO
        // wait some time for radio rx!
        mdelay(100);
        radioTryRx();
        radioOff();
#endif

        // turn everything off and sleep for a while
        // ledOff();
        msleep(5000);
    }
}
