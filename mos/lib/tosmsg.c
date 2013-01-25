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

#include "byteorder.h"
#include "tosmsg.h"
#include <string.h>

enum { TOS_SERMSG_META_SIZE = sizeof(TosSerialMsg_t) 
     - sizeof(TosSerialMsgPayload_t) };

uint16_t tosSerialMsgEnc(void *dataBuf, uint16_t dataSize, 
        uint16_t dstAddr, uint8_t amType, void *resultBuf, 
        uint16_t resultSize, uint16_t *bytesWritten) {
    uint16_t n = dataSize;
    if (n > MAX_TOS_MSG_PAYLOAD) n = MAX_TOS_MSG_PAYLOAD;
    if (n > resultSize - TOS_SERMSG_META_SIZE) n = resultSize - TOS_SERMSG_META_SIZE;
    if (resultSize < TOS_SERMSG_META_SIZE + 1) n = 0;
    
    if (n > 0) {
        TosSerialMsg_t *msg = (TosSerialMsg_t *) resultBuf;
        msg->zero = 0;
        be16Write(&msg->dstAddr, dstAddr);
        msg->srcAddr[0] = 0;
        msg->srcAddr[1] = 0;
        msg->msgLen = n;
        msg->group = 0;
        msg->amType = amType;
        memcpy(&msg->payload, dataBuf, n);
        if (bytesWritten) *bytesWritten = n + TOS_SERMSG_META_SIZE;
    } else {
        if (bytesWritten) *bytesWritten = 0;
    }
    
    return n;
}
