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

/// @file
/// MansOS network addressing
///

#ifndef MANSOS_ADDRESS_H
#define MANSOS_ADDRESS_H

#include <stdtypes.h>

// MAC Frame format:
// +-----+----------------------+---------+
// | FCF | FCF-specified fields | payload |
// +-----+----------------------+---------+
// all fields are optional.

// Frame Control Field (FCF) flags
#define FCF_SRC_ADDR_NONE   0x00
#define FCF_SRC_ADDR_SHORT  0x01
#define FCF_SRC_ADDR_LONG   0x02

#define FCF_DST_ADDR_NONE   0x00
#define FCF_DST_ADDR_SHORT  0x03
#define FCF_DST_ADDR_LONG   0x06
// +----------------------+-------------------------------------------+
// | address combination  |   usage                                   |
// +----------------------+-------------------------------------------+
// | no src, no dst       | broadcast from gateway                    |
// | no src, dst short    | gateway to initialised node               |
// | no src, dst long     | ? (gateway to uninitialised node)         |
// | src short, no dst    | bcast from initialised non-gateway node   |
// | src short, dst short | typical - e.g. unicast to gateway         |
// | src short, dst long  | gateway to uninitialised node             |
// | src long, no dst     | ? (broadcast from uninitialised node)     |
// | src long, dst short  | uninitialised node to gateway             |
// | src long, dst long   | [invalid]                                 |
// +----------------------+-------------------------------------------+
// NB: SRC_ADDR_LONG + DST_ADDR_LONG is an invalid combination,
// since it does not fit in 3 bits.

#define FCF_IMMED_SRC       0x08
#define FCF_IMMED_DST       0x10
#define FCF_SEQNUM          0x20
#define FCF_COST            0x40
#define FCF_EXTENDED        0x80

// Extended FCF flags (more to come...)
#define FCF_EXT_AUTH_TYPE_NONE  0x00
#define FCF_EXT_AUTH_TYPE_1     0x01
#define FCF_EXT_AUTH_TYPE_2     0x02
#define FCF_EXT_AUTH_TYPE_3     0x03

#define FCF_EXT_SRC_PORT        0x04
#define FCF_EXT_DST_PORT        0x08
#define FCF_EXT_HOPLIMIT        0x10
// there are more data
//#define FCF_EXT_MORE            0x20
// this packet is an acknowledgement
#define FCF_EXT_IS_ACK          0x40

enum {
    MOS_ADDR_TYPE_SHORT,
    MOS_ADDR_TYPE_LONG,
};

#define MOS_LONG_ADDR_SIZE        8
#define MOS_SHORT_ADDR_SIZE       2

#define MOS_ADDR_BROADCAST        0xffff
#define MOS_ADDR_ROOT             0x0000 // as destination, filled in by network stack
#define MOS_ADDR_BASESTATION      0x0001 // for now, BS always has this address

//----------------------------------------------------------
// Types
//----------------------------------------------------------

typedef uint16_t MosShortAddr;
typedef uint8_t MosLongAddr[MOS_LONG_ADDR_SIZE];

typedef struct MosAddr_s {
#if SUPPORT_LONG_ADDR
    uint8_t type;
#endif
    union {
        MosShortAddr saddr;
#if SUPPORT_LONG_ADDR
        MosLongAddr laddr;
#endif
    } u;
} MosAddr;

#define shortAddr u.saddr
#define longAddr u.laddr

//----------------------------------------------------------
// Variables
//----------------------------------------------------------

// This is the local short address (unspecified by default).
// NB: local long address = the serial number
extern MosShortAddr localAddress;

//----------------------------------------------------------
// Functions
//----------------------------------------------------------

#if SUPPORT_LONG_ADDR
#define intToAddr(result, ia) do {             \
        result.type = MOS_ADDR_TYPE_SHORT;     \
        result.shortAddr = ia;                 \
    } while (0)

#define isBroadcast(addr)                      \
    ((addr)->type == MOS_ADDR_TYPE_SHORT       \
            && ((addr)->shortAddr & 0x8000))   \

#define isUnspecified(addr)                    \
    ((addr)->type == MOS_ADDR_TYPE_SHORT       \
            && ((addr)->shortAddr == 0))       \

#else

#define intToAddr(result, ia)                  \
    result.shortAddr = ia;

#define isBroadcast(addr)                      \
    ((addr)->shortAddr & 0x8000)

#define isUnspecified(addr)                    \
    ((addr)->shortAddr == 0)


#endif // SUPPORT_LONG_ADDR

#endif
