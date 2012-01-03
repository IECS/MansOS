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

#include <stdmansos.h>
#include <lib/list.h>
#include <lib/buffer.h>
#include <lib/assert.h>
#include <lib/algo.h>
#include <arch_mem.h>
#include <kernel/threads/mutex.h>

#define BUFFER_SIZE      130

typedef struct QueuedPacket_s {
    STAILQ_ENTRY(QueuedPacket_s) chain;
    Buffer_t buffer;
    int i;
} QueuedPacket_t;

static QueuedPacket_t *newQpacket(void) {
    QueuedPacket_t *ret = memAlloc(sizeof(*ret));
    bufferInit(&ret->buffer, memAlloc(BUFFER_SIZE), BUFFER_SIZE);
    return ret;
}

static void freeQpacket(QueuedPacket_t *p) {
    memFree(p->buffer.data);
    memFree(p);
}

void test1() {
    STAILQ_HEAD(tailhead, QueuedPacket_s) head;

    QueuedPacket_t *n1, *n2, *np;

    // ---------------------- init
    STAILQ_INIT(&head);

    // ---------------------- insert
    n1 = newQpacket();
    STAILQ_INSERT_HEAD(&head, n1, chain);

    n1 = newQpacket();
    STAILQ_INSERT_TAIL(&head, n1, chain);

    n2 = newQpacket();
    STAILQ_INSERT_AFTER(&head, n1, n2, chain);

    // ---------------------- remove
    np = STAILQ_FIRST(&head);
    STAILQ_REMOVE_HEAD(&head, chain);
    freeQpacket(np);
    while (head.stqh_first) {
        np = STAILQ_FIRST(&head);
        STAILQ_REMOVE(&head, head.stqh_first, QueuedPacket_s, chain);
        freeQpacket(np);
    }
}

void test2() {
    STAILQ_HEAD(h, QueuedPacket_s) workQueue, userQueue;
    QueuedPacket_t *p;
    Mutex_t mutex;

    STAILQ_INIT(&workQueue);
    STAILQ_INIT(&userQueue);
    mutexInit(&mutex);

    ASSERT(STAILQ_COUNT(&workQueue, chain) == 0);

    // insert in nextQueue
    p = newQpacket();
    STAILQ_INSERT_TAIL(&userQueue, p, chain);
    ASSERT(STAILQ_COUNT(&userQueue, chain) == 1);
    p = newQpacket();
    STAILQ_INSERT_TAIL(&userQueue, p, chain);
    p = newQpacket();
    STAILQ_INSERT_TAIL(&userQueue, p, chain);
    ASSERT(STAILQ_COUNT(&userQueue, chain) == 3);

    p = newQpacket();
    STAILQ_INSERT_TAIL(&workQueue, p, chain);
    p = newQpacket();
    STAILQ_INSERT_TAIL(&workQueue, p, chain);

    // workQueue at the moment contains only old unsent stuff.
    // append contents of userQueue to the end of workQueue
    // and clear userQueue
    mutexLock(&mutex);
    STAILQ_MERGE(&workQueue, &userQueue);
    STAILQ_INIT(&userQueue);
    mutexUnlock(&mutex);
    ASSERT(STAILQ_COUNT(&workQueue, chain) == 5);

    // remove some from workQueue (simulate sending)
    {
        uint16_t i = 0;
        QueuedPacket_t **p = &workQueue.stqh_first;
        while (*p) {
            QueuedPacket_t *t = *p;
            if (!(i & 0x1)) {
                // remove p
                QueuedPacket_t *next = t->chain.stqe_next;
                t->chain.stqe_next = NULL;
                *p = next;
                freeQpacket(t);
            } else {
                // skip p
                p = &(*p)->chain.stqe_next;
            }
            ++i;
        }
        // update the queue struct
        workQueue.stqh_last = p;
    }
    ASSERT(STAILQ_COUNT(&workQueue, chain) == 2);

    // insert some in userQueue
    p = newQpacket();
    STAILQ_INSERT_TAIL(&userQueue, p, chain);
    p = newQpacket();
    STAILQ_INSERT_TAIL(&userQueue, p, chain);
    ASSERT(STAILQ_COUNT(&userQueue, chain) == 2);

    // merge both queues
    mutexLock(&mutex);
    STAILQ_MERGE(&workQueue, &userQueue);
    STAILQ_INIT(&userQueue);
    mutexUnlock(&mutex);

    ASSERT(STAILQ_COUNT(&workQueue, chain) == 4);

    {
        int i = 0;
        STAILQ_FOREACH(p, &workQueue, chain) p->i = i++;
    }
    STAILQ_REMOVE_IF(&workQueue, p, chain, __t->i == 2);
    ASSERT(p);
    ASSERT(p->i == 2);
    ASSERT(STAILQ_COUNT(&workQueue, chain) == 3);

    STAILQ_REMOVE_IF(&workQueue, p, chain, __t->i == 3);
    ASSERT(p);
    ASSERT(p->i == 3);
    ASSERT(STAILQ_COUNT(&workQueue, chain) == 2);

    STAILQ_REMOVE_IF(&workQueue, p, chain, __t->i == 5);
    ASSERT(!p);
    ASSERT(STAILQ_COUNT(&workQueue, chain) == 2);

    STAILQ_REMOVE_IF(&workQueue, p, chain, __t->i == 0);
    ASSERT(p);
    ASSERT(p->i == 0);
    ASSERT(STAILQ_COUNT(&workQueue, chain) == 1);

    STAILQ_REMOVE_IF(&workQueue, p, chain, __t->i == 1);
    ASSERT(p);
    ASSERT(p->i == 1);
    ASSERT(STAILQ_COUNT(&workQueue, chain) == 0);
}

void appMain(void)
{
    test1();
    test2();

    // ---------------------- signal that all was ok
    for (;;) {
        msleep(1000);
        redLedToggle();
    }
}
