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

#ifndef MANSOS_RADIO_HAL_H
#define MANSOS_RADIO_HAL_H

//===========================================================
// Data types and constants
//===========================================================
enum {
    PROXY_SERVER_PORT = 6293, // TCP port where sockets subscribe to cloud
    MAX_PACKET_SIZE = 1024,
};

typedef uint16_t PcRadioPackSize_t;

extern RadioRecvFunction radioCallback;
extern int mosSendSock;

// data received from cloud stored here
extern unsigned char pcRadioBuf[MAX_PACKET_SIZE];
extern uint16_t pcRadioBufLen; // bytes stored in pcRadioBuffer
extern uint8_t pcRadioOn;

#ifndef min
#define min(a, b)  ((a) < (b) ? (a) : (b))
#endif

#define RADIO_MAX_PACKET     0xffff
#define RADIO_TX_POWER_MIN        0
#define RADIO_TX_POWER_MAX        0

#endif
