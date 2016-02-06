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

#include "queue.h"

// Add item to queue. Return true on success
bool queueEnq( Queue_t * queue, QItem_t *item_p )
{
    if( item_p==NULL ) return false;    // Do not accept NULL pointers as items
    if (queueIsFull(queue)) return false;

    queue->data[ queue->head++ ] = item_p;
    if(queue->head >= queue->length) queue->head = 0;

    return true;
}

// Remove item from the queue and return. Return NULL on empty.
QItem_t * queueDeq( Queue_t * queue )
{
    QItem_t * item_p = queue->data[queue->tail];
    if(item_p != NULL){
        queue->data[queue->tail] = NULL;
        queue->tail++;
        if(queue->tail>=queue->length) queue->tail = 0;
    }
    return item_p;
}
