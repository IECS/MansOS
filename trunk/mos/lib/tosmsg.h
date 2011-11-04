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

#ifndef MANSOS_TOSMSG_H
#define MANSOS_TOSMSG_H

#include <kernel/stdtypes.h>

// max bytes which can be encapsulated in one TinyOS message
enum { MAX_TOS_MSG_PAYLOAD = 28 };

typedef uint8_t TosSerialMsgPayload_t;

typedef struct {
    uint8_t zero;
    uint16_t dstAddr __attribute__((packed)); 
    uint16_t srcAddr __attribute__((packed)); 
    uint8_t msgLen; 
    uint8_t group;
    uint8_t amType;
    TosSerialMsgPayload_t payload; // actually 0..28 bytes long
} TosSerialMsg_t;

/*
 * http://docs.tinyos.net/index.php/Mote-PC_serial_communication_and_SerialForwarder
 * 
 * TinyOS SerialMsg format :
 *     1B: Zero (0x0) 
 *     2B: Destination address
 *     2B: Link source address
 *     1B: Message length
 *     1B: Group ID
 *     1B: Active Message handler type
 * 0..28B: Payload, up to 28 bytes
 *
 * All values big-endian 
 */

/**
 * Encapsulate data in TinyOS serial message(s), which is/are used by 
 * SerialForwarder. Data is stored in resultBuf. ResultSize must point to an
 * If only n bytes can be encapsulated, first n bytes are used and
 * If bytesWritten is not NULL, byte count written in resultBuf is stored here. 
 * Encapsulated byte count is returned 
 */
uint16_t tosSerialMsgEnc(void *dataBuf, uint16_t dataSize, 
        uint16_t dstAddr, uint8_t amType, void *resultBuf, uint16_t resultSize,
        uint16_t *bytesWritten);

#endif /* MANSOS_TOSMSG_H */
