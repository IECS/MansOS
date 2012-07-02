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

#ifndef MANSOS_SEAL_COMM_H
#define MANSOS_SEAL_COMM_H

#include <hil/radio.h>

#ifndef SEAL_MAGIC
#define SEAL_MAGIC    0xABCD
#endif

// some default field codes
#define PACKET_FIELD_ID_COMMAND   0
#define PACKET_FIELD_ID_TIMESTAMP 1
#define PACKET_FIELD_ID_ADDRESS   2

//
// Each packet must have this header; data fields (if any) follow it.
//
struct SealHeader_s {
    uint16_t magic;
    uint16_t crc;
    uint32_t typeMask;
} PACKED;
typedef struct SealHeader_s SealHeader_t;

// use uint32_t to hold all integral types (string and other types are not supported)
typedef void (*CallbackFunction)(uint32_t value);

//
// Register a listener to specific sensor (determined by code).
// Multiple listeners are possible (determined by different callbacks!)
//
bool sealCommRegisterInterest(uint16_t code, CallbackFunction callback);

//
// Unregister a listener
//
bool sealCommUnregisterInterest(uint16_t code, CallbackFunction callback);

//
// Send a seal packet with specific sensor ID
//
void sealCommSendValue(uint16_t code, uint32_t value);

//
// Send a seal packet with command
//
static inline void sealCommSendCommand(uint16_t command)
{
    sealCommSendValue(PACKET_FIELD_ID_COMMAND, command);
}

//
// Read last received sensor value (identified by code only)
//
uint32_t sealCommReadValue(uint16_t code);

// ----------------------------------
// system & private API

void sealCommInit(void);

typedef struct SealCommListener_s {
    uint16_t code;
    CallbackFunction callback;
    uint32_t lastValue;
} SealCommListener_t;

#ifndef MAX_SEAL_LISTENERS
#define MAX_SEAL_LISTENERS 32
#endif

#endif
