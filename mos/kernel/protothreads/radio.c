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

/*
 * Portions copyright (c) 2007, Swedish Institute of Computer Science
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*---------------------------------------------------------------------------*/
#include <process.h>
#include <pt-sem.h>
#include <radio.h>
#include <timing.h> // !!! for getJiffies
#include <net/radio_packet_buffer.h>

#if DEBUG
#define RADIO_DEBUG 1
#endif

#define RADIO_DEBUG 0

#if RADIO_DEBUG
#include <lib/dprint.h>
#define RPRINTF(...) PRINTF(__VA_ARGS__)
#else
#define RPRINTF(...) do {} while (0)
#endif

// !!!
// Not using net, therefore redefine radio packet buffer here

#ifndef RADIO_BUFFER_SIZE
#define RADIO_BUFFER_SIZE RADIO_MAX_PACKET
#endif

static struct RadioPacketBufferReal_s {
    uint8_t bufferLength;     // length of the buffer
    int8_t receivedLength;    // length of data stored in the packet, or error code if negative
    uint8_t buffer[RADIO_BUFFER_SIZE]; // a buffer where the packet is stored
} realBuf = {RADIO_BUFFER_SIZE, 0, {0}};

RadioPacketBuffer_t *radioPacketBuffer = (RadioPacketBuffer_t *) &realBuf;
// !!!



process_event_t radioEvent;

PROCESS(radio_process, "radio driver");
/*---------------------------------------------------------------------------*/

// Semaphore signaling radio packet reception
//struct pt_sem radioSem;

/*---------------------------------------------------------------------------*/
// Driver protothread
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(radio_process, ev, data)
{
  PROCESS_BEGIN();

//  PT_SEM_INIT(&radioSem, 0);
  radioEvent = process_alloc_event();

  RPRINTF("Started %s\n", radio_process.name);

  while(1) {
    PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);
    RPRINTF("%lu, %s received poll\n", getJiffies(), radio_process.name);

    radioBufferReset();
    // packetbuf_set_attr(PACKETBUF_ATTR_TIMESTAMP, last_packet_timestamp);
    static int len;
    len = radioRecv(radioPacketBuffer->buffer, radioPacketBuffer->bufferLength);
    radioPacketBuffer->receivedLength = len;

    // !!! Just to be sure, there is no "packet pending" which can cause
    // missing interrupt handling
    // radioDiscard();

    RPRINTF("%lu, radioRecv len = %i\n", getJiffies(), len);

    // send event to listeners (if any)
    if (len > 0) {
        // PT_SEM_INIT(&radioSem, 1);
        process_post(PROCESS_BROADCAST, radioEvent, &len);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
