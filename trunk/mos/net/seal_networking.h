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

#ifndef MANSOS_SEAL_NETWORKING_H
#define MANSOS_SEAL_NETWORKING_H

/// \file
/// SEAL networking public API
///

#include <radio.h>

//! The magic code at start of all SEAL data packets
#ifndef SEAL_MAGIC
#define SEAL_MAGIC    0x5EA1 // "SEAl"
#endif

//! The data port used for SEAL packets
#ifndef SEAL_DATA_PORT
#define SEAL_DATA_PORT 123
#endif

//
// Some default field codes (also defined in file seal/components.py).
// All codes belong pseudo sensors. Real sensor codes follow,
// but they are platform (and application?) specific,
// and cannot be declared here.
//
#define PACKET_FIELD_ID_COMMAND   0 // remote command
#define PACKET_FIELD_ID_SEQNUM    1 // sequence numne
#define PACKET_FIELD_ID_TIMESTAMP 2 // time stamp
#define PACKET_FIELD_ID_ADDRESS   3 // short network address
#define PACKET_FIELD_ID_IS_SENT   4 // false if the measurement
                                    // is in local storage only

#define COMMAND_TYPE_MASK   (1 << PACKET_FIELD_ID_COMMAND)
#define SEQUENCENUMBER_TYPE_MASK  (1 << PACKET_FIELD_ID_SEQNUM)
#define TIMESTAMP_TYPE_MASK (1 << PACKET_FIELD_ID_TIMESTAMP)
#define ADDRESS_TYPE_MASK   (1 << PACKET_FIELD_ID_ADDRESS)
#define ISSENT_TYPE_MASK    (1 << PACKET_FIELD_ID_IS_SENT)

//! Each SEAL packet must have this header; data fields (if any) follow it.
struct SealHeader_s {
    uint16_t magic;
    uint16_t crc;
    uint32_t typeMask;
} PACKED;
typedef struct SealHeader_s SealHeader_t;

//! SEAL data packet
struct SealPacket_s {
    SealHeader_t header;
    int32_t fields[1];
} PACKED;
typedef struct SealPacket_s SealPacket_t;

typedef void (*MultiValueCallbackFunction)(int32_t *packet);
typedef void (*SingleValueCallbackFunction)(uint16_t code, int32_t value);

//
// Register a listener to multiple sensors (determined by typemask).
// Multiple listeners are possible (determined by different callbacks!)
//
bool sealNetPacketRegisterInterest(uint32_t typemask,
                                   MultiValueCallbackFunction callback,
                                   int32_t *buffer);

//
// Register a listener to specific sensor (determined by sensor code).
// Multiple listeners are possible (determined by different callbacks!)
//
bool sealNetRegisterInterest(uint16_t code,
                             SingleValueCallbackFunction callback);

//
// Unregister a listener
//
bool sealNetPacketUnregisterInterest(uint32_t typemask,
                                     MultiValueCallbackFunction callback);

//
// Unregister a listener to a single event
//
bool sealNetUnregisterInterest(uint16_t code,
                               SingleValueCallbackFunction callback);

//
// Start building a SEAL packet
//
void sealNetPacketStart(SealPacket_t *buffer);

//
// Add a value to SEAL packet
//
void sealNetPacketAddField(uint16_t code, int32_t value);

//
// End building & send a SEAL packet
//
void sealNetPacketFinish(void);

//
// Shortcut function: send a SEAL packet with specific sensor ID
//
void sealNetSendValue(uint16_t code, int32_t value);

//
// Shortcut function: send a SEAL packet with command
//
static inline void sealNetSendCommand(uint16_t command)
{
    sealNetSendValue(PACKET_FIELD_ID_COMMAND, command);
}

//
// Read last received sensor value (identified by code only)
//
int32_t sealNetReadValue(uint16_t code);

// ----------------------------------
// System & private API

void sealNetInit(void);

//! SEAL networking packet subscriber
typedef struct SealNetListener_s {
    uint32_t typeMask;
    union {
        SingleValueCallbackFunction sv;
        MultiValueCallbackFunction mv;
    } callback;
    union {
        // a read buffer
        int32_t *buffer;
        // in place (room for one...)
        int32_t lastValue;
    } u;
} SealNetListener_t;

//! The maximal number of SEAL data packet subscribers
#ifndef MAX_SEAL_LISTENERS
#define MAX_SEAL_LISTENERS 32
#endif

#endif
