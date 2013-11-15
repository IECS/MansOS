#ifndef SAD_COMMON_H
#define SAD_COMMON_H

#include "stdmansos.h"
#include <string.h>
#include <net/socket.h>
#include <net/routing.h>
#include <lib/codec/crc.h>
#include <assert.h>

#define MANSOS
#include "data_packet.h"

#define DATA_PORT        123
//#define TIME_SYNC_PORT 124

// #define DATA_INTERVAL_JIFFIES      (5ul * 60 * HZ)
// #define TIME_SYNC_INTERVAL_JIFFIES (5ul * 60 * HZ)
//#define DATA_INTERVAL_JIFFIES         (HZ)
//#define TIME_SYNC_INTERVAL_JIFFIES  (5ul * HZ)

#define DATA_INTERVAL (1000ul * 60 * 5)  // -- once in every five minutes
//#define DATA_INTERVAL       (5000ul)
#define DATA_INTERVAL_SMALL (5000ul * 1)

#define EXT_FLASH_RESERVED  (256 * 1024ul) // 256kb

// #define ADS_CONVERSION_REGISTER 0
// #define ADS_CONFIG_REGISTER 1

#endif
