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

#include "mac.h"
#include <lib/assert.h>
#include <lib/unaligned.h>
#include <lib/byteorder.h>
#include <print.h>
#include <stdlib.h>
#include <errors.h>
#include <kernel/threads/mutex.h>

static Mutex_t macMutex;

#define MAX_MAC_HEADER_LEN  30
static uint8_t headerBuffer[MAX_MAC_HEADER_LEN]; // XXX: watch out for size

int8_t macSend(MosAddr *dst, const uint8_t *data, uint16_t length) {
    static MacInfo_t mi;
    memset(&mi, 0, sizeof(mi));
    fillLocalAddress(&mi.originalSrc);
    if (dst) {
        memcpy(&mi.originalDst, dst, sizeof(MosAddr));
    } else {
        mi.originalDst.shortAddr = MOS_ADDR_BROADCAST;
    }
    return macSendEx(&mi, data, length);
}

int8_t macSendEx(MacInfo_t *mi, const uint8_t *data, uint16_t length) {
    int8_t ret;

    // this function is not reentrant, because as single buffer is used for all mac headers
    mutexLock(&macMutex);

    ASSERT(mi);
    if (!mi->macHeaderLen) {
        // do this only if the mac header was not provided by the user
        if (!macProtocol.buildHeader(mi, &mi->macHeader, &mi->macHeaderLen)) {
            ret = -EINVAL;
            goto end;
        }
    }
    ret = macProtocol.send(mi, data, length);
  end:
    mutexUnlock(&macMutex);
    return ret;
}

bool defaultBuildHeader(MacInfo_t *mi, uint8_t **header /* out */,
                        uint16_t *headerLength /* out */) {
    uint8_t *p = headerBuffer;
    uint8_t fcf1 = 0, fcf2 = 0; // FCF flags: byte 1 and 2

    if (mi->srcPort) fcf2 |= FCF_EXT_SRC_PORT;
    if (mi->dstPort) fcf2 |= FCF_EXT_DST_PORT;
    if (mi->hoplimit) fcf2 |= FCF_EXT_HOPLIMIT;
    if (mi->flags & MI_FLAG_IS_ACK) fcf2 |= FCF_EXT_IS_ACK;

    ++p;
// always include this...
//    if (fcf2) {
        fcf1 |= FCF_EXTENDED;
        ++p;
//    }

#if SUPPORT_LONG_ADDR
    if (mi->originalSrc.type == MOS_ADDR_TYPE_SHORT) {
        fcf1 += FCF_SRC_ADDR_SHORT;
        putU16(p, htons(mi->originalSrc.shortAddr));
        p += MOS_SHORT_ADDR_SIZE;
    } else {
        fcf1 += FCF_SRC_ADDR_LONG;
        memcpy(p, mi->originalSrc.longAddr, MOS_LONG_ADDR_SIZE);
        p += MOS_LONG_ADDR_SIZE;
    }
    if (mi->originalDst.type == MOS_ADDR_TYPE_SHORT) {
        fcf1 += FCF_DST_ADDR_SHORT;
        putU16(p, htons(mi->originalDst.shortAddr));
        p += MOS_SHORT_ADDR_SIZE;
    } else {
        if (mi->originalSrc.type != MOS_ADDR_TYPE_SHORT) return false;
        fcf1 += FCF_DST_ADDR_LONG;
        memcpy(p, mi->originalDst.longAddr, MOS_LONG_ADDR_SIZE);
        p += MOS_LONG_ADDR_SIZE;
    }
    if (mi->immedDst.type != MOS_ADDR_TYPE_SHORT
            || mi->immedSrc.type != MOS_ADDR_TYPE_SHORT) return false;
#else
    // src
    fcf1 += FCF_SRC_ADDR_SHORT;
    putU16(p, htons(mi->originalSrc.shortAddr));
    p += MOS_SHORT_ADDR_SIZE;
    // dst
    fcf1 += FCF_DST_ADDR_SHORT;
    putU16(p, htons(mi->originalDst.shortAddr));
    p += MOS_SHORT_ADDR_SIZE;
#endif
    if (mi->immedSrc.shortAddr) {
        fcf1 |= FCF_IMMED_SRC;
        putU16(p, htons(mi->immedSrc.shortAddr));
        p += MOS_SHORT_ADDR_SIZE;
    }
    if (mi->immedDst.shortAddr) {
        fcf1 |= FCF_IMMED_DST;
        putU16(p, htons(mi->immedDst.shortAddr));
        p += MOS_SHORT_ADDR_SIZE;
    }
    if (mi->seqnum) {
        fcf1 |= FCF_SEQNUM;
        *p++ = mi->seqnum;
    }
    if (mi->cost) {
        fcf1 |= FCF_COST;
        *p++ = mi->cost;
    }
    if (mi->srcPort) {
        *p++ = mi->srcPort;
    }
    if (mi->dstPort) {
        *p++ = mi->dstPort;
    }
    if (mi->hoplimit) {
        *p++ = mi->hoplimit;
    }
    headerBuffer[0] = fcf1;
    if (fcf2) headerBuffer[1] = fcf2;

    *header = headerBuffer;
    *headerLength = p - headerBuffer;

    return true;
}

