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

//-----------------------------------------------------------------------------
// Serial Pipe - read data at 115200 from USART0, forward it to USART1 at 38400
// Works only on platforms with at least two UARTs
//-----------------------------------------------------------------------------

#include "stdmansos.h"

#define BUF_SIZE 128
#define RECV_TIMEOUT 200


static uint8_t buf[BUF_SIZE];
static uint8_t bufPos = 0;
static Alarm_t printAlarm;

void printBuf(void *userData);
void usart0Recv(uint8_t b);

// store bytes received from UART0 in a buffer, set alarm - when no data
// received for X milliseconds, forward buffer content to UART1
void usart0Recv(uint8_t b) {
    greenLedToggle();
    if (bufPos < BUF_SIZE - 1) {
        buf[bufPos++] = b;
        blueLedToggle();
    }
    alarmSchedule(&printAlarm, RECV_TIMEOUT);
}

void doPrintBuffer(void *userData) {
    if (bufPos > 0) {
        Handle_t h;
        ATOMIC_START(h);
        buf[bufPos++] = 0;
        atomic(PRINTF("%s", buf));
        bufPos = 0;
        ATOMIC_END(h);
    }
}

//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    alarmInit(&printAlarm, doPrintBuffer, NULL);
    PRINT_INIT(256); // inits UART1 automatically
    if (USARTInit(0, 115200, 0)) redLedOn();
    if (USARTEnableRX(0)) redLedOn();
    USARTSetReceiveHandle(0, usart0Recv);
}
