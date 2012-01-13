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

#include "socket.h"
#include "comm.h"
#include <string.h>
#include <lib/dprint.h>
#include <kernel/threads/mutex.h>

static SLIST_HEAD(head, Socket_s) socketList;
static Mutex_t socketListMutex;

#define lock()    mutexLock(&socketListMutex)
#define unlock()  mutexUnlock(&socketListMutex)

void socketsInit(void)
{
    SLIST_INIT(&socketList);
}

int8_t socketOpen(Socket_t *s, SocketRecvFunction cb)
{
    s->recvCb = cb;
    s->dstAddress = MOS_ADDR_ROOT;
    s->port = MOS_PORT_ANY;
    s->recvMacInfo = NULL;
    lock();
    SLIST_INSERT_HEAD(&socketList, s, chain);
    unlock();
    return 0; // XXX
}

int8_t socketClose(Socket_t *s)
{
    lock();
    SLIST_REMOVE_SAFE(&socketList, s, Socket_s, chain);
    unlock();
    return 0; // XXX
}

int8_t socketSend(Socket_t *s,const void *data, uint16_t len)
{
    return sendPacket(s->dstAddress, s->port, data, len);
}

void socketInputData(MacInfo_t *macInfo, void *data, uint16_t len) {
    Socket_t *s;
    Socket_t *catchall;
    Socket_t *t;

    //PRINT("socketInputData\n");

    s = NULL;
    catchall = NULL;
    lock();
    SLIST_FOREACH(t, &socketList, chain) {
        if (t->port == macInfo->dstPort) {
            s = t;
            break;
        }
        
        if (t->port == MOS_PORT_ANY) {
            catchall = t;
        }
    }

    if (!s) {
        if (!catchall) {
            PRINTF("warning: dropping data, no sockets listening to port %d\n",
                    macInfo->dstPort);
            unlock();
            return;
        }
        s = catchall;
    }

    if (s->recvCb) {
        // set meta info
        s->recvMacInfo = macInfo;
        // call user's callback
        s->recvCb(s, data, len);
    }
    unlock();
}

int8_t sendPacket(MosShortAddr addr, NetPort_t port,
                  const void *buf, uint16_t bufLen) {
    static MacInfo_t mi;
    memset(&mi, 0, sizeof(mi));
    fillLocalAddress(&mi.originalSrc);
    intToAddr(mi.originalDst, addr);
    mi.dstPort = port;
    mi.flags |= MI_FLAG_LOCALLY_ORIGINATED;
    commForwardData(&mi, (uint8_t *) buf, bufLen);
    return 0;
}