static uint8_t calcMacHeaderLen(uint8_t fcf1, uint8_t fcf2) {
    uint8_t result;

    result = 1;

    switch ((fcf1 & 0x7) % 3) {
    case FCF_SRC_ADDR_NONE:
        break;
    case FCF_SRC_ADDR_SHORT:
        result += MOS_SHORT_ADDR_SIZE;
        break;
    case FCF_SRC_ADDR_LONG:
        result += MOS_LONG_ADDR_SIZE;
        break;
    }
    switch ((fcf1 & 0x7) / 3 * 3) {
    case FCF_DST_ADDR_NONE:
        break;
    case FCF_DST_ADDR_SHORT:
        result += MOS_SHORT_ADDR_SIZE;
        break;
    case FCF_DST_ADDR_LONG:
        result += MOS_LONG_ADDR_SIZE;
        break;
    }

    if (fcf1 & FCF_IMMED_SRC) result += MOS_SHORT_ADDR_SIZE;
    if (fcf1 & FCF_IMMED_DST) result += MOS_SHORT_ADDR_SIZE;
    if (fcf1 & FCF_SEQNUM) ++result;
    if (fcf1 & FCF_COST) ++result;
    if (fcf1 & FCF_EXTENDED) {
        ++result;
        if (fcf2 & FCF_EXT_SRC_PORT) ++result;
        if (fcf2 & FCF_EXT_DST_PORT) ++result;
        if (fcf2 & FCF_EXT_HOPLIMIT) ++result;
    }

    return result;
}

