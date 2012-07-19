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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <poll.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <hil/radio.h>
#include <net/addr.h>
#include <pthread.h>
#include "sem_hal.h"

//----------------------------------------------------------
// constants
//----------------------------------------------------------
enum {
    LOOPBACK_ADDR = (127 << 24) | 1,
};

//----------------------------------------------------------
// variables
//----------------------------------------------------------

// socket pair. used for data flow from apps to radio listen/send thread,
// which is polling all the time
//int mosSendSock = -1;
int mosSendPairSock = -1;
static int sockets[2] = {-1, -1};

// socket for communication with the cloud
static int cloudSock = 0;

// poll structures. contains elements:
// 0 = incoming socket for reading data from the cloud
// 1 = outgoing socket to receive data from apps and forward it to the cloud
struct pollfd pollFd[2];

// outgoing data received from apps stored here before forwarding to the cloud
unsigned char mosOutBuf[MAX_PACKET_SIZE];
unsigned mosOutBufLen = 0;
unsigned mosOutBufOffset = 0;

sem_t alarmMutex;

//----------------------------------------------------------
// internal functions
//----------------------------------------------------------
void *intHandler(void *);
int connectSock(unsigned port);
int8_t makeSocketPair();
void closeSocket(int sock);

// receive data from socket into buffer. return received byte count
int16_t pcRadioRecvPack(int sock, void *buf, uint16_t bufLen);
int16_t pcRadioSend(int sock, const void *buf, uint16_t bufLen);

// read until exactly len bytes from socket, return bytes red or -1 on error
int16_t readExactly(int sock, void *buf, uint16_t len);

void decAndFireAlarm(); // decrementing alarms timeout and fire if needed

//----------------------------------------------------------
// function implementation
//----------------------------------------------------------
static pthread_t intThread;

// simulate interrupt handler in separate thread
void *intHandler(void *dummy) {
    if (makeSocketPair() != 0) {
        printf("cannot create outgoing socket pair!\n");
        return NULL;
    }

    cloudSock = connectSock(PROXY_SERVER_PORT);
    // do not break job, when cannot connect to cloud
    // alarms should still work just fine
    // if (cloudSock <= 0) return NULL;

    // setup local address (use local port number for the moment)
    if (cloudSock != -1) {
        struct sockaddr_in saddr;
        socklen_t slen = sizeof(saddr);
        if (getsockname(cloudSock, (struct sockaddr *) &saddr, &slen) == 0) {
            localAddress = htons(saddr.sin_port);
            printf("set local address to port number: 0x%04x\n", localAddress);
        }
    }

    // poll for incoming data. outgoing data also treated as "incoming", because
    // it comes from the socket pair
    pollFd[0].fd = cloudSock;
    pollFd[0].events = POLLIN;
    pollFd[1].fd = mosSendPairSock;
    pollFd[1].events = POLLIN;

    while (1) {
        int p = poll(pollFd, 2, -1); // poll without timeout

        if (p > 0) {
            if (pollFd[0].revents & POLLIN) {
                // incoming data received
                int16_t l = pcRadioRecvPack(cloudSock, pcRadioBuf,
                        sizeof(pcRadioBuf));
                if (l < 0) {
                    // error occured
                    return NULL;
                }
                pcRadioBufLen = l;
                if (pcRadioOn && radioCallback) {
                    radioCallback();
                }
            }
            if (pollFd[1].revents & POLLIN) {
                // outgoing data received
                int16_t l = pcRadioRecvPack(mosSendPairSock, mosOutBuf,
                        sizeof(mosOutBuf));
                if (l > 0) {
                    // start polling cloud for data write only
                    pollFd[0].events = POLLOUT;
                    mosOutBufOffset = 0;
                    mosOutBufLen = l;
                } else if (l < 0) {
                    // error occured
                    return NULL;
                }
            }
            if (pollFd[0].revents & POLLOUT) {
                // cloud socket ready to receive data, send it
                int16_t l = pcRadioSend(cloudSock, mosOutBuf + mosOutBufOffset,
                        mosOutBufLen);
                if (l > 0) {
                    mosOutBufLen -= l;
                    if (mosOutBufLen == 0) {
                        // all data sent, stop write-polling
                        pollFd[0].events = POLLIN;
                        // TODO - post a semaphore to notify radioSend caller?
                    } else {
                        // some bytes left to send, update offset
                        mosOutBufOffset += l;
                    }
                } else if (l < 0) {
                    // error occurred
                    return NULL;
                } else {
                    // l = 0, just keep retrying
                }
            }
        } else if (p == 0) {
            // timeout
        } else {
            printf("poll error: %s\n", strerror(errno));
        }
    }

    // should never get here
    return NULL;
}

