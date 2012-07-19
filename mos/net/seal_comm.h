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

// some default field codes (also in file seal/components.py)
#define PACKET_FIELD_ID_COMMAND   0
#define PACKET_FIELD_ID_SEQNUM    1
#define PACKET_FIELD_ID_TIMESTAMP 2
#define PACKET_FIELD_ID_ADDRESS   3

//
// Each packet must have this header; data fields (if any) follow it.
//
struct SealHeader_s {
    uint16_t magic;
    uint16_t crc;
    uint32_t typeMask;
} PACKED;
typedef struct SealHeader_s SealHeader_t;

struct SealPacket_s {
    SealHeader_t header;
    uint32_t fields[1];
} PACKED;
typedef struct SealPacket_s SealPacket_t;

typedef void (*CallbackFunction)(uint32_t *packet);

//
// Register a listener to multiple sensors (determined by typemask).
// Multiple listeners are possible (determined by different callbacks!)
//
bool sealCommPacketRegisterInterest(uint32_t typemask, CallbackFunction callback,
                                    uint32_t *buffer);

//
// Register a listener to specific sensor (determined by sensor code).
// Multiple listeners are possible (determined by different callbacks!)
//
static inline bool sealCommRegisterInterest(uint16_t code, CallbackFunction callback) {
    return sealCommPacketRegisterInterest(1 << code, callback, NULL);
}

//
// Unregister a listener
//
bool sealCommPacketUnregisterInterest(uint32_t typemask, CallbackFunction callback);

//
// Unregister a listener to a single event
//
static inline bool sealCommUnregisterInterest(uint16_t code, CallbackFunction callback) {
    return sealCommPacketUnregisterInterest(1 << code, callback);
}

//
// Start building a SEAL packet
//
void sealCommPacketStart(SealPacket_t *buffer);

//
// Add a value to SEAL packet
//
void sealCommPacketAddField(uint16_t code, uint32_t value);

//
// End building & send a SEAL packet
//
void sealCommPacketFinish(void);

//
// Shortcut function: send a SEAL packet with specific sensor ID
//
void sealCommSendValue(uint16_t code, uint32_t value);

//
// Shortcut function: send a SEAL packet with command
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
// System & private API

void sealCommInit(void);

typedef struct SealCommListener_s {
    uint32_t typeMask;
    CallbackFunction callback;
    union {
        uint32_t *buffer;
        uint32_t lastValue;
    } u;
//    uint32_t values[1];
} SealCommListener_t;

#ifndef MAX_SEAL_LISTENERS
#define MAX_SEAL_LISTENERS 32
#endif

#endif
