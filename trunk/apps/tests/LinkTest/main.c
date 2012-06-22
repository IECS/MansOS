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
// LinkTest appplication, communicates with other nodes and tests connection 
// by sending multiple messages(TEST_COUNT).
//

#include "random.h"
#include "stdmansos.h"
#include <snum.h>
#include <net/socket.h>
#include <net/routing.h>
#include <net/net-stats.h>

//-----------------
// constants
//-----------------
enum {
    SLEEP_BETWEEN_SEND = 16,
    ADDR_PORT = 123,
    TEST_PORT = 99,
    TX_POWER = 31,
    TEST_BYTE = 0x55,
    TEST_BYTE_COUNT = 100,
    TEST_COUNT = 100,
    REPLY_TEST_BYTE = 0x66,
    REPLY_TEST_BYTE_COUNT = 5,
    ADDR_BYTE = 0xFEED,
    ADDR_BYTE_COUNT = 1,
    REPLY_ADDR_BYTE = 0xABBA,
    UNBLOCK_BYTE = 0x99,
    RELEASE_PACKET = 0xDC,
    MAX_NEIGHBORS = 100,
    WAIT_FOR_NEIGHBORS = 500,
    MAX_WAIT_FOR_REPLY_COUNT = 3,
    TEST_FREQUENCY = 10000,
};

#define ADD_NEIGHBOR(addr)  getNeighborByAddress(addr, true)
#define INC_SENT(addr)      neighbors[getNeighborByAddress(addr, false)].sent++
#define INC_RECV(addr, seqnum)  do{                                     \
        neighbors[getNeighborByAddress(addr, false)].recv++;            \
        neighbors[getNeighborByAddress(addr, false)].rssi += radioGetLastRSSI(); \
        neighbors[getNeighborByAddress(addr, false)].lqi += radioGetLastLQI(); \
        neighbors[getNeighborByAddress(addr, false)].lastSeqnum = seqnum; \
    } while(0)
#define localSeqnum(addr)   neighbors[getNeighborByAddress(addr, false)].lastSeqnum                     
#define originAddr socket->recvMacInfo->originalSrc.shortAddr
#define macSeqnum  socket->recvMacInfo->seqnum


bool blocked = 0;   /* We start free of will */
bool blocker = 0;   
MosShortAddr blockerAddress = 0;

/* Here we store info about our neighbors */
struct Neighbors{
    MosShortAddr address;
    uint32_t sent;
    uint32_t recv;
    uint8_t lastSeqnum;
    int32_t rssi;
    int32_t lqi;
}neighbors[MAX_NEIGHBORS + 1];

uint16_t neighborCount = 0;

/* Get's neighbors array id form address, 
   appends neighbor to list if he's not there. */
int getNeighborByAddress(MosShortAddr address, bool add){
    uint16_t counter;
    for (counter = 0; counter < neighborCount; counter++){
        if (neighbors[counter].address == address)
            return counter;
    }
    if (add){
        //PRINTF("\n\nADDING NEIGHBOR %#x\n\n",address);
        neighbors[counter].address = address;
        neighbors[counter].sent = 0;
        neighbors[counter].recv = 0;
        neighbors[counter].lastSeqnum = 0;
        neighbors[counter].rssi = 0;
        neighbors[counter].lqi = 0;
        neighborCount++;
        return counter;
    }
    return MAX_NEIGHBORS;   // recycle bin :D
}

static void recvTest(Socket_t *socket, uint8_t *data, uint16_t len)
{   
    uint8_t temp = *data, temp2[REPLY_TEST_BYTE_COUNT] = {REPLY_TEST_BYTE}; 
    // PRINTF("got %d bytes from 0x%04x (%#x) - recvTest\n", len, originAddr, temp);
    if (temp == REPLY_TEST_BYTE){
        if (blocker){
            //if (macSeqnum > localSeqnum(originAddr) || macSeqnum < localSeqnum(originAddr)-5){
            if (data[1] > localSeqnum(originAddr) || data[1] < localSeqnum(originAddr)-5){
                INC_RECV(originAddr, data[1]);
            } else{
                //PRINTF("wrong recv for neighbor %#x, %d <= %d\n",originAddr, data[1], localSeqnum(originAddr));
            }
        }
    } else if (temp == TEST_BYTE){
        socketSetDstAddress(socket, originAddr);
        temp2[1] = data[1];
        if (socketSend(socket, &temp2, sizeof(temp2))) {
            PRINT("socketSend failed\n");
        } else{
            //PRINTF("answered test to neighbor %#x, no %d\n",originAddr,data[1]);
        }
    } else if (temp == UNBLOCK_BYTE){
        blocked = false;
        //PRINTF("Got release\n");
    } else{
        PRINT("unknow packet, dropping\n");
    }
    redLedToggle();
}

