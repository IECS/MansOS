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

#ifndef MANSOS_ROUTING_H
#define MANSOS_ROUTING_H

/// \file
/// Routing protocol public API
///

#include "address.h"

//===========================================================
// Data types and constants
//===========================================================

// - All supported routing protocols- 

//! A simple distance vector (DV) routing protocol
#define ROUTING_PROTOCOL_DV  1
//! SAD routing protocol
#define ROUTING_PROTOCOL_SAD 2

//! Routing decision
typedef enum {
    //! Discard the packet
    RD_DROP,
    //! Receive the packet locally
    RD_LOCAL,
    //! Send/forward as unicast packet
    RD_UNICAST,
    //! Send/forward as broadcast packet, also receive locally
    RD_BROADCAST,
} RoutingDecision_e;

typedef uint16_t Seqnum_t;

//! Routing packet type
enum {
    ROUTING_INFORMATION,
    ROUTING_REQUEST,
};

//! Routing packet sender role
enum {
    //! The sender is a base station
    SENDER_BS,
    //! The sender forwards data between BS and motes / collectors
    SENDER_FORWARDER,
    //! The sender collects data from motes and passes to a forwarder
    SENDER_COLLECTOR,
    //! The sender is a non-forwarding mote (data source)
    SENDER_MOTE,
};

//! Routing information, sent out from base station, forwarded by nodes
struct RoutingInfoPacket_s {
    //! Always set to ROUTING_INFORMATION
    uint8_t packetType;
    //! The role of the sender 
    uint8_t senderType;
    //! Address of the base station / gateway
    MosShortAddr rootAddress;
    //! Distance from root in hops
    uint16_t hopCount;
    //! Sequence number
    Seqnum_t seqnum;
    //! The automatically allocated number of the mote this packet is addressed to
    uint16_t moteNumber;
    //! The time on the network's root node in milliseconds. Used for time sync
    uint64_t rootClockMs;
} PACKED;
typedef struct RoutingInfoPacket_s RoutingInfoPacket_t;

//! Routing information, sent out by all nodes except base station
typedef struct RoutingRequestPacket_s {
    //! Always set to ROUTING_REQUEST
    uint8_t packetType;
    //! The role of the sender
    uint8_t senderType;
} RoutingRequestPacket_t;

//! The destination port used by the MansOS routing protocol
#define ROUTING_PROTOCOL_PORT  112

#if 0
# define ROUTING_ORIGINATE_TIMEOUT    (60 * 1000ul)
# define ROUTING_REQUEST_TIMEOUT      (5 * 1000ul)
# define SAD_SUPERFRAME_LENGTH        524288ul // (10 * 60 * 1000ul)
# define ROUTING_INFO_VALID_TIME      (1600 * 1000ul)
// during this time, radio is never turned off on forwarders and collectors
# define NETWORK_STARTUP_TIME_SEC     (2 * 60 * 60) // 2h in seconds
#else // for testing
# define ROUTING_ORIGINATE_TIMEOUT    (5 * 1000ul)
# define ROUTING_REQUEST_TIMEOUT      (5 * 1000ul)
# define SAD_SUPERFRAME_LENGTH        32768ul // (30 * 1000ul)
# define ROUTING_INFO_VALID_TIME      (70 * 1000ul)
# define NETWORK_STARTUP_TIME_SEC     0
#endif

#define MOTE_INFO_VALID_TIME         (5 * SAD_SUPERFRAME_LENGTH)

#define TIMESLOT_LENGTH              1000
// timeslot is adjusted (to both ends) by this number
#define TIMESLOT_IMPRECISION         1000 // because time sync protocol has second graduality
#define TOTAL_LISTENING_TIME         (TIMESLOT_LENGTH + 2 * TIMESLOT_IMPRECISION)

#define ROUTING_REPLY_WAIT_TIMEOUT   1000 + TIMESLOT_IMPRECISION

// XXX: this must be chip-specific. For AMB8420 its quite large (all the serial comm)
#define RADIO_TX_TIME 0


//! The maximal number of nodes with the "mote" role serverd by a single collector
#ifndef MAX_MOTES
#define MAX_MOTES 22
#endif

#define MAX_HOP_COUNT 16

#if USE_ROLE_FORWARDER || USE_ROLE_COLLECTOR
// turn off radio, but only after two hours of uninterrupted listening
#define RADIO_OFF_ENERGSAVE() \
    if (getTimeSec() > NETWORK_STARTUP_TIME_SEC) radioOff()
#else
// on mote: always turn off radio
#define RADIO_OFF_ENERGSAVE() \
    radioOff()
#endif

//! The address of the root node
extern MosShortAddr rootAddress;

//===========================================================

void routingInit(void);

/// Route a packet
/// @param info  The header of the packet
RoutingDecision_e routePacket(MacInfo_t *info);

#endif
