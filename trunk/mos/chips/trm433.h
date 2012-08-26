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

#ifndef MANSOS_TRM433_H
#define MANSOS_TRM433_H

//-------------------------------------------
// Driver for TRM433-LT radio
//-------------------------------------------

#include <digital.h>
#include "adc.h"

// constants, which should be moved to platform specific code

// TRM radio connection pins
// DATA pin attached to P2.3
#define TRM433_DATA_PORT 2
#define TRM433_DATA_PIN 3

#define TRM433_DATA_PORT_ADDR_IN P2IN
#define TRM433_DATA_PORT_ADDR_OUT P2OUT

// T/R SEL pin attached to P2.1
#define TRM433_TRSEL_PORT 2
#define TRM433_TRSEL_PIN 1

// Power Down (PDN) pin attached to P2.0
#define TRM433_PDN_PORT 2
#define TRM433_PDN_PIN 0

// ADC channel to which RSSI pin is attached
#define TRM433_RSSI_CH 6


// platform-independent routines

#define TRM433_INIT() \
    pinAsData(TRM433_PDN_PORT, TRM433_PDN_PIN); \
    pinAsOutput(TRM433_PDN_PORT, TRM433_PDN_PIN); \
    pinAsData(TRM433_TRSEL_PORT, TRM433_TRSEL_PIN); \
    pinAsOutput(TRM433_TRSEL_PORT, TRM433_TRSEL_PIN); \
    pinAsData(TRM433_DATA_PORT, TRM433_DATA_PIN); \

#define TRM433_TX_MODE() \
    pinAsOutput(TRM433_DATA_PORT, TRM433_DATA_PIN); \
    pinSet(TRM433_TRSEL_PORT, TRM433_TRSEL_PIN);

#define TRM433_RX_MODE() \
    pinAsInput(TRM433_DATA_PORT, TRM433_DATA_PIN); \
    pinClear(TRM433_TRSEL_PORT, TRM433_TRSEL_PIN);

#define TRM433_ON() pinSet(TRM433_PDN_PORT, TRM433_PDN_PIN)
#define TRM433_OFF() pinClear(TRM433_PDN_PORT, TRM433_PDN_PIN)

#define TRM433_READ_DATA() (!pinRead(TRM433_DATA_PORT, TRM433_DATA_PIN))
#define TRM433_CLEAR_DATA() pinClear(TRM433_DATA_PORT, TRM433_DATA_PIN)
#define TRM433_SET_DATA() pinSet(TRM433_DATA_PORT, TRM433_DATA_PIN)
#define TRM433_TOGGLE_DATA() pinToggle(TRM433_DATA_PORT, TRM433_DATA_PIN)
#define TRM433_WRITE_DATA(bit) pinWrite(TRM433_DATA_PORT, TRM433_DATA_PIN, bit)

#define TRM433_READ_RSSI() adcRead(TRM433_RSSI_CH)

// supposedly faster version of TRM433_READ_DATA()
#define TRM433_READ_DATA_FAST() \
    (TRM433_DATA_PORT_ADDR_IN & (1 << TRM433_DATA_PIN))

// faster version of TRM433_WRITE_DATA()
#define TRM433_WRITE_DATA_FAST(bit) do {        \
        if (bit) TRM433_DATA_PORT_ADDR_OUT |= (1 << TRM433_DATA_PIN);   \
        else TRM433_DATA_PORT_ADDR_OUT &= ~(1 << TRM433_DATA_PIN);      \
    } while (0)

#endif // !MANSOS_TRM433

