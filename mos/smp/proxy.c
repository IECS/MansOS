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

//
// This is a very simple SerialPort <-> radio proxy (i.e. forwarder)
//
// WORKS ONLY IN SINGLE-HOP NETWORK!!!
//
// State machine:
// --------------
// receive SMP Request or Walk ON serial port:
//      -- forward to radio
//      -- receive locally 
//
// receive SMP Request or Walk ON radio:
//      -- receive locally
//
// receive SMP Response ON radio:
//      -- forward to serial port
//
// receive SMP Response ON serial port:
//      -- ignore it (shouldn't happen)
// 
// local send (SMP response only):
//      -- send to serial port
//      -- send to radio

#define USE_MAC 0

#include "smp.h"
#include <hil/usart.h>
#include <hil/udelay.h>
#include <lib/random.h>
#include <lib/unaligned.h>
#include <lib/codec/crc.h>
#include <lib/dprint.h>
#include <hil/leds.h>
#include <stdarg.h>
#include <stdio.h>

#if USE_MAC
#include <net/mac.h>
MacDriver_t *mac;
// buffer used by mac protocol
static uint8_t macBuffer[400];
#else
#include <net/addr.h>
#include <hil/radio.h>
#endif

bool isBaseStation;

#ifdef DEBUG
void smpDebugPrint(const char *format, ...) {
    char buffer[100];
    va_list ap;
    uint16_t len;

    va_start(ap, format);
    vsnprintf(buffer, sizeof(buffer), format, ap);
    va_end(ap);
    len = strlen(buffer);

    USARTSendByte(1, SERIAL_PACKET_DELIMITER);
    USARTSendByte(1, PROTOCOL_DEBUG);
    USARTSendByte(1, len >> 8);
    USARTSendByte(1, len & 0xFF);
    USARTSendData(1, (uint8_t *) buffer, len);
}
#endif

#if USE_MAC
static void smpProxyRecv(MacInfo_t *mi, uint8_t *data, uint16_t length) {
    if (length < 2) {
        PRINTF("smpProxyRecv: no data!\n");
        return;
    } 
    PRINTF("buffer recv %d bytes from 0x%04x\n", length, mi->originalSrc.shortAddr);

    switch (data[1]) {
    case SMP_PACKET_REQUEST:
    case SMP_PACKET_WALK:
        smpRecv(data, length);
        break;
    case SMP_PACKET_RESPONSE:
        // simply forward it to serial port
        USARTSendByte(1, SERIAL_PACKET_DELIMITER);
        USARTSendByte(1, PROTOCOL_SMP);
        USARTSendByte(1, length >> 8);
        USARTSendByte(1, length & 0xFF);
        USARTSendData(1, data, length);
        break;
    }
}
#endif

// The format of every SMP packet that arrives here starts with 4 bytes:
//    address high, address low, length, type
void smpProxyRecvSerial(uint8_t *data, uint16_t length) {
#if USE_MAC
    const static MosAddr bcastAddr = {MOS_ADDR_TYPE_SHORT, {MOS_ADDR_BROADCAST}};
#endif
    if (length < 4) {
        PRINTF("serial recv: too small packet!\n");
        return;
    }

    uint16_t address = getU16Network(data);

    bool recvLocally = false;
    bool forward = false;
    if (address == 0 || address == 0xffff) {
        recvLocally = true;
        forward = true;
    } else {
        if (address == localAddress) {
            recvLocally = true;
        } else {
            forward = true;
        }
    }

    isBaseStation = true;

    if (forward) {
#if USE_MAC
        // forward to radio
        macSend(&bcastAddr, data, length);
#else
        uint8_t header[2];
        putU16Network(header, crc16(data, length));
        radioSendHeader(header, sizeof(header), data, length);
        // PRINTF("radioSendHeader %d bytes\n", length + 2);
#endif
    }

    if (recvLocally) smpRecv(data + 2, length - 2);
}

