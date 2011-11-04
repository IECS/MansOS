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

#include "mansos.h"
#include "leds.h"
#include "usart.h"
#include "smp.h"
#include "radio.h"
#include "dev_snum.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#ifdef DEBUG
void DPRINT(const char *format, ...) {
    char buffer[100];
    va_list ap;
    uint16_t len;

    va_start(ap, format);
    vsnprintf(buffer, sizeof(buffer), format, ap);
    va_end(ap);
    len = strlen(buffer);

    USARTSendByte(1, PROTOCOL_DEBUG);
    USARTSendByte(1, len >> 8);
    USARTSendByte(1, len & 0xFF);
    USARTSendData(1, buffer, len);
}
#endif // DEBUG

bool getMoteType(bool set, uint8_t oidLen, SmpOid_t oid,
                 SmpVariant_t *arg, SmpVariant_t *response) {
    if (set) {
        DPRINT("getMoteType: read only!");
        return false;
    }
    response->type = ST_OCTET;
#ifdef PLATFORM_TELOSB
    response->u.octet = SMP_TMOTE_SKY;
#else
#error implement this...
#endif
    return true;
}

bool getIeeeAddress(bool set, uint8_t oidLen, SmpOid_t oid,
                    SmpVariant_t *arg, SmpVariant_t *response) {
    if (set) {
        DPRINT("getIeeeAddress: read only!");
        return false;
    }

    devParams_t params;
    devMgrErr_t ret;

    response->type = ST_UINTEGER64;
    params.data = (char *) &response->u.uint64;
    ret = devCall(DEV_SERIAL_NUMBER, 0, DMF_READ, &params);
    return ret == DME_SUCCESS;
}

bool processLedCommand(bool set, uint8_t oidLen, SmpOid_t oid,
                       SmpVariant_t *arg, SmpVariant_t *response) {
    if (set && !arg) {
        DPRINT("processLedCommand: argument not supplied");
        return false;
    }
    if (arg && arg->type != ST_OCTET) {
        DPRINT("processLedCommand: wrong arg type 0x%x", arg->type);
        return false;
    }

    devParams_t params;
    uint8_t led = oid[0];
    devMgrErr_t ret;

    if (set) {
        params.data = (char *) &arg->u.octet;
        ret = devCall(DEV_LEDS, led, DMF_WRITE, &params);
        if (ret != DME_SUCCESS) return false;
    }

    response->type = ST_OCTET;
    params.data = (char *) &response->u.octet;
    ret = devCall(DEV_LEDS, led, DMF_READ, &params);
    return ret == DME_SUCCESS;
}

bool processCommand(bool set, uint8_t command, uint8_t oidLen, SmpOid_t oid,
                    SmpVariant_t *arg, SmpVariant_t *response) {
//    DPRINT("process %s command %d", (set ? "set" : "get"), command);
    if (arg && arg->type == 0xFF) arg = NULL;
    switch (command) {
    case SMP_RES_TYPE:
        return getMoteType(set, oidLen, oid, arg, response);
    case SMP_RES_IEEE_ADDRESS:
        return getIeeeAddress(set, oidLen, oid, arg, response);
    case SMP_RES_LED:
        return processLedCommand(set, oidLen, oid, arg, response);
    default:
        DPRINT("command %d not implemented", command);
        break;
    }

    return false;
}

void appMain(void)
{
    USARTInit(1, 115200, 0);
    USARTEnableTX(1);
    USARTEnableRX(1);

    radioInit(); 

    proxyInit();

    // do nothing (XXX: should sleep instead)
    while (1);
}

