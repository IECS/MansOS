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

#define NUM_SENDERS 10

typedef struct Sender_s {
    uint32_t address;
    uint32_t timestamp;
} Sender_t;

Sender_t senders[NUM_SENDERS];

static bool findDuplicate(uint16_t address, uint32_t timestamp)
{
    uint16_t i;
    for (i = 0; i < NUM_SENDERS; ++i) {
        if (senders[i].address == address) {
            if (senders[i].timestamp == timestamp) {
                return true;
            }
            senders[i].timestamp = timestamp;
            return false;
        }
    }

    for (i = 0; i < NUM_SENDERS; ++i) {
        if (senders[i].address == 0) {
            senders[i].address = address;
            senders[i].timestamp = timestamp;
            break;
        }
    }
    return false;
}

static void recvData(Socket_t *socket, uint8_t *data, uint16_t len)
{
    // PRINTF("got %d bytes from 0x%04x\n", len, socket->recvMacInfo->originalSrc.shortAddr);
    if (len < sizeof(DataPacket_t)) {
        PRINTF("recvData: too short! (%u vs %u)\n", len, sizeof(DataPacket_t));
        return;
    }

    DataPacket_t packet;
    memcpy(&packet, data, len);
    uint16_t calcCrc = crc16((uint8_t *)&packet, sizeof(packet) - 2);
    if (calcCrc != packet.crc) {
        PRINTF("recvData: wrong crc! (%#x vs %#x)\n", calcCrc, packet.crc);
        return;
    }
    if (findDuplicate(socket->recvMacInfo->originalSrc.shortAddr, packet.timestamp)) {
        PRINTF("duplicate!\n");
        return;
    }
    PRINTF("got packet from 0x%04x\n",
            socket->recvMacInfo->originalSrc.shortAddr);

    // PRINTF("got packet from 0x%04x, time %lu (local %lu):\n",
    //         socket->recvMacInfo->originalSrc.shortAddr,
    //         packet.timestamp,
    //         getRealTime());
#if 1
    PRINTF("timestamp=%#x\n", packet.timestamp);
    PRINTF("islLight=%#x\n", packet.islLight);
    PRINTF("apdsLight=%#x/%#x\n", packet.apdsLight0, packet.apdsLight1);
    PRINTF("sq100Light=%#x\n", packet.sq100Light);
    PRINTF("internalVoltage=%u\n", packet.internalVoltage);
    PRINTF("internalTemperature=%u\n", packet.internalTemperature);
    PRINTF("sht75Humidity=%#x\n", packet.sht75Humidity);
    PRINTF("sht75Temperature=%#x\n", packet.sht75Temperature);
#endif
    char buffer[100];
    sprintf(buffer, "$%x %lx %x %x %x %x %x %x %x %x^",
            socket->recvMacInfo->originalSrc.shortAddr,
            packet.timestamp,
            packet.islLight,
            packet.apdsLight0,
            packet.apdsLight1,
            packet.sq100Light,
            packet.internalVoltage,
            packet.internalTemperature,
            packet.sht75Humidity,
            packet.sht75Temperature);
    PRINT(buffer);
    PRINTF("%x\n", crc16((uint8_t *)buffer, strlen(buffer)));

    toggleBlueLed();
}

// void writeTestData(void)
// {
//     Socket_t socket;
//     MacInfo_t mi;

//     mi.originalSrc.shortAddr = 0xCACA;
//     socket.recvMacInfo = &mi;

//     DataPacket_t packet;
//     packet.timestamp = getRealTime();
//     packet.magicNumber = 0xcafe;
//     packet.islLight = 0x151;
//     packet.apdsLight0 = 0xad1;
//     packet.apdsLight1 = 0xad2;
//     packet.sq100Light = 0x5100;
//     packet.internalVoltage = 4095;
//     packet.internalTemperature = 0x1e1;
//     packet.sht75Temperature = 0x751;
//     packet.sht75Humidity = 0x752;
//     packet.crc = crc16((uint8_t *) &packet, sizeof(packet) - 2);
//     recvData(&socket, (uint8_t *)&packet, sizeof(packet));
// }

#define DELIMITER '$'

extern uint32_t lastRootSyncJiffies;
extern uint32_t lastRootClockSeconds;

void onUsartDataRecvd(uint8_t *data) {
    struct {
        unsigned char delimiter1;
        unsigned char delimiter2;
        unsigned short crc;
        unsigned long time;
    } s;

    memcpy(&s, data, sizeof(s));
    if (s.delimiter1 != DELIMITER
            || s.delimiter2 != 0) {
        PRINT("onUsartDataRecvd: wrong delimiters!\n");
        return;
    }
    uint16_t crc = crc16((uint8_t *) &s.time, sizeof(s.time));
    if (crc != s.crc) {
        PRINT("onUsartDataRecvd: wrong crc\n");
        return;
    }

    PRINTF("rx time %lu via serial\n", s.time);
    // change the system time used in routing protocol
    lastRootClockSeconds = s.time;
    lastRootSyncJiffies = getJiffies();
}

void usartReceive(uint8_t byte) {
    static uint8_t recvBuffer[8];
    static int bytesAfterDelimiter = -1;
    // PRINTF("usartReceive %#x (%c)\n", byte, byte);
    if (bytesAfterDelimiter == -1) {
        if (byte == DELIMITER) bytesAfterDelimiter = 0;
        else return;
    }
    recvBuffer[bytesAfterDelimiter++] = byte;
    if (bytesAfterDelimiter > sizeof(recvBuffer)) {
        bytesAfterDelimiter = -1;
        onUsartDataRecvd(recvBuffer);
    }
}

void appMain(void)
{
    PRINTF("Base station %#04x starting...\n", localAddress);

    Socket_t dataSocket;
    socketOpen(&dataSocket, recvData);
    socketBind(&dataSocket, DATA_PORT);

    // will read router time via serial
    // USARTSetReceiveHandle(PRINTF_USART_ID, usartReceive);

    for (;;) {
        PRINT("in app main\n");
        toggleRedLed();
        // writeTestData();
        mdelay(3000);

        radioOff();
        mdelay(100);
        radioOn();
    }
}
