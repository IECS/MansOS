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

//--------------------------------------------------------------------------------
// MansOS memory block management
// Base taken from Mantis: http://mantis.cs.colorado.edu/index.php/tiki-index.php
//--------------------------------------------------------------------------------

#include <stdlib.h>
#include "mem.h"

//----------------------------------------------------------
// types
//----------------------------------------------------------
/** @brief A node of managed memory */
typedef struct Node_s {
   /** @brief Size of blocks */
   uint16_t size;
   /** @brief Linked list of pointers */
   struct Node_s *prev;
   /** @brief Pointer to ned node */
   struct Node_s *next;
} Node_t;

//--------------------------------------------------------------------------------
// variables
//--------------------------------------------------------------------------------
static Node_t *freelist; // List head

//--------------------------------------------------------------------------------
// functions
//--------------------------------------------------------------------------------
inline void flagBlock(Node_t *n);
inline void combine(Node_t *n1, Node_t *n2);

void memoryInit(void *region, uint16_t size)
{
    /*set up the initial free list with one region*/
    freelist = (Node_t *)region;
    freelist->size = size - sizeof(Node_t);
    freelist->prev = freelist;
    freelist->next = freelist;
}

void *memoryAlloc(uint16_t size)
{
    Node_t *current;
    Node_t *best;

    if (freelist == NULL) {
        // No free memory available
        return NULL;
    }

    current = freelist;
    best = NULL;

    if (size % 2 != 0) {
        //need to increase size for word alignment
        size++;
    }

    // Run through free list and find best fitting block.
    do
    {
        if (current->size >= size)
        {
            if (best == NULL) {
                best = current;
            } else if (current->size < best->size) {
                best = current;
            }
        }
        current = current->next;
    } while (current != freelist);

    // if we could not find a block, return NULL
    if (best == NULL) {
        return NULL;
    }

    // Determine if we should cut a piece off of this free region or return
    //   the whole thing if it isn't large enough to do so.
    if (best->size <= size+sizeof(Node_t)) // Just return this region
    {
        // Pull the node out of the list.
        if (best->prev == best) {
            freelist = NULL;     // Last free node...
        } else {
            best->prev->next = best->next;
            best->next->prev = best->prev;
        }

        // If we just removed the front node, then we have to set freelist
        //   to the next node
        if (best == freelist) {
            freelist = best->next;
        }

        flagBlock(best); // Fill the block for later analysis

        return (uint8_t *)best + sizeof(Node_t);
    }
    else // Cut a piece off the end of this block to create a new node.
    {
        // Pull the new node off the best fitting one.
        current = (Node_t *)((uint8_t *)best + best->size - size);
        best->size -= size + sizeof(Node_t);
        current->size = size;

        flagBlock(current); // Fill the block for later analysis
        return (uint8_t *)current + sizeof(Node_t);
    }
}

/** @brief Flag a block of memory with 0xEF
 *
 * Fill a block of memory with 0xEF so it's easier to do analysis later on the usage.
 * @param n Block to fill
 */
inline void flagBlock(Node_t *n)
{
    uint16_t index;
    uint8_t *ptr;

    ptr = (uint8_t *)n + sizeof(Node_t);

    for (index = 0; index < (n->size); index++)
        ptr[index] = 0xEF;
}

void memoryFree(void* block)
{
    Node_t *current;

    // Get the region's header.
    Node_t *region = (Node_t *) ((uint8_t *)block - sizeof(Node_t));

    if (freelist == NULL) { // This is now the only free memory
        freelist = region;
        region->next = region;
        region->prev = region;
    } else {
        /* Put the free'd node onto the freelist.  The list is organized
           by each node's physical location in memory. */
        current = freelist;
        while (current < region) {
            current = current->next;
            if (current == freelist) {
                break;
            }
        }

        /* Now current is the node immediately after the one to insert. */
        region->next = current;
        region->prev = current->prev;
        current->prev->next = region;
        current->prev = region;

        /* If this is now the first block, we need to update freelist pointer. */
        if (region < freelist) {
            freelist = region;
        }
    }

    /* Check to see if we can combine blocks of memory since the new node
      could be physically adjacent to it's neighbors. */
    if ((uint8_t *)region->next ==
        (uint8_t *)(region) + region->size + sizeof(Node_t))  {
        // Next is adjacent
        combine(region, region->next);
    }

    if ((uint8_t *)region->prev ==
        (uint8_t *)(region) - region->prev->size - sizeof(Node_t)) {
        // Previous adjacent
        combine(region->prev, region);
    }
}

/** @brief Combine two adjacent blocks(node_ts)
 *
 * NOTE: n1 must precede n2 both physically and in the freelist.
 * @param n1 First block
 * @param n2 Second block
 */
inline void combine(Node_t *n1, Node_t *n2) {
    /*remove n2 from the free list*/
    n1->next = n2->next;
    n2->next->prev = n1;

    /*adjust size of n1*/
    n1->size = n1->size + n2->size + sizeof(Node_t);
}
