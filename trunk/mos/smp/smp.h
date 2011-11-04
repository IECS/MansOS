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

#ifndef MANSOS_SMP_H
#define MANSOS_SMP_H

#include <kernel/defines.h>

typedef enum {
    SMP_PACKET_REQUEST,
    SMP_PACKET_RESPONSE,
    SMP_PACKET_WALK,
} SmpPacketType_t;

typedef enum {
    SMP_META,
    SMP_MOS,

    TOTAL_SMP_TOPLEV,
} SmpTopLevelId_e;

// META OIDs
typedef enum {
    SMP_META_TIMESTAMP,
    SMP_META_REPEAT_INTERVAL,
    SMP_META_REPEAT_TIMES,

    TOTAL_SMP_META_OIDS,
} SmpMetaId_e;

// MOS resources (TODO: make this to match codes in devman)
typedef enum {
    SMP_RES_TYPE,         // platform (e.g. Tmote Sky)
    SMP_RES_ADDRESS,      // 2-byte address
    SMP_RES_IEEE_ADDRESS, // 8-byte address
    SMP_RES_LED,
    SMP_RES_RADIO,
    SMP_RES_USART,
    SMP_RES_SENSOR,
    SMP_RES_SOFTWARE,
    SMP_RES_MEMORY,
    SMP_RES_FLASH,
    SMP_RES_UPTIME,
    SMP_RES_BINARY_PACKET, // input packet type handled by user code

    TOTAL_SMP_RES_GROUPS,
} SmpResourceGroupId_e;

typedef enum {
    SMP_MOTE_TELOSB,
    SMP_MOTE_MSP430,
    SMP_MOTE_EPIC,
    SMP_MOTE_ATMEGA,
    SMP_MOTE_NRF,
    SMP_MOTE_PC,
    SMP_MOTE_OTHER,
} SmpMoteType_e;

typedef enum {
    SMP_ELEM_COMMAND    = 0 << 6,
    SMP_ELEM_OID        = 1 << 6,
    SMP_ELEM_OID_PREFIX = 2 << 6,
    SMP_ELEM_VALUE      = 3 << 6,
} SmpElementType_e;

typedef enum {
    SMP_COMMAND_GET,
    SMP_COMMAND_SET,
} SmpCommand_e;

typedef enum {
    SMP_LED_RED,
    SMP_LED_GREEN,
    SMP_LED_BLUE,
} SmpLedNum_e;

typedef enum {
    SMP_SENSOR_TSR,
    SMP_SENSOR_PAR,
    SMP_SENSOR_VOLTAGE,
    SMP_SENSOR_HUMIDITY,
    SMP_SENSOR_TEMPERATURE,
} SmpSensorType_e;

#define MAX_OID_LEN 64

typedef uint8_t *SmpOid_t;

typedef enum {
    ST_ERROR,      // one byte, error code
    ST_OCTET,      // one byte
    ST_UINTEGER16, // 2 bytes
    ST_INTEGER,    // 1-5 bytes
    ST_UINTEGER,   // same as ST_INTEGER, bu interpreted as unsigned
    ST_UINTEGER64, // 8 bytes
    ST_BINARY,     // blob
} SmpType_e;

typedef struct {
    uint8_t type;
    union {
        uint8_t uint8;
        uint16_t uint16;
        int32_t int32;
        uint32_t uint32;
        uint64_t uint64;
        uint8_t *data;
    } u;
} SmpVariant_t;

void smpRecv(uint8_t *data, uint16_t len);

#ifdef DEBUG
void smpDebugPrint(const char *format, ...);
#else
static inline void smpDebugPrint(const char *format, ...)
{
}
#endif

#define SERIAL_PACKET_DELIMITER '$'

#define PROTOCOL_DEBUG 'd'
#define PROTOCOL_SMP   's'

#define RET_SUCCESS 0
#define RET_ERROR   0xFF

// this function must be defined by application
bool processCommand(bool doSet, uint8_t command, uint8_t oidLen, SmpOid_t oid,
                    SmpVariant_t *arg, SmpVariant_t *response);

uint8_t encodeOctet(uint8_t **data, uint16_t *maxLen, uint8_t value);
uint8_t encodeUint16(uint8_t **data, uint16_t *maxLen, uint16_t value);
uint8_t encodeInt32(uint8_t **data, uint16_t *maxLen, int32_t value);
uint8_t encodeUint64(uint8_t **data, uint16_t *maxLen, uint64_t value);
uint8_t encodeBinary(uint8_t **data, uint16_t *maxLen, void *src, uint8_t len);
uint8_t encodeVariant(uint8_t **data, uint16_t *maxLen, SmpVariant_t *value);

uint8_t decodeOctet(uint8_t **data, uint16_t *maxLen, uint8_t *result);
uint8_t decodeUint16(uint8_t **data, uint16_t *maxLen, uint16_t *result);
uint8_t decodeInt32(uint8_t **data, uint16_t *maxLen, int32_t *result);
uint8_t decodeUint64(uint8_t **data, uint16_t *maxLen, uint64_t *result);
uint8_t decodeBinary(uint8_t **data, uint16_t *maxLen, uint8_t **result);
uint8_t decodeVariant(uint8_t **data, uint16_t *maxLen, SmpVariant_t *result);

void smpProxyRecvSerial(uint8_t *data, uint16_t len);
void smpProxySendSmp(uint8_t *data, uint16_t len);

void smpSerialReceive(uint8_t x);

void smpInit(void);

#define SMP_BAUDRATE  38400

#endif
