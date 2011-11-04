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

#ifndef MANSOS_CONFIG_H
#define MANSOS_CONFIG_H

#include "stdtypes.h"

// default config

#ifndef CFG_DATA_READ_INTERVAL
#define CFG_DATA_READ_INTERVAL             0
#endif
#ifndef CFG_DATA_STORE_LOCALLY
#define CFG_DATA_STORE_LOCALLY             false
#endif
#ifndef CFG_MAC_TDMA_SUPERFRAME_INTERVAL
#define CFG_MAC_TDMA_SUPERFRAME_INTERVAL   (60*1000ul)
#endif
#ifndef CFG_MAC_TDMA_SLOT_COUNT
#define CFG_MAC_TDMA_SLOT_COUNT            8
#endif
#ifndef CFG_MAC_TDMA_IS_MASTER
#define CFG_MAC_TDMA_IS_MASTER             false
#endif
#ifndef CFG_MAC_TDMA_IS_BASE_STATION
#define CFG_MAC_TDMA_IS_BASE_STATION       false
#endif
#ifndef CFG_MAC_TDMA_SLAVE_NUMBER
#define CFG_MAC_TDMA_SLAVE_NUMBER          0
#endif
#ifndef CFG_RADIO_PRIMARY_CHANNEL
#define CFG_RADIO_PRIMARY_CHANNEL          26
#endif
#ifndef CFG_RADIO_PRIMARY_TX_POWER
#define CFG_RADIO_PRIMARY_TX_POWER         31
#endif
#ifndef CFG_RADIO_SECONDARY_CHANNEL
#define CFG_RADIO_SECONDARY_CHANNEL        11
#endif
#ifndef CFG_RADIO_SECONDARY_TX_POWER
#define CFG_RADIO_SECONDARY_TX_POWER       1
#endif
#ifndef CFG_SYSTEM_LOCATION
#define CFG_SYSTEM_LOCATION                0
#endif
#ifndef CFG_BASE_STATION_ADDR
#define CFG_BASE_STATION_ADDR              MAKE_U16('B', 'S')
#endif

#endif