// dispatch to radio or serial port, depending on type and own status
void smpProxySendSmp(uint8_t *data, uint16_t length) {
    PRINTF("smpProxySendSmp (%d bytes)\n", length);

#if USE_MAC
    const static MosAddr bcastAddr = {MOS_ADDR_TYPE_SHORT, {MOS_ADDR_BROADCAST}};
#endif

    if (isBaseStation) {
        // send to serial
        USARTSendByte(1, SERIAL_PACKET_DELIMITER);
        USARTSendByte(1, PROTOCOL_SMP);
        USARTSendByte(1, length >> 8);
        USARTSendByte(1, length & 0xFF);
        USARTSendData(1, data, length);
    } else {
        // add a random delay here to minimize collision probability
        // XXX: this could be improved a lot.
        radioOff();
        mdelay(randomRand() % 600);
        radioOn();

        uint8_t header[2];
        putU16Network(header, crc16(data, length));
        radioSendHeader(header, sizeof(header), data, length);
    }

#if USE_MAC
    // send to radio
    macSend(&bcastAddr, data, length);
#endif
}

void serialReceive(uint8_t x) {
    static uint8_t recvBuf[400];
    static uint16_t symbolsRead;
    static uint16_t expectedLen;
    static enum {
        READ_DELIMITER,
        READ_LEN_BYTE1,
        READ_LEN_BYTE2,
        READ_DATA
    } state = READ_DELIMITER;

    switch (state) {
    case READ_DELIMITER:
        if (x == SERIAL_PACKET_DELIMITER) state = READ_LEN_BYTE1;
        break;
    case READ_LEN_BYTE1:
        expectedLen = x << 8;
        state = READ_LEN_BYTE2;
        break;
    case READ_LEN_BYTE2:
        expectedLen |= x;
        if (expectedLen == 0 || expectedLen > sizeof(recvBuf)) {
            PRINTF("invalid packet length on serial port: %d\n", expectedLen);
            expectedLen = 0;
            state = READ_DELIMITER;
        } else {
            state = READ_DATA;
        }
        break;
    case READ_DATA:
        recvBuf[symbolsRead++] = x;
        if (symbolsRead == expectedLen) {
            smpProxyRecvSerial(recvBuf, symbolsRead);
            state = READ_DELIMITER;
            symbolsRead = 0;
        }
        break;
    }
}

#if !USE_MAC
void radioReceive(void) {
    static uint8_t buffer[128];
    int16_t length = radioRecv(buffer, sizeof(buffer));

    // greenLedToggle();

    if (length < (isBaseStation ? 4 : 6)) {
        // PRINTF("radioRecv: too short!\n");
        return;
    }

    uint8_t *p = buffer + 2;
    length -= 2;

    uint16_t crc = getU16Network(buffer);
    if (crc != crc16(p, length)) {
        PRINTF("radioRecv: wrong CRC!");
        return;
    }

    if (isBaseStation) {
        // forward it to serial port
        USARTSendByte(1, SERIAL_PACKET_DELIMITER);
        USARTSendByte(1, PROTOCOL_SMP);
        USARTSendByte(1, length >> 8);
        USARTSendByte(1, length & 0xFF);
        USARTSendData(1, p, length);
    } else {
        // receive it locally
        uint16_t address = getU16Network(p);
        if (address != 0 && address != 0xffff
                && address != localAddress) {
            PRINTF("radioRecv: not for me!");
            return;
        }

        smpRecv(p + 2, length - 2);
    }
}
#endif

void smpInit(void) {
    USARTInit(1, SMP_BAUDRATE, 0);
    USARTEnableTX(1);
    USARTEnableRX(1);
    USARTSetReceiveHandle(1, serialReceive);

#if USE_MAC
    mac = getSimpleMac();
    mac->init(NULL, false, macBuffer, sizeof(macBuffer));
    mac->recvCb = smpProxyRecv;
#else
    radioInit();
    radioSetReceiveHandle(radioReceive);
    radioOn();
#endif
}

