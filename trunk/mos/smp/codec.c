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

#include "smp.h"
#include <string.h>

uint8_t decodeOctet(uint8_t **data, uint16_t *maxLen, uint8_t *result) {
    if (*maxLen == 0) {
        return RET_ERROR;
    }
    *result = **data;
    (*data)++;
    (*maxLen)--;
    return RET_SUCCESS;
}

uint8_t encodeOctet(uint8_t **data, uint16_t *maxLen, uint8_t value) {
    if (*maxLen == 0) {
        return RET_ERROR;
    }
    **data = value;
    (*data)++;
    (*maxLen)--;
    return RET_SUCCESS;
}

uint8_t decodeUint16(uint8_t **data, uint16_t *maxLen, uint16_t *result) {
    if (*maxLen < 2) {
        return RET_ERROR;
    }
    memcpy(result, *data, 2);
    *data += 2;
    *maxLen -= 2;
    return RET_SUCCESS;
}

uint8_t encodeUint16(uint8_t **data, uint16_t *maxLen, uint16_t value) {
    if (*maxLen < 2) {
        return RET_ERROR;
    }
    memcpy(*data, &value, 2);
    *data += 2;
    *maxLen -= 2;
    return RET_SUCCESS;
}

uint8_t decodeInt32(uint8_t **data, uint16_t *maxLen, int32_t *result) {
    uint8_t *p = *data;
    uint8_t len;

    if (*maxLen == 0) {
        return RET_ERROR;
    }

    if (!(*p & 0x80)) {
        len = 1;
    } else if ((*p & 0xC0) == 0x80) {
        len = 2;
        *p &= ~0x80;
    } else if ((*p & 0xE0) == 0xC0) {
        len = 3;
        *p &= ~0xC0;
    } else if ((*p & 0xF0) == 0xE0) {
        len = 4;
        *p &= ~0xE0;
    } else if ((*p & 0xF8) == 0xF0) {
        len = 5;
    } else {
        return RET_ERROR;
    }

    if (*maxLen < len) {
        return RET_ERROR;
    }
    *maxLen -= len;

    *result = 0;
    switch (len) {
    case 5:
        p++;
    case 4:
        *result = *p++;
    case 3:
        *result = (*result << 8) + *p++;
    case 2:
        *result = (*result << 8) + *p++;
    case 1:
        *result = (*result << 8) + *p++;
    }

    *data = p;
    return RET_SUCCESS;
}

uint8_t encodeInt32(uint8_t **data, uint16_t *maxLen, int32_t value) {
    uint8_t *p = *data;
    uint8_t len;

    if (*maxLen == 0) {
        return RET_ERROR;
    }

    if (value < 0x80) {
        len = 1;
    } else if (value < 0x4000) {
        len = 2;
        value |= 0x8000;
    } else if (value < 0x200000) {
        len = 3;
        value |= 0xC00000;
    } else if (value < 0x10000000) {
        len = 4;
        value |= 0xE0000000;
    } else {
        len = 4;
        (*maxLen)--;
        *p++ = 0xF0;
    }

    if (*maxLen < len) {
        return RET_ERROR;
    }
    *maxLen -= len;

    switch (len) {
    case 4:
        *p++ = value >> 24;
    case 3:
        *p++ = (value >> 16) & 0xFF;
    case 2:
        *p++ = (value >> 8) & 0xFF;
    case 1:
        *p++ = value & 0xFF;
    }

    *data = p;
    return RET_SUCCESS;
}

uint8_t decodeUint64(uint8_t **data, uint16_t *maxLen, uint64_t *result) {
    if (*maxLen < 8) {
        return RET_ERROR;
    }
    memcpy(result, *data, 8);
    *data += 8;
    *maxLen -= 8;
    return RET_SUCCESS;
}

uint8_t encodeUint64(uint8_t **data, uint16_t *maxLen, uint64_t value) {
    if (*maxLen < 8) {
        return RET_ERROR;
    }
    memcpy(*data, &value, 8);
    *data += 8;
    *maxLen -= 8;
    return RET_SUCCESS;
}

uint8_t decodeBinary(uint8_t **data, uint16_t *maxLen, uint8_t **result) {
    uint8_t len;
    if (*maxLen == 0) {
        return RET_ERROR;
    }
    len = **data;
    if (*maxLen < len) {
        return RET_ERROR;
    }
    *result = *data;
    *data += len;
    *maxLen -= len;
    return RET_SUCCESS;
}

uint8_t encodeBinary(uint8_t **data, uint16_t *maxLen, void *src, uint8_t len) {
    if (*maxLen + 1 < len) {
        return RET_ERROR;
    }
    // include prologue too!
    **data = SMP_ELEM_VALUE | ST_BINARY;
    (*data)++;
    memcpy(*data + 1, src, len);
    ++len;
    **data = len;
    *data += len;
    *maxLen -= len + 1;
    return RET_SUCCESS;
}

uint8_t decodeVariant(uint8_t **data, uint16_t *maxLen, SmpVariant_t *result) {
    if (*maxLen == 0) {
        return RET_ERROR;
    }
    result->type = **data & 0x3f;
    (*data)++;
    (*maxLen)--;
    switch (result->type) {
    case ST_OCTET:
        return decodeOctet(data, maxLen, &result->u.uint8);
    case ST_UINTEGER16:
        return decodeUint16(data, maxLen, &result->u.uint16);
    case ST_INTEGER:
        return decodeInt32(data, maxLen, &result->u.int32);
    case ST_UINTEGER:
        return decodeInt32(data, maxLen, (int32_t *) &result->u.uint32);
    case ST_UINTEGER64:
        return decodeUint64(data, maxLen, &result->u.uint64);
    case ST_BINARY:
        return decodeBinary(data, maxLen, &result->u.data);
    }
    return RET_SUCCESS;
}

uint8_t encodeVariant(uint8_t **data, uint16_t *maxLen, SmpVariant_t *value) {
    if (*maxLen == 0) {
        return RET_ERROR;
    }
    **data = SMP_ELEM_VALUE | value->type;
    (*data)++;
    (*maxLen)--;
    switch (value->type) {
    case ST_OCTET:
        return encodeOctet(data, maxLen, value->u.uint8);
    case ST_UINTEGER16:
        return encodeUint16(data, maxLen, value->u.uint16);
    case ST_INTEGER:
        return encodeInt32(data, maxLen, value->u.int32);
    case ST_UINTEGER:
        return encodeInt32(data, maxLen, value->u.uint32);
    case ST_UINTEGER64:
        return encodeUint64(data, maxLen, value->u.uint64);
#if 0
    case ST_BINARY:
        break;
#endif
    }
    return RET_SUCCESS;
}
