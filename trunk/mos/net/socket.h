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

#ifndef MANSOS_SOCKET_H
#define MANSOS_SOCKET_H

/// \file
/// Socket public API
///

#include "mac.h"
#include <lib/list.h>

//----------------------------------------------------------
// Types
//----------------------------------------------------------

typedef uint8_t NetPort_t;

typedef void (*SocketRecvFunction)(struct Socket_s *, uint8_t *data, uint16_t len);

//! MansOS socket structure
typedef struct Socket_s {
    //! Link to the next socket
    SLIST_ENTRY(Socket_s) chain;
    //! Port, used both as destination and source; can be MOS_PORT_ANY
    NetPort_t port;
    //! Destionation address
    MosShortAddr dstAddress;
    //! Asynchronous receive callback
    SocketRecvFunction recvCb;
    //! Information about the received packet
    MacInfo_t *recvMacInfo;
} Socket_t;

//----------------------------------------------------------
// Functions
//----------------------------------------------------------

// -------------------- internal use only

void socketInputData(MacInfo_t *, void *data, uint16_t len);

void socketsInit(void);

// -------------------- User API

//! Open a socket
int8_t socketOpen(Socket_t *, SocketRecvFunction cb);

//! Close a socket
int8_t socketClose(Socket_t *);

//! Bind a socket to a specific port
static inline void socketBind(Socket_t *s, NetPort_t port)
{
    s->port = port;
}

//! Set destination address of a specific socket
static inline void socketSetDstAddress(Socket_t *s, MosShortAddr addr)
{
    s->dstAddress = addr;
}

//! Send data via socket
int8_t socketSend(Socket_t *s, const void *data, uint16_t len);

//! Send data via socket, extended version
static inline int8_t socketSendEx(Socket_t *s, const void *data, uint16_t len, MosShortAddr addr)
{
    socketSetDstAddress(s, addr);
    return socketSend(s, data, len);
}

///
/// Send a packet using the MansOS network stack without opening a socket.
///
/// If 'address' is zero, the packet is sent to root;
/// If 'address' is a broadcast address, the packet is broadcasted to all motes in single-hop neighborhood;
/// Otherwise the packet is sent to the unicast address specified.
///
int8_t sendPacket(MosShortAddr address, NetPort_t port, const void *data, uint16_t len);

#endif
