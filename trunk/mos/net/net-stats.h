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

#ifndef MANSOS_NET_STATS_H
#define MANSOS_NET_STATS_H

#if USE_NET_STATS

#include "addr.h"
#include "string.h"
#include "dprint.h"

#ifndef LINQ_MAX_NEIGHBOR_COUNT
#define LINQ_MAX_NEIGHBOR_COUNT 10
#endif //LINQ_MAX_NEIGHBOR_COUNT

#define INC_NETSTAT(code, addr) incNetstat(code, addr)

#define PRINT_LINQ() linqPrintResults()
#define PRINT_NETSTAT()  do {                                           \
        PRINTF(" PACKETS_SENT \t%lu\n", netstats[NETSTAT_PACKETS_SENT]);         \
        PRINTF(" PACKETS_RTX \t%lu\n", netstats[NETSTAT_PACKETS_RTX]);           \
        PRINTF(" PACKETS_FWD \t%lu\n", netstats[NETSTAT_PACKETS_FWD]);           \
        PRINTF(" PACKETS_ACK_TX \t%lu\n", netstats[NETSTAT_PACKETS_ACK_TX]);       \
        PRINTF(" PACKETS_RECV \t%lu\n", netstats[NETSTAT_PACKETS_RECV]);         \
        PRINTF(" PACKETS_ACK_RX \t%lu\n", netstats[NETSTAT_PACKETS_ACK_RX]);       \
        PRINTF(" PACKETS_DROPPED_TX \t%lu\n", netstats[NETSTAT_PACKETS_DROPPED_TX]); \
        PRINTF(" PACKETS_DROPPED_RX \t%lu\n", netstats[NETSTAT_PACKETS_DROPPED_RX]); \
        PRINTF(" RADIO_TX \t%lu\n", netstats[NETSTAT_RADIO_TX]);               \
        PRINTF(" RADIO_RX \t%lu\n", netstats[NETSTAT_RADIO_RX]);               \
    } while (0)
#define PRINT_NETSTAT_ALL()   do {    \
        PRINT_NETSTAT();              \
        PRINT_LINQ();                 \
    } while (0)
    
#define EMPTY_ADDR 0

enum {
    NETSTAT_PACKETS_SENT,  // locally generated packets sent out 
    NETSTAT_PACKETS_RTX,   // number of times a packet was retransmitted
    NETSTAT_PACKETS_FWD,   // forwarded
    NETSTAT_PACKETS_ACK_TX, // ack sent    
    
    NETSTAT_PACKETS_RECV,  // accepted locally
    NETSTAT_PACKETS_ACK_RX, // acked by the "other side"

    NETSTAT_PACKETS_DROPPED_TX, // including forwarding path and "sent but not acked"
    NETSTAT_PACKETS_DROPPED_RX, // including "no sockets listening to this port"

    NETSTAT_RADIO_TX,      // total radio tx count (including unsuccessful, but started)
    NETSTAT_RADIO_RX,      // total radio rx count (including crc errors etc.)

    TOTAL_NETSTAT
};
uint32_t netstats[TOTAL_NETSTAT];

typedef struct LinkQuality_s{
    MosShortAddr addr;
    uint32_t sent;
    uint32_t sentAck;   //ack's sent
    uint32_t recv;
    uint32_t recvAck;   //ack's recieved
    uint16_t lastSeqnumAcked;
} LinkQuality_t;

LinkQuality_t linq[LINQ_MAX_NEIGHBOR_COUNT + 1];

uint8_t linqNeighborCount;

void linqInit();

uint16_t getIdFromAddress(MosShortAddr addr);

uint16_t addNeighbor(MosShortAddr addr);

void incNetstat(uint8_t code, MosShortAddr addr);

void linqPrintResults();

#else
#define INC_NETSTAT(code, addr)
#define PRINT_LINQ()
#define PRINT_NETSTAT()
#endif  //USE_NET_STATS

#endif //MANSOS_NET_STATS_H
