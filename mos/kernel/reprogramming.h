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

#ifndef MANSOS_REPROGRAMMING_H
#define MANSOS_REPROGRAMMING_H

#include <kernel/boot.h>

#define REPROGRAMMING_DATA_CHUNK_SIZE 64u // bytes

//typedef enum ReprogrammingAddress {
//    RA_DST_LOCAL = 0x00,         // for local use only
//    RA_DST_EVERYONE = 0xff,      // for all nodes in the network
//    // an network address otherwise
//} ReprogrammingAddress_t;

typedef void __attribute__((noreturn)) (*ApplicationStartExec)();

typedef enum {
    RPROG_PACKET_START,
    RPROG_PACKET_CONTINUE,
    RPROG_PACKET_REBOOT,
} BinarySmpPacketType_t;

struct ReprogrammingStartPacket_s {
    uint8_t type;                // packet type
    uint8_t __reserved;
    uint16_t imageId;            // identification number
    uint32_t extFlashAddress;    // in which to store the image
    uint16_t imageBlockCount;    // number of blocks
    // uint16_t destinationAddress; // ReprogrammingAddress_t
} PACKED;

struct ReprogrammingContinuePacket_s {
    uint8_t type;                // packet type
    uint8_t __reserved;
    uint16_t imageId;            // image identification number
    uint16_t blockId;            // block number (starting from 0)
    uint16_t address;            // internal flash address of this data[] chunk
    uint8_t data[REPROGRAMMING_DATA_CHUNK_SIZE];
    uint16_t crc;                // 2 byte checksum of data
} PACKED;

struct RebootCommandPacket_s {
    uint8_t type;                // packet type
    uint8_t doReprogram;         // if nonzero: reprogram internal flash after reboot
    uint16_t intFlashAddress;    // to-be-programmed internal flash address
    uint32_t extFlashAddress;    // image location on external flash
} PACKED;

typedef struct ReprogrammingStartPacket_s ReprogrammingStartPacket_t;
typedef struct ReprogrammingContinuePacket_s ReprogrammingContinuePacket_t;
typedef struct RebootCommandPacket_s RebootCommandPacket_t;

// this must be same as end of ReprogrammingContinuePacket_s
struct ExternalFlashBlock_s
{
    // internal flash address of this chunk.
    // if the lowest bit is set, it's the first block in this internal flash segment
    uint16_t address;
    // image data
    uint8_t data[REPROGRAMMING_DATA_CHUNK_SIZE];
    // 2 byte checksum of address & data
    uint16_t crc;
} PACKED;

typedef struct ExternalFlashBlock_s ExternalFlashBlock_t;

void processRSPacket(ReprogrammingStartPacket_t *);
bool processRCPacket(ReprogrammingContinuePacket_t *);
void processRebootCommand(RebootCommandPacket_t *);

#endif
