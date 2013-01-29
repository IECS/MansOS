/*
 * Copyright (c) 2013 the MansOS team. All rights reserved.
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

#ifndef PC_RADIO_HAL_H
#define PC_RADIO_HAL_H

#include <defines.h>

void pcRadioInit(void);
void pcRadioReinit(void);
int8_t pcRadioSendHeader(const void *header, uint16_t headerLength,
                         const void *data, uint16_t dataLength);
int16_t pcRadioRecv(void *buffer, uint16_t buffLen);
void pcRadioDiscard(void);
RadioRecvFunction pcRadioSetReceiveHandle(RadioRecvFunction functionHandle);
void pcRadioOn(void);
void pcRadioOff(void);
int pcRadioGetRSSI(void);
int8_t pcRadioGetLastRSSI(void);
uint8_t pcRadioGetLastLQI(void);
void pcRadioSetChannel(int channel);
void pcRadioSetTxPower(uint8_t power);
bool pcRadioIsChannelClear(void);

#endif
