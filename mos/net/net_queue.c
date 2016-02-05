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

#include "net_queue.h"
#include <assert.h>
#include <print.h>
#include <errors.h>
#include <mutex.h>

PacketQueue_t packetQueue;

static Mutex_t mutex;
#define lock()     mutexLock(&mutex) 
#define unlock()   mutexUnlock(&mutex)

// -----------------------------------------------------

void netQueueInit(void) {
    STAILQ_INIT(&packetQueue);
}

int8_t netQueueAddPacket(MacInfo_t *mi, const uint8_t *data, uint16_t length,
                      QueuedPacket_t *result) {
    // QueuedPacket_t *p = newQpacket(replace);
    if (!result || result->isUsed) {
        PRINTF("netQueueAddPacket: queue is full!\n");
        return -ENOMEM;
    }
    result->isUsed = true;
    memcpy(result->data, mi->macHeader, mi->macHeaderLen);
    memcpy(result->data + mi->macHeaderLen, data, length);

    lock();
    STAILQ_INSERT_TAIL(&packetQueue, result, chain);
    unlock();
    return 0;
}

void netQueuePop() {
    QueuedPacket_t *p = STAILQ_FIRST(&packetQueue);
    ASSERT(p);
    ASSERT(p->isUsed);
    p->isUsed = false;
    // PRINTF("netQueuePop\n");
    lock();
    STAILQ_REMOVE_HEAD(&packetQueue, chain);
    unlock();
}

void netQueueForEachPacket(QpacketProcessFn fn) {
    QueuedPacket_t *p;
    STAILQ_FOREACH(p, &packetQueue, chain) fn(p);
}

QueuedPacket_t *netQueueGetPacket(QpacketMatchFn fn, void *userData) {
    QueuedPacket_t *p;
    STAILQ_FOREACH(p, &packetQueue, chain) if (fn(p, userData)) return p;
    return NULL;
}

QueuedPacket_t *netQueueRemovePacket(QpacketMatchFn fn, void *userData) {
    QueuedPacket_t *ret;
    STAILQ_REMOVE_IF(&packetQueue, ret, chain, fn(__t, userData));
    if (ret) ret->isUsed = false;
    return ret;
}
