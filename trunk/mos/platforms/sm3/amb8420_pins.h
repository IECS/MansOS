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

#ifndef AMB8420_PINS_H
#define AMB8420_PINS_H

#include <digital.h>

#define AMB8420_UART_ID     0

// the only platform so far where this is so
#define RADIO_ON_UART0      1

#define AMB8420_SERIAL_BAUDRATE  9600

// when low: restart the module
#define AMB8420_RESET_PORT  4
#define AMB8420_RESET_PIN   4

// on falling edge: switch to config mode
#define AMB8420_CONFIG_PORT 4
#define AMB8420_CONFIG_PIN  2

// when high: enter low power mode
#define AMB8420_SLEEP_PORT  4
#define AMB8420_SLEEP_PIN   5

// when high: switch off Tx. XXX: needed?
#define AMB8420_TRX_DISABLE_PORT  4
#define AMB8420_TRX_DISABLE_PIN   3

// on falling edge: take the data from UART buffer and send out wirelessly
#define AMB8420_DATA_REQUEST_PORT 4
#define AMB8420_DATA_REQUEST_PIN  0

// input.
// when low: ready to send.
// when high: UART buffer is full, or wireless reception is detected
#define AMB8420_RTS_PORT  4
#define AMB8420_RTS_PIN   1

// input;
// falling edge signals that valid frame is received via wireless
#define AMB8420_DATA_INDICATE_PORT  1
#define AMB8420_DATA_INDICATE_PIN   7

#endif
