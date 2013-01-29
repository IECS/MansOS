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

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sem_hal.h>
#include <fcntl.h>
#include <radio.h>
#include <print.h>

RadioRecvFunction pcRadioCallback;
unsigned char pcRadioBuf[MAX_PACKET_SIZE];
uint16_t pcRadioBufLen;
bool pcRadioIsOn = false;
pthread_mutex_t pcRadioSendMutex;
bool pcRadioInitialized;

// TODO: remake this;
// at the moment it's here only so that USE_RADIO on PC
// can be enabled witout enabling USE_NET.
int mosSendSock = -1;

void pcRadioInit(void) {
    if (pcRadioInitialized) return;
    pcRadioInitialized = true;
    pcRadioIsOn = 0;
    pthread_mutex_init(&pcRadioSendMutex, NULL);
}

void pcRadioReinit(void) {
    // do nothing
}

int8_t pcRadioSendHeader(const void *header, uint16_t headerLength,
                       const void *data, uint16_t dataLength) {
    // first byte(s) in the packet is packet size
    union {
        unsigned char buf[MAX_PACKET_SIZE + sizeof(PcRadioPackSize_t)];
        PcRadioPackSize_t msgLength;
    } tmpBuf;
    uint16_t len, done;

    if (mosSendSock < 0) return -1;

    // only one can send at a time
    pthread_mutex_lock(&pcRadioSendMutex);

    // set length in first byte(s)
    tmpBuf.msgLength = headerLength + dataLength;
    // copy msg
    if (headerLength) {
        memcpy(tmpBuf.buf + sizeof(PcRadioPackSize_t), header, headerLength);
    }
    memcpy(tmpBuf.buf + headerLength + sizeof(PcRadioPackSize_t), data, dataLength);
    
    // send until done
    len = sizeof(PcRadioPackSize_t) + headerLength + dataLength;
    done = 0;

    int16_t l = write(mosSendSock, tmpBuf.buf + done, len - done);
    if (l < 0) {
        perror("radioSendHeader write");
    } else if (l == 0) {
        PRINTF("radioSendHeader: EOF on socket\n");
    }
    pthread_mutex_unlock(&pcRadioSendMutex);
    return 0;
}

int16_t pcRadioRecv(void *buffer, uint16_t buffLen) {
    if (!pcRadioIsOn) {
        PRINTF("radioRecv: pcRadio is off\n");
        return 0;
    }

    if (pcRadioBufLen <= sizeof(PcRadioPackSize_t)) return 0;

    // copy min(bufLen, redBytes) from pcRadio buffer to dst buffer
    uint16_t len = pcRadioBufLen - sizeof(PcRadioPackSize_t);
    if (len > buffLen) len = buffLen;
    memcpy(buffer, pcRadioBuf + sizeof(PcRadioPackSize_t), len);
    pcRadioBufLen = 0;
    return len;
}

void pcRadioDiscard(void) {
}

RadioRecvFunction pcRadioSetReceiveHandle(RadioRecvFunction functionHandle) {
    RadioRecvFunction old = pcRadioCallback;
    pcRadioCallback = functionHandle;
    return old;
}

void pcRadioOn(void) {
//    TPRINTF("radioOn\n");
    pcRadioIsOn = 1;
}

void pcRadioOff(void) {
//    TPRINTF("radioOff\n");
    pcRadioIsOn = 0;
}

int pcRadioGetRSSI(void) {
    return 0;
}

int8_t pcRadioGetLastRSSI(void) {
    return 0;
}

uint8_t pcRadioGetLastLQI(void) {
    return 0;
}

void pcRadioSetChannel(int channel) {
    return;
}

void pcRadioSetTxPower(uint8_t power) {
    return;
}

bool pcRadioIsChannelClear(void) {
    return true;
}