uint8_t *defaultParseHeader(uint8_t *data, uint16_t length, MacInfo_t *mi /* out */) {
    uint8_t fcf1, fcf2;
    uint8_t macHeaderLen;
    uint8_t *p;

    // XXX: recv len can be 1, but then it's invalid anyway...
    fcf1 = data[0];
    fcf2 = data[1];
    macHeaderLen = calcMacHeaderLen(fcf1, fcf2);

    if (macHeaderLen > length) {
        PRINTF("MAC parseHeader: too short frame (%d required, %d received)\n",
                macHeaderLen, length);
        return NULL;
    }

    p = data + ((fcf1 & FCF_EXTENDED) ? 2 : 1);
    memset(mi, 0, sizeof(*mi));

    switch ((fcf1 & 0x7) % 3) {
    case FCF_SRC_ADDR_NONE:
        break;
    case FCF_SRC_ADDR_SHORT:
#if SUPPORT_LONG_ADDR
        mi->originalSrc.type = MOS_ADDR_TYPE_SHORT;
#endif
        mi->originalSrc.shortAddr = ntohs(getU16(p));
        p += MOS_SHORT_ADDR_SIZE;
        break;
#if SUPPORT_LONG_ADDR
    case FCF_SRC_ADDR_LONG:
        mi->originalSrc.type = MOS_ADDR_TYPE_LONG;
        memcpy(mi->originalSrc.longAddr, p, MOS_LONG_ADDR_SIZE);
        p += MOS_LONG_ADDR_SIZE;
        break;
#endif
    }

    switch ((fcf1 & 0x7) / 3 * 3) {
    case FCF_DST_ADDR_NONE:
        break;
    case FCF_DST_ADDR_SHORT:
#if SUPPORT_LONG_ADDR
        mi->originalDst.type = MOS_ADDR_TYPE_SHORT;
#endif
        mi->originalDst.shortAddr = ntohs(getU16(p));
        p += MOS_SHORT_ADDR_SIZE;
        break;
#if SUPPORT_LONG_ADDR
    case FCF_DST_ADDR_LONG:
        mi->originalDst.type = MOS_ADDR_TYPE_LONG;
        memcpy(mi->originalDst.longAddr, p, MOS_LONG_ADDR_SIZE);
        p += MOS_LONG_ADDR_SIZE;
        break;
#endif
    }

    if (fcf1 & FCF_IMMED_SRC) {
        mi->immedSrc.shortAddr = ntohs(getU16(p));
        p += MOS_SHORT_ADDR_SIZE;
    }
    if (fcf1 & FCF_IMMED_DST) {
        mi->immedDst.shortAddr = ntohs(getU16(p));
        p += MOS_SHORT_ADDR_SIZE;
    }
    if (fcf1 & FCF_SEQNUM) {
        mi->seqnum = *p++;
    }
    if (fcf1 & FCF_COST) {
        mi->cost = *p++;
    }
    if (fcf1 & FCF_EXTENDED) {
        if (fcf2 & FCF_EXT_SRC_PORT) mi->srcPort = *p++;
        if (fcf2 & FCF_EXT_DST_PORT) mi->dstPort = *p++;
        if (fcf2 & FCF_EXT_HOPLIMIT) mi->hoplimit = *p++;
        if (fcf2 & FCF_EXT_IS_ACK) mi->flags |= MI_FLAG_IS_ACK;
    }

    mi->macHeader = data;
    mi->macHeaderLen = p - data;

    return p;
}

bool defaultIsKnownDstAddress(MosAddr *dst) {
    return true;
}

uint8_t getMacHeaderSeqnum(uint8_t *data)
{
    uint8_t offset = 1;
    uint8_t fcf1 = data[0];
    if (fcf1 & FCF_EXTENDED) offset++;

    switch ((fcf1 & 0x7) % 3) {
    case FCF_SRC_ADDR_NONE:
        break;
    case FCF_SRC_ADDR_SHORT:
        offset += MOS_SHORT_ADDR_SIZE;
        break;
    case FCF_SRC_ADDR_LONG:
        offset += MOS_LONG_ADDR_SIZE;
        break;
    }
    switch ((fcf1 & 0x7) / 3 * 3) {
    case FCF_DST_ADDR_NONE:
        break;
    case FCF_DST_ADDR_SHORT:
        offset += MOS_SHORT_ADDR_SIZE;
        break;
    case FCF_DST_ADDR_LONG:
        offset += MOS_LONG_ADDR_SIZE;
        break;
    }

    if (fcf1 & FCF_IMMED_SRC) offset += MOS_SHORT_ADDR_SIZE;
    if (fcf1 & FCF_IMMED_DST) offset += MOS_SHORT_ADDR_SIZE;

    return data[offset];
}

void invertDirection(MacInfo_t *mi)
{
    MosAddr tmp;
    uint8_t tmpPort;

    tmp = mi->originalSrc;
    mi->originalSrc = mi->originalDst;
    mi->originalDst = tmp;

    tmp = mi->immedSrc;
    mi->immedSrc = mi->immedDst;
    mi->immedDst = tmp;

    tmpPort = mi->srcPort;
    mi->srcPort = mi->dstPort;
    mi->dstPort = tmpPort;
}
