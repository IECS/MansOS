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

#include "smp.h"
#include <serial.h>
#include <kernel/mansos.h>
#include <print.h>
#include <string.h>
#include <net/addr.h>

void smpRecv(uint8_t *data, uint16_t recvLen) {
    bool set = false;
    uint8_t oid[MAX_OID_LEN];
    uint8_t oidLen = 0;
    uint8_t oidPrefixLen = 0;

    static uint8_t sendBuffer[400];
    uint8_t *response = sendBuffer + 1; // leave space for length at the beginning
    uint16_t responseMaxLen = sizeof(sendBuffer) - 1;

    SmpVariant_t arg = { .type = 0xFF };
    SmpVariant_t resp;

    uint8_t *p = data;

#ifdef DEBUG
    // PRINTF("SMP recv quelque chose (%d bytes)\n", recvLen);
    // debugHexdump(data, recvLen);
#endif

    uint16_t packetLen = *p++;

    // check packet length
    // PRINTF("packetLen = %d", packetLen);
    if (packetLen > recvLen) {
        PRINTF("packetLen is longer than physical length (%d vs %d)\n",
                packetLen, recvLen);
        return;
    }

    // check packet type
    switch (*p) {
    case SMP_PACKET_REQUEST:
        break;
    case SMP_PACKET_RESPONSE:
        PRINTF("ignoring SMP response packet\n");
        return;
    default:
        PRINTF("unrecognized SMP packet type %d\n", *p);
        return;
    }
    ++p;

    // prepare response packet
    *response++ = SMP_PACKET_RESPONSE;
    responseMaxLen--;

    // parse the received packet
    for (; p < data + packetLen; ++p) {
        uint8_t len;
        bool doProcess = false;

        switch (*p & 0xc0) {
        case SMP_ELEM_COMMAND:
            // PRINTF("SMP_ELEM_COMMAND\n");
            switch (*p & 0x3F) {
            case SMP_COMMAND_GET:
                set = false;
                break;
            case SMP_COMMAND_SET:
                set = true;
                break;
            default:
                PRINTF("unrecognized command 0x%x\n", *p & 0x3F);
                break;
            }
            break;

        case SMP_ELEM_OID:
        case SMP_ELEM_OID_PREFIX:
            // PRINTF("SMP_ELEM_OID\n");
            len = *p & 0x3F;
            if (packetLen < len + 1) {
                PRINTF("element length too large %d, packetLen=%d\n", len, packetLen);
                return;
            }
            // include same field in response
            memcpy(response, p, len + 1);
            response += len + 1;
            responseMaxLen -= len + 1;

            if ((*p & 0xc0) == SMP_ELEM_OID) {
                // whole OID
                if (oidPrefixLen + oidLen > MAX_OID_LEN) {
                    PRINTF("OID too long\n");
                    return;
                }
                memcpy(oid + oidPrefixLen, p + 1, len);
                oidLen = len + oidPrefixLen;

                if (!set) doProcess = true;
            } else {
                // OID prefix
                oidPrefixLen = len;
                memcpy(oid, p + 1, oidPrefixLen);
            }

            p += len;
            break;

        case SMP_ELEM_VALUE:
            // PRINTF("SMP_ELEM_VALUE\n");
            if (decodeVariant(&p, (uint16_t *) &packetLen, &arg)) {
                return;
            }
            doProcess = true;
            break;
        }

        if (!doProcess) continue;

        // PRINTF("process a command\n");

        // process the command and write reply
        do {
            if (oidLen < 2) {
                PRINTF("ignoring command, oid too short\n");
                break;
            }
            if (oid[0] != SMP_MOS) {
                PRINTF("ignoring command for unknown oid[0]=0x%x\n", oid[0]);
                break;
            }
            if (!processCommand(set, oid[1], oidLen - 2, oid + 2, &arg, &resp)) {
                break;
            }
            if (encodeVariant(&response, &responseMaxLen, &resp)) {
                return;
            }
        } while (false);

        arg.type = 0xFF;
    }

    {
        // encode response length in send buffer (255 byte length supported!)
        uint8_t responseLen = response - sendBuffer;
        sendBuffer[0] = responseLen;
        // send response packet
        smpProxySendSmp(sendBuffer, responseLen);
    }
}
