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

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <sem_hal.h>
#include <fcntl.h>
#include <radio.h>
#include <print.h>

RadioRecvFunction radioCallback = 0;
unsigned char pcRadioBuf[MAX_PACKET_SIZE];
uint16_t pcRadioBufLen;
uint8_t pcRadioOn = 0;
pthread_mutex_t pcRadioSendMutex;
bool pcRadioInitialized;

// TODO: remake this;
// at the moment it's here only so that USE_RADIO on PC
// can be enabled witout enabling USE_NET.
int mosSendSock = -1;

void radioInit(void) {
    if (pcRadioInitialized) return;
    pcRadioInitialized = true;
    pcRadioOn = 0;
    pthread_mutex_init(&pcRadioSendMutex, NULL);
}

int8_t radioSendHeader(const void *header, uint16_t headerLength,
                       const void *data, uint16_t dataLength) {
    // first byte(s) in the packet is packet size
    unsigned char tmpBuf[MAX_PACKET_SIZE + sizeof(PcRadioPackSize_t)];
    uint16_t len, done;

    if (mosSendSock < 0) return -1;

    // only one can send at a time
    pthread_mutex_lock(&pcRadioSendMutex);

    // set length in first byte(s)
    *((PcRadioPackSize_t *) tmpBuf) = headerLength + dataLength;
    // copy msg
    if (headerLength) {
        memcpy(tmpBuf + sizeof(PcRadioPackSize_t), header, headerLength);
    }
    memcpy(tmpBuf + headerLength + sizeof(PcRadioPackSize_t), data, dataLength);
    
    // send until done
    len = sizeof(PcRadioPackSize_t) + headerLength + dataLength;
    done = 0;

    int16_t l = write(mosSendSock, tmpBuf + done, len - done);
    if (l < 0) {
        perror("radioSendHeader write");
    } else if (l == 0) {
        printf("radioSendHeader: EOF on socket\n");
    }
    pthread_mutex_unlock(&pcRadioSendMutex);
    return 0;
}

int16_t radioRecv(void *buffer, uint16_t buffLen) {
    if (!pcRadioOn) {
        fprintf(stderr, "radioRecv: radio is off\n");
        return 0;
    }

    if (pcRadioBufLen <= sizeof(PcRadioPackSize_t)) return 0;

    // copy min(bufLen, redBytes) from radio buffer to dst buffer
    uint16_t len = pcRadioBufLen - sizeof(PcRadioPackSize_t);
    if (len > buffLen) len = buffLen;
    memcpy(buffer, pcRadioBuf + sizeof(PcRadioPackSize_t), len);
    pcRadioBufLen = 0;
    return len;
}

void radioDiscard(void) {
}

RadioRecvFunction radioSetReceiveHandle(RadioRecvFunction functionHandle) {
    RadioRecvFunction old = radioCallback;
    radioCallback = functionHandle;
    return old;
}

void radioOn(void) {
//    TPRINTF("radioOn\n");
    pcRadioOn = 1;
}

void radioOff(void) {
//    TPRINTF("radioOff\n");
    pcRadioOn = 0;
}

int radioGetRSSI(void) {
    return 0;
}

int8_t radioGetLastRSSI(void) {
    return 0;
}

uint8_t radioGetLastLQI(void) {
    return 0;
}

void radioSetChannel(int channel) {
    return;
}

void radioSetTxPower(uint8_t power) {
    return;
}

bool radioIsChannelClear(void) {
    return true;
}
