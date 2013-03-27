/*
 * Copyright (c) 2011, Institute of Electronics and Computer Science
 * All rights reserved.
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

#ifndef _PLATFORM_FARMMOTE_H_
#define _PLATFORM_FARMMOTE_H_

#include <msp430/msp430_clock.h>
#include <msp430/msp430_timers.h>
#include <msp430/msp430_int.h>
#include <msp430/msp430_adc10.h>
#include <msp430/msp430_usci.h>

#include "flash.h"

#define RADIO_CHIP RADIO_CHIP_CC1101
#define SNUM_CHIP  SNUM_DS2411

// The actual chip is SHT10
#define SHT11_VER  4  // FIXME: We don't know whether it's v3 or v4!
#define SHT11_VCC  35 // 3.5 V

//===========================================================
// Functions
//===========================================================

void initPlatform(void);

//===========================================================
// Data types and constants
//===========================================================

#define DS2411_PORT 4
#define DS2411_PIN  5

#define DS2411_VCC_PORT 4
#define DS2411_VCC_PIN  6

// What is this?
#define VDD_SWITCH_PORT 2
#define VDD_SWITCH_PIN  3

#define DS18B20_PORT 2
#define DS18B20_PIN  4

// ADC channel for the soil humidity sensor
#define ADC_SOIL_HUMIDITY 12

// Analog input enable bit for the soil humidity sensor input
#define SOIL_HUMIDITY_AE_BIT  4 // In ADC10AE1



// serial pins, for sw serial (TODO: check if correct!)
#define UART0_TX_PORT 3
#define UART0_TX_PIN  4
#define UART0_RX_PORT 3
#define UART0_RX_PIN  5

#define UART1_TX_PORT 3
#define UART1_TX_PIN  6
#define UART1_RX_PORT 3
#define UART1_RX_PIN  7



#endif