// read packet lenght = l, then fetch l bytes and store in buffer
// l is also stored in the first byte(s)
int16_t pcRadioRecvPack(int sock, void *buf, uint16_t bufLen) {
    // read packet length
    if (readExactly(sock, buf, sizeof(PcRadioPackSize_t))
            != sizeof(PcRadioPackSize_t)) return -1;
    PcRadioPackSize_t len = *((PcRadioPackSize_t *) buf);
    int16_t r = readExactly(sock, buf + sizeof(PcRadioPackSize_t), len);
    return r > 0 ? r + sizeof(PcRadioPackSize_t) : r;
}

// read until exactly len bytes from socket
// return len + sizeof(PcRadioPackSize_t) or -1 on error
int16_t readExactly(int sock, void *buf, uint16_t len) {
    if (len == 0) return 0;
    int16_t l = read(sock, buf, len);
    if (l == 0) {
        printf("socket %i disconnected\n", sock);
        return -1;
    }
    if (l < 0) {
        printf("socket %i reading error: %s\n", sock, strerror(errno));
    }
    return l;
}

int16_t pcRadioSend(int sock, const void *buf, uint16_t bufLen) {
//    printf("pcRadioSend(%i, %i, %i)\n", sock, (int) buf, bufLen);
    if (bufLen == 0) return 0;
    int16_t l = write(sock, buf, bufLen);
    if (l == 0) {
        printf("socket %i disconnected\n", sock);
        return -1;
    }
    if (l < 0) {
        printf("socket %i writing error: %s\n", sock, strerror(errno));
    }
    return l;
}

int connectSock(unsigned port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        printf("error = %s\n", strerror(errno));
        radioOff();
        return sock;
    }
    int on = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)))
    {
        printf("cannot set socket to reuse addr: %s\n",
                strerror(errno));
        close(sock);
        return -1;
    }
    struct sockaddr_in proxyAddr;
    memset((char *) &proxyAddr, 0, sizeof(proxyAddr));
    proxyAddr.sin_family = AF_INET;
    proxyAddr.sin_addr.s_addr = htonl(LOOPBACK_ADDR);
    proxyAddr.sin_port = htons(port);
    if (connect(sock, (struct sockaddr *) &proxyAddr, sizeof(proxyAddr)) < 0)
    {
        printf("can not connect to cloud: %s\n", strerror(errno));
        radioOff();
        close(sock);
        return -1;
    }
    return sock;
}

// create pair of connected, unnamed Unix sockets
int8_t makeSocketPair() {
    if (socketpair(PF_UNIX, SOCK_STREAM, 0, sockets) != 0) return -1;
    mosSendSock = sockets[0];
    mosSendPairSock = sockets[1];
    return 0;
}

void closeSocket(int sock) {
    if (close(sock) < 0) {
        printf("error closing socket: %s\n", strerror(errno));
    } else {
        //printf("socket closed\n");
    }
}


void initArchComm(void) {
    mos_sem_init(&alarmMutex, 0);
    // this is a "specific thread", not part of the scheduler
    // create it even, when threads are turned off
    pthread_create(&intThread, NULL, intHandler, NULL);
    // initialization of alarmIntHandler moved to alarms_hal.c
}
