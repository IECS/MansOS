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

//-------------------------------------------
//      TinyOS Serial message encapsulation test
//-------------------------------------------

#include "mansos.h"
#include "dprint.h"
#include "scheduler.h"
#include "tosmsg.h"

//-------------------------------------------
// constants
//-------------------------------------------
enum { 
    SMALL_PAYLOAD_SIZE = 2,
    LARGE_PAYLOAD_SIZE = 81,

    SMALL_TOSBUF_SIZE = 9, 
    LARGE_TOSBUF_SIZE = 120,

    SLEEP_TIME = 1000
};


//-------------------------------------------
// types
//-------------------------------------------
typedef enum { SMALL = 0, LARGE = 1 } Size_t;


//-------------------------------------------
// variables
//-------------------------------------------
static uint16_t smallPayload[SMALL_PAYLOAD_SIZE];
static uint16_t largePayload[LARGE_PAYLOAD_SIZE];
static uint16_t tosBufSmall[SMALL_TOSBUF_SIZE];
static uint16_t tosBufLarge[LARGE_TOSBUF_SIZE];

// bytes processed and written
static uint16_t processed;
static uint16_t written;

// tmp
static uint16_t *buf;
static uint16_t *pay;
static uint16_t bufSize;
static uint16_t paySize;
static uint16_t offset;


//-------------------------------------------
// functions
//-------------------------------------------
void testEnc(Size_t payload, Size_t buffer);


//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    PRINT_INIT(128);
    uint16_t i;
    for (i = 0; i < SMALL_PAYLOAD_SIZE; ++i) {
        smallPayload[i] = i + 1;
    }
    for (i = 0; i < LARGE_PAYLOAD_SIZE; ++i) {
        largePayload[i] = i + 1;
    }
    
    while(1)
        {
            testEnc(SMALL, SMALL);
            testEnc(SMALL, LARGE);
            testEnc(LARGE, SMALL);
            testEnc(LARGE, LARGE);
        }
}

void testEnc(Size_t payload, Size_t buffer) {
    PRINTF("--- encoding ");
    if (payload == SMALL) { 
        PRINTF("small payload (%u)", sizeof(smallPayload));
        pay = smallPayload;
        paySize = SMALL_PAYLOAD_SIZE * 2;
    } else {
        PRINTF("large payload (%u)", sizeof(largePayload));
        pay = largePayload;
        paySize = LARGE_PAYLOAD_SIZE * 2;
    }
    PRINTF(" into a ");
    if (buffer == SMALL) {
        PRINTF("small buffer (%u):\n", sizeof(tosBufSmall));
        buf = tosBufSmall;
        bufSize = SMALL_TOSBUF_SIZE * 2;
    } else {
        PRINTF("large buffer (%u):\n", sizeof(tosBufLarge));
        buf = tosBufLarge;
        bufSize = LARGE_TOSBUF_SIZE * 2;
    }
    offset = 0;
    do {
        processed = tosSerialMsgEnc(pay + (offset >> 1), paySize, 
                0x1234, 67, buf, bufSize, &written);
        PRINTF("offset = %u, bytes processed: %u, written: %u\n", 
                offset, processed, written);
        if (written) debugHexdump((uint8_t *) buf, written);
        paySize -= processed;
        offset += processed;
    } while (paySize && processed);
    PRINTF("\n");
    threadSleep(SLEEP_TIME);
}
