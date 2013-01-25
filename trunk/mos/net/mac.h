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

#ifndef MANSOS_MAC_H
#define MANSOS_MAC_H

/// \file
/// MAC protocol public API
///

#include "address.h"
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

//! MAC protocol header structure (source and destination addresses, ports, etc.)
typedef struct MacInfo_s {
    //! The address of the originator of the packet
    MosAddr originalSrc;
    //! The address of the destination of the packet
    MosAddr originalDst;
    //! The address of the immediate previous hop of the packet
    MosAddr immedSrc; 
    //! The address of the immediate next hop of the packet
    MosAddr immedDst;
    //! Source port (set at originator)
    uint8_t srcPort;
    //! Destination port (set at originator, refers to final destinatination)
    uint8_t dstPort;
    //! Sequence number
    uint8_t seqnum;
    //! The path cost of the packed
    uint8_t cost;
    //! The hop limit of the packet. The packet is dropped when hoplimit becomes zero.
    uint8_t hoplimit;
    //! Flags
    uint8_t flags;
    //! Pointer to the buffer where this structure is stored in a size-optimized way
    uint8_t *macHeader;
    //! Length of the packed buxffer
    uint16_t macHeaderLen;
    //! Time in miliseconds (absolute value) when this packet should be sent out
    uint32_t timeWhenSend;
} MacInfo_t; 

//! Check whether a MAC buffer describes a locally originated packet
#define IS_LOCAL(mi) (mi->flags & MI_FLAG_LOCALLY_ORIGINATED)

//! Returns immediate destination if set, and original destination otherwise
#define getNexthop(mi) \
    (mi->immedDst.shortAddr ? : mi->originalDst.shortAddr)

typedef void (*RecvFunction)(MacInfo_t *, uint8_t *data, uint16_t len);


//! A MansOS MAC (media access control) protocol
typedef struct MacProtocol_s {
    //! The code of the MAC procotol
    uint8_t name;
    
    //! Initialization function (protocol-specific)
    void (*init)(RecvFunction);

    //! Initialization function (protocol-specific)
    int8_t (*send)(MacInfo_t *, const uint8_t *data, uint16_t length);

    //! Receive function callback (protocol-specific)
    RecvFunction recvCb;

    //! Polling function  (protocol-specific)
    void (*poll)(void);

    //! Known address check (protocol-specific)
    bool (*isKnownDstAddress)(MosAddr *dst);

    // Internal - build the packed binary header using MacInfo_t as a source
    bool (*buildHeader)(MacInfo_t *, uint8_t **header /* out */, uint16_t *headerLength /* out */);
} MacProtocol_t;

///
/// The globally active MAC protocol
///
/// Only one MAC protocol can be used by application, and it is selected at compile time.
///
extern MacProtocol_t macProtocol;

//===========================================================
// Procedures
//===========================================================

//! Send a packet
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

//! The size of MAC protocol receive/send buffer
#ifndef MAC_PROTOCOL_BUFFER_SIZE
#define MAC_PROTOCOL_BUFFER_SIZE RADIO_MAX_PACKET
#endif

//! The time (milliseconds) to wait for acknowledgement from the receiver (in case software ACKs are used)
#ifndef MAC_PROTOCOL_ACK_TIME
#define MAC_PROTOCOL_ACK_TIME 200 // milliseconds
#endif

//! The number of attemps a single packet is send to a unicast destination
#ifndef MAC_PROTOCOL_MAX_ATTEMPTS
#define MAC_PROTOCOL_MAX_ATTEMPTS 3
#endif

//! Milliseconds to back off between sending attempts. Exponential multiplier is used for subsequent retries
#ifndef MAC_PROTOCOL_RETRY_TIMEOUT
#define MAC_PROTOCOL_RETRY_TIMEOUT 100 // 
#endif

//! The initial back off betwwen sending a packet
#ifndef MAC_PROTOCOL_MAX_INITIAL_BACKOFF
#define MAC_PROTOCOL_MAX_INITIAL_BACKOFF 30
#endif

//! The size of the unsent packet queue
#ifndef MAC_PROTOCOL_QUEUE_SIZE
#define MAC_PROTOCOL_QUEUE_SIZE  3
#endif

//! The delay (in milliseconds) for MAC-layer packet forwarding
#ifndef MAC_FORWARDING_DELAY
#define MAC_FORWARDING_DELAY 1
#endif

#endif
