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

#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <poll.h>
#include <arpa/inet.h>
#include "radio_hal.h"

enum {
    MAX_CONN_COUNT = 128,
    QUEUE_SIZE = 128 * 1024 // 128K
};

pollfd fds[MAX_CONN_COUNT]; // polled sockets. listening socket as 0th
int fdCount = 0;
int listenSock = 0;

// ring-buffer, where received data is stored in
unsigned char queue[QUEUE_SIZE];
int queuePos; // position of last stored byte in the queue
// each socket has position of last sent byte in queue
int pos[MAX_CONN_COUNT];

int createListenSock(int port);
void newClientConnected();

// return:
//  0 = nothing red, but no error
// -1 = error or socket closed
//  n = n bytes received (n > 0)
int receiveData(int sockNr);

void sendData(int sockNr);

void closeSocket(int sockNr);

// set POLLOUT flag for all sockets except originator
void kickOthers(int originatorSock);

int main()
{
    listenSock = createListenSock(PROXY_SERVER_PORT);
    if (listenSock <= 0) return listenSock;

    fds[0].fd = listenSock;
    fds[0].events = POLLIN;
    fdCount = 1;

    // reset queue
    queuePos = -1;

    while (1)
    {
        // poll return value:
        // 0  = means timeout (should not happen)
        // -1 = error
        // > 0 = count of sockets ready
        //printf("polling %i sockets\n", fdCount);
        int p = poll(fds, fdCount, -1);
        if (p > 0)
        {
            // some sockets ready
            //printf("some sockets ready\n");
            if (fds[0].revents && fdCount < MAX_CONN_COUNT)
            {
                //printf("listenSock.revents = %i\n", fds[0].revents);
                newClientConnected();
            }

            for (int i = 1; i < fdCount; ++i)
            {
                if (fds[i].revents & POLLIN) {
                    int r = receiveData(i);
                    // something happened to this socket, it is now replaced
                    // with last socket in fds array (if any) which has
                    // to be checked
                    if (r <= 0) --i;
                }
                if (fds[i].revents & POLLOUT) sendData(i);
            }

        } else if (p < 0) {
            // error
            printf("polling error: %s\n", strerror(errno));
            return -1;
        } else {
            // timeout - should not happen
            printf("timeout? :O\n");
        }
    }

    return 0;
}

int createListenSock(int port)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock <= 0)
    {
        printf("cannot create socket for listening on port %i: %s\n",
                port, strerror(errno));
        return sock;
    }

    // if proxy app crashed, allow to restart it
    int on = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)))
    {
        printf("cannot set socket to reuse addr: %s\n",
                strerror(errno));
        return -1;
    }

    if (fcntl(sock, F_SETFL, O_NONBLOCK) == -1)
    {
        printf("cannot make socket non blocking: %s\n", strerror(errno));
        return -1;
    }
    printf("listening socket created\n");

    // bind listening socket
    struct sockaddr_in si_me;
    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(port);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sock, (struct sockaddr *) &si_me, sizeof(si_me)) < 0) {
        printf("cannot bind socket: %s\n", strerror(errno));
    } else {
        printf("socket bound to port %i\n", port);
    }
    if (listen(sock, MAX_CONN_COUNT))
    {
        printf("cannot start listening: %s\n", strerror(errno));
        return -1;
    } else {
        printf("listening started\n");
    }
    return sock;
}

void newClientConnected()
{
    // new connection started
    //printf("new connection, accepting...\n");
    struct sockaddr_in clientAddr;
    socklen_t sinSize = sizeof(clientAddr);
    int clientSock = accept(listenSock, (struct sockaddr *) &clientAddr,
            &sinSize);
    if (clientSock <= 0) {
        printf("cannot accept client socket: %s\n", strerror(errno));
        return;
    } else {
        //printf("socket %i accepted\n", clientSock);
    }
    int clientNr = fdCount++;
    pollfd *newConn = &fds[clientNr];
    newConn->fd = clientSock;
    if (newConn->fd)
    newConn->events = POLLIN;
    newConn->revents = 0;

    printf("connected client from %s, stored as %i\n",
            inet_ntoa(clientAddr.sin_addr), clientNr);
}

// return:
//  0 = nothing red, but no error
// -1 = error or socket closed
//  n = n bytes received (n > 0)
int receiveData(int sockNr)
{
    int buffStart = (queuePos == QUEUE_SIZE - 1) ? 0 : queuePos + 1;
    int buffLen = QUEUE_SIZE - buffStart;
//    printf("receiveData(%i), buffStart = %i, buffLen = %i\n",
//            sockNr, buffStart, buffLen);
    int r = read(fds[sockNr].fd, &queue[buffStart], buffLen);
    if (r >= 1) {
        kickOthers(sockNr);
        queuePos = (queuePos + r) % QUEUE_SIZE;
        //printf("%i bytes received, queuePos = %i\n", r, queuePos);
        printf("[%i]>> %i byte(s)\n", sockNr, r);
        return r;
    } else if (r == 0) {
        // socket closed on the remote end, close it here also
        closeSocket(sockNr);
        return -1;
    } else if (errno == EAGAIN) {
        printf("cannot read now, will try again later: %s\n", strerror(errno));
        return 0;
    } else {
        printf("error while reading socket: %s\n", strerror(errno));
        closeSocket(sockNr);
        return -1;
    }
}

void sendData(int sockNr)
{
    int buffStart = (pos[sockNr] == QUEUE_SIZE - 1) ? 0 : pos[sockNr] + 1;
    int buffLen;
    if (queuePos < buffStart) {
        // queuePos overflow occured, send all data until the end of queue
        // another packet will be sent afterwards
        buffLen = QUEUE_SIZE - buffStart;
    } else {
        // send all data until queuePos
        buffLen = queuePos - buffStart + 1;
    }
//    printf("sendData(%i), start = %i, len = %i, queuePos = %i\n",
//            sockNr, buffStart, buffLen, queuePos);

    int w = write(fds[sockNr].fd, &queue[buffStart], buffLen);
    if (w >= 1) {
        pos[sockNr] = (pos[sockNr] + w) % QUEUE_SIZE;
//        printf("%i bytes sent, pos[i] = %i\n", w, pos[sockNr]);
        printf("[%i]<< %i byte(s)\n", sockNr, w);
        if (pos[sockNr] == queuePos) {
//            printf("job done for socket %i\n", sockNr);
            fds[sockNr].events &= ~(POLLOUT); // stop write poll for this socket
        }
    } else if (errno == EAGAIN) {
        printf("cannot write now, will try again later: %s\n", strerror(errno));
    } else {
        printf("error while writing to socket: %s\n", strerror(errno));
        closeSocket(sockNr);
    }
}


void closeSocket(int sockNr) {
    if (close(fds[sockNr].fd) < 0) {
        printf("error closing socket: %s\n", strerror(errno));
    } else {
        printf("socket %i closed\n", sockNr);
    }
    if (fdCount > sockNr + 1) {
        // move last pollfd to this place
        fds[sockNr] = fds[fdCount - 1];
        printf("fds[%i] <- fds[%i]\n", sockNr, fdCount -1);
    }
    --fdCount;
}

void kickOthers(int originatorSock) {
    for (int i = 0; i < fdCount; ++i) {
        if (i != originatorSock) {
            if (!(fds[i].events & POLLOUT)) {
                // socket not sending anything yet, set queue position
                pos[i] = queuePos;
            }
            fds[i].events |= POLLOUT;
        }
    }
}