static void sendTestRequest(Socket_t *socket, MosShortAddr addr)
{
    uint8_t i,o;
    uint8_t data[TEST_BYTE_COUNT] = {TEST_BYTE};
    socketSetDstAddress(socket, addr);
    
    for (i = 0; i < TEST_COUNT; i++){
        data[1] = i+1;
        if (socketSend(socket, &data, sizeof(data))) {
            PRINT("socketSend failed\n");
        }
        INC_SENT(addr);
        // PRINTF("INCE'd sent for %#x\n",addr);
        for (o = 0;o < MAX_WAIT_FOR_REPLY_COUNT; o++){
            if (localSeqnum(addr) == i + 1){
                //PRINTF("Got it %d\n", i + 1);
                o = MAX_WAIT_FOR_REPLY_COUNT;
            } else{
                //PRINTF("Sleeping, local:%d, sent:%d\n",localSeqnum(addr), i + 1);
                mdelay(SLEEP_BETWEEN_SEND + randomNumberBounded(4));
            }
        }
    }
}

static void sendAddrRequest(Socket_t *socket)
{
    uint8_t i;
    uint16_t data = ADDR_BYTE;
    
    for (i = 0; i < ADDR_BYTE_COUNT; i++){
        if (socketSend(socket, &data, sizeof(data))) {
            PRINT("socketSend failed\n");
        } else{
            // PRINT("Addr request sent\n");
        }
        mdelay(SLEEP_BETWEEN_SEND + randomNumberBounded(128));
    }
}

static void sendUnblock(Socket_t *socket)
{
    uint8_t data = UNBLOCK_BYTE;
    socketSetDstAddress(socket, MOS_ADDR_BROADCAST);
    if (socketSend(socket, &data, sizeof(data))) {
        PRINT("socketSend failed\n");
    } else{
        //PRINT("Unblock sent\n");
    }
    blocker = false;
}

static void recvAddrRequest(Socket_t *socket, uint8_t *data, uint16_t len){
    uint16_t temp = *(data + 1) << 8 | *data;
    //PRINTF("got %d bytes from 0x%04x (%#x) - addrRequest\n", len, originAddr, temp);
    socketSetDstAddress(socket, originAddr);
    if (temp == REPLY_ADDR_BYTE){
        ADD_NEIGHBOR(originAddr);
        // PRINTF("Added neighbor %#x\n",originAddr);
    } else if (temp == ADDR_BYTE){
        temp = REPLY_ADDR_BYTE;
        if (socketSend(socket, &temp, sizeof(temp))) {
            PRINT("socketSend failed\n");
        } else{
            //PRINT("Addr request replied\n");
            blocked = true;
            //PRINTF("Got blocked\n");
        }
    } else{
        PRINT("unknow packet, dropping\n");
    }
    redLedToggle();
}

void appMain(void)
{
    uint8_t i;
    Socket_t addrSocket, testSocket;
    
    socketOpen(&addrSocket, recvAddrRequest);
    socketBind(&addrSocket, ADDR_PORT);
    
    socketOpen(&testSocket, recvTest);
    socketBind(&testSocket, TEST_PORT);
    
    socketSetDstAddress(&addrSocket, MOS_ADDR_BROADCAST);

    PRINT_INIT(128);
    PRINTF("Local address: %#x\n",localAddress);
    
    // Wait for everyone else who might wanna test
    while(true) {
        // Wait random time
        msleep(randomInRange(1000, 2024));
    
        // Try to be first
        while (!blocker){
            if (!blocked){
                PRINTF("Searching for neighbors...\n");
                sendAddrRequest(&addrSocket);
                mdelay(WAIT_FOR_NEIGHBORS);
                if (neighborCount){
                    blocker = true;
                }
            }
            mdelay(WAIT_FOR_NEIGHBORS);
        }
        
        PRINTF("Got neighbors:\n");
        for (i = 0; i < neighborCount; i++){
            PRINTF("\t%#x\n", neighbors[i].address);
        }
        
        for (i = 0; i < neighborCount; i++){
            //PRINTF("Starting test for %#x\n", neighbors[i].address);
            //socketSetDstAddress(&testSocket, neighbors[i].address);
            sendTestRequest(&testSocket,neighbors[i].address);
            mdelay(WAIT_FOR_NEIGHBORS);
            //PRINTF("Test finished for %#x, sent - %ld, recv - %ld\n", neighbors[i].address, neighbors[i].sent, neighbors[i].recv);
        }

        sendUnblock(&testSocket);
        mdelay(WAIT_FOR_NEIGHBORS);  // in case of DEBUG=y incoming ack spoils output
        PRINTF("Results:\n");
        for (i = 0; i < neighborCount; i++){
            PRINTF("\t%#x\tsent: %ld\trecv :%ld\t", neighbors[i].address, neighbors[i].sent, neighbors[i].recv);
            if (neighbors[i].recv){
                PRINTF("RSSI: %ld\t",neighbors[i].rssi);
                PRINTF("LQI: %ld",neighbors[i].lqi);
            }
            PRINTF("\n");
        }
        PRINT_NETSTAT_ALL();
        mdelay(TEST_FREQUENCY);
        redLedToggle();
    }
}
