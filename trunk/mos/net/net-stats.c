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

#include "net-stats.h"

inline void linqInit(){
    linqNeighborCount = 0;
}

inline uint16_t getIdFromAddress(MosShortAddr addr){
    uint8_t i;
    for (i = 0; i < linqNeighborCount; i++){
        if (linq[i].addr == addr){
            return i;
        }
    }
    return addNeighbor(addr);
}

inline uint16_t addNeighbor(MosShortAddr addr){
    if (linqNeighborCount < LINQ_MAX_NEIGHBOR_COUNT){
        memset(&linq[linqNeighborCount], 0, sizeof (LinkQuality_t));
        memcpy(&linq[linqNeighborCount].addr, &addr, sizeof(MosShortAddr));
        return linqNeighborCount++;
    }
    return LINQ_MAX_NEIGHBOR_COUNT;
}

void incNetstat(uint8_t code, MosShortAddr addr){
    if (addr != EMPTY_ADDR){
        switch (code){
            case NETSTAT_PACKETS_RTX:
            case NETSTAT_PACKETS_FWD:
            case NETSTAT_PACKETS_SENT:
                linq[getIdFromAddress(addr)].sent++;
                break;
            case NETSTAT_PACKETS_ACK_RX:
                linq[getIdFromAddress(addr)].recvAck++;
                break;
            case  NETSTAT_PACKETS_RECV:
                linq[getIdFromAddress(addr)].recv++;
                break;
            case  NETSTAT_PACKETS_ACK_TX:
                linq[getIdFromAddress(addr)].sentAck++;
                break;
        }
    }
    netstats[code]++;
}

void linqPrintResults(){
    uint8_t i;
    PRINTF("Addr\tSent\tAck sent\tRecv\tAck recv\n");
    for (i = 0; i < linqNeighborCount; i++){
        PRINTF("%#x\t",linq[i].addr);
        PRINTF("%d\t",linq[i].sent);
        PRINTF("%d\t\t",linq[i].sentAck);
        PRINTF("%d\t",linq[i].recv);
        PRINTF("%d\n",linq[i].recvAck);
    }
}
