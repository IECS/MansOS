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

#include "queue.h"
#include <arch_mem.h>
#include <lib/assert.h>
#include <print.h>
#include <errors.h>
#include <kernel/threads/mutex.h>

#define DYNAMIC_ALLOCATION      0

#define BUFFER_SIZE             MAC_PROTOCOL_BUFFER_SIZE

PacketQueue_t packetQueue;

static Mutex_t mutex;
#define lock()     mutexLock(&mutex) 
#define unlock()   mutexUnlock(&mutex)

static uint8_t packetCount;

#if !DYNAMIC_ALLOCATION
static bool used[MAC_PROTOCOL_QUEUE_SIZE];
static QueuedPacket_t packetPool[MAC_PROTOCOL_QUEUE_SIZE];
static uint8_t bufferPool[MAC_PROTOCOL_QUEUE_SIZE][BUFFER_SIZE];
#endif

// -----------------------------------------------------

void queueInit(void) {
    STAILQ_INIT(&packetQueue);
}

static inline QueuedPacket_t *newQpacket(bool replace) {
    QueuedPacket_t *ret = NULL;
    if (packetCount == MAC_PROTOCOL_QUEUE_SIZE) {
        if (replace) queuePop();
        else return NULL;
    }
    ++packetCount;
#if DYNAMIC_ALLOCATION
    ret = memAlloc(sizeof(*ret));
    bufferInit(&ret->buffer, memAlloc(BUFFER_SIZE), BUFFER_SIZE);
//    ret->sendTries = 0;
//    ret->ackTime = 0;
#else
    uint8_t i;
    for (i = 0; i < MAC_PROTOCOL_QUEUE_SIZE; ++i) {
        if (!used[i]) {
            used[i] = true;
            ret = &packetPool[i];
            bufferInit(&ret->buffer, bufferPool[i], BUFFER_SIZE);
//            ret->sendTries = 0;
//            ret->ackTime = 0;
            break;
        }
    }
#endif
    return ret;
}

void queueFreePacket(QueuedPacket_t *p) {
    ASSERT(packetCount);
    --packetCount;
#if DYNAMIC_ALLOCATION
    memFree(p->buffer.data);
    memFree(p);
#else
    uint8_t i;
    for (i = 0; i < MAC_PROTOCOL_QUEUE_SIZE; ++i) {
        if (p == &packetPool[i]) {
            used[i] = false;
            break;
        }
    }
#endif
}

int8_t queueAddPacket(MacInfo_t *mi, const uint8_t *data, uint16_t length,
                      bool replace, QueuedPacket_t **result) {
    QueuedPacket_t *p = newQpacket(replace);
    if (!p) {
        PRINTF("queueAddPacket: queue is full!\n");
        return -ENOMEM;
    }
    if (bufferWrite(&p->buffer, mi->macHeader, mi->macHeaderLen)
            || bufferWrite(&p->buffer, data, length)) {
        PRINTF("queueAddPacket: buffer too short!\n");
        return -ENOMEM;
    }
    lock();
    STAILQ_INSERT_TAIL(&packetQueue, p, chain);
    unlock();
    if (result) *result = p;
    return 0;
}

QueuedPacket_t *queueHead() {
    return STAILQ_FIRST(&packetQueue);
}

void queuePop() {
    QueuedPacket_t *p = STAILQ_FIRST(&packetQueue);
    ASSERT(p);
    // PRINTF("queuePop\n");
    lock();
    STAILQ_REMOVE_HEAD(&packetQueue, chain);
    unlock();
    queueFreePacket(p);
}

void queueForEachPacket(QpacketProcessFn fn) {
    QueuedPacket_t *p;
//    PRINTF("queueForEachPacket\n");
    STAILQ_FOREACH(p, &packetQueue, chain) fn(p);
}

QueuedPacket_t *queueGetPacket(QpacketMatchFn fn, void *userData) {
    QueuedPacket_t *p;
    STAILQ_FOREACH(p, &packetQueue, chain) if (fn(p, userData)) return p;
    return NULL;
}

QueuedPacket_t *queueRemovePacket(QpacketMatchFn fn, void *userData) {
    QueuedPacket_t *ret;
//    PRINTF("queueRemovePacket\n");
    STAILQ_REMOVE_IF(&packetQueue, ret, chain, fn(__t, userData));
    return ret;
}

// void queueRemovePacketByPtr(QueuedPacket_t *p) {
//     QueuedPacket_t *t;
//     STAILQ_REMOVE_IF(&packetQueue, t, chain, __t == p);
// }
