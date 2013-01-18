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

#ifndef MANSOS_QUEUE_H
#define MANSOS_QUEUE_H

#include "mac.h"
#include <lib/list.h>
#include <lib/buffer.h>

typedef struct QueuedPacket_s {
    STAILQ_ENTRY(QueuedPacket_s) chain;
    uint8_t sendTries; // how many times already tried to send
    uint8_t __reserved; // XXX
    uint32_t ackTime;  // await ack until this time
    Buffer_t buffer;
} QueuedPacket_t;

void queueInit(void);

// TODO: inline most of these!

//  add new packet to userQueue tail. returns error code. locks mutex.
int8_t queueAddPacket(MacInfo_t *, const uint8_t *data, uint16_t length,
        bool replace, QueuedPacket_t **result);
// frees the userQueue head packet. locks mutex. 
void queuePop(void);
// mutex is not locked
QueuedPacket_t *queueHead(void);

typedef void (*QpacketProcessFn)(QueuedPacket_t *);
typedef bool (*QpacketMatchFn)(QueuedPacket_t *, void *userData);

// work queue processing. mutex is not locked.
void queueForEachPacket(QpacketProcessFn);
QueuedPacket_t *queueGetPacket(QpacketMatchFn, void *userData);
QueuedPacket_t *queueRemovePacket(QpacketMatchFn, void *userData);
//void queueRemovePacketByPtr(QueuedPacket_t *);

void queueFreePacket(QueuedPacket_t *);

typedef STAILQ_HEAD(head, QueuedPacket_s) PacketQueue_t;
extern PacketQueue_t packetQueue;

#endif
