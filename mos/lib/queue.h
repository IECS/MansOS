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


// Implementation of a simple (circular) queue data structure
// Objects are arbitrary data structures


#ifndef MANSOS_QUEUE_H
#define MANSOS_QUEUE_H

#include "stdtypes.h"

// Queue item object type.
// Use your own item type and cast to QItem_t as needed.

typedef void QItem_t;

// Queue_t data structure for a circular queue
typedef struct {
    QItem_t **data;
    int length;

    int head;
    int tail;
} Queue_t;

// Initialize queue with a new buffer. Length must denote the size of the buffer
#define queueInit( queue, buffer, buf_length ) \
    queue->data = (QItem_t **)buffer; \
    queue->length = buf_length;      \
    queueReset( queue );             \


// Reset queue, consider it empty.
#define queueReset( queue )           \
    queue->head = 0; \
    queue->tail = 0; \
    queue->data[queue->tail] = NULL;  \


// Return true if the queue is empty
#define queueIsEmpty( queue ) \
    ((queue->head == queue->tail) && (queue->data[queue->head] == NULL))


#define queueIsFull( queue ) \
    ((queue->head == queue->tail) && (queue->data[queue->head] != NULL))


// Add item to queue. Return true on success
bool queueEnq( Queue_t * queue, QItem_t *item );

// Remove item from the queue and return. Return NULL on empty.
QItem_t * queueDeq( Queue_t * queue );

#endif      // MANSOS_QUEUE_H
