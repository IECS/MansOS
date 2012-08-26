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

#ifndef MANSOS_MAC_H
#define MANSOS_MAC_H

#include "addr.h"
#include <radio.h>

//===========================================================
// Data types and constants
//===========================================================

// MAC protocols supported
#define MAC_PROTOCOL_NULL 1
#define MAC_PROTOCOL_CSMA 2
#define MAC_PROTOCOL_CSMA_ACK 3
#define MAC_PROTOCOL_SAD 4

#define MI_FLAG_LOCALLY_ORIGINATED  0x1
#define MI_FLAG_MORE_DATA           0x2
#define MI_FLAG_ACK_REQUESTED       0x4
#define MI_FLAG_IS_ACK              0x8

typedef struct MacInfo_s {
    MosAddr originalSrc;
    MosAddr originalDst;
    MosAddr immedSrc; 
    MosAddr immedDst;
    uint8_t srcPort;
    uint8_t dstPort;
    uint8_t seqnum;
    uint8_t cost;
    uint8_t hoplimit;
    uint8_t flags;
    uint8_t *macHeader;
    uint16_t macHeaderLen;
    uint32_t timeWhenSend;
} MacInfo_t; 

#define IS_LOCAL(mi) (mi->flags & MI_FLAG_LOCALLY_ORIGINATED)

// return immed dst, if set, and original dst otherwise
#define getNexthop(mi) \
    (mi->immedDst.shortAddr ? : mi->originalDst.shortAddr)

typedef void (*RecvFunction)(MacInfo_t *, uint8_t *data, uint16_t len);

typedef struct MacProtocol_s {
    uint8_t name; // code

    RecvFunction recvCb;

    void (*init)(RecvFunction);

    int8_t (*send)(MacInfo_t *, const uint8_t *data, uint16_t length);

    void (*poll)(void);

    bool (*isKnownDstAddress)(MosAddr *dst);

    // internal
    bool (*buildHeader)(MacInfo_t *, uint8_t **header /* out */, uint16_t *headerLength /* out */);
} MacProtocol_t;

extern MacProtocol_t macProtocol;

//===========================================================
// Procedures
//===========================================================

int8_t macSend(MosAddr *dst, const uint8_t *data, uint16_t length);
int8_t macSendEx(MacInfo_t *mi, const uint8_t *data, uint16_t length);

// default MAC header creation from MacInfo
bool defaultBuildHeader(MacInfo_t *mi, uint8_t **header /* out */, uint16_t *headerLength /* out */);
// returns pointer to data if succeeded, or NULL if failed
// mac header pointer and length are stored in mi fields
uint8_t *defaultParseHeader(uint8_t *data, uint16_t length, MacInfo_t *mi /* out */);
// always return true
bool defaultIsKnownDstAddress(MosAddr *);

uint8_t getMacHeaderSeqnum(uint8_t *data);

void invertDirection(MacInfo_t *);

// internal
void fillLocalAddress(MosAddr *result);
bool isLocalAddress(MosAddr *addr);

#ifndef MAC_PROTOCOL_BUFFER_SIZE
#define MAC_PROTOCOL_BUFFER_SIZE RADIO_MAX_PACKET
#endif

#ifndef MAC_PROTOCOL_ACK_TIME
#define MAC_PROTOCOL_ACK_TIME 200 // milliseconds
#endif

#ifndef MAC_PROTOCOL_MAX_TRIES
#define MAC_PROTOCOL_MAX_TRIES 3
#endif

// subsequent retries are sent with exponential backoff
#ifndef MAC_PROTOCOL_RETRY_TIMEOUT
#define MAC_PROTOCOL_RETRY_TIMEOUT 100 // milliseconds
#endif

#ifndef MAC_PROTOCOL_MAX_INITIAL_BACKOFF
#define MAC_PROTOCOL_MAX_INITIAL_BACKOFF 30
#endif

#ifndef MAC_PROTOCOL_QUEUE_SIZE
#define MAC_PROTOCOL_QUEUE_SIZE  3
#endif

#ifndef MAC_FORWARDING_DELAY
#define MAC_FORWARDING_DELAY 1
#endif

#endif
