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

#ifndef _PLATFORM_ZOLERTIA_Z1_H_
#define _PLATFORM_ZOLERTIA_Z1_H_

#include <msp430/msp430_clock.h>
#include <msp430/msp430_timers.h>
#include <msp430/msp430_int.h>
#include <msp430/msp430_adc.h>
#include <msp430/msp430_usci.h>

#include "amb8420_pins.h"

#include "cc2420_pins.h"
#include "sht_pins.h"
#include "adxl_config.h"

// define missing symbols: P1SEL2 and P5SEL2
#ifdef __IAR_SYSTEMS_ICC__
#ifndef P1SEL2_
#define P1SEL2_              (0x0041u)  /* Port 1 Selection 2*/
DEFC(   P1SEL2             , P1SEL2_)
#endif
#ifndef P5SEL2_
#define P5SEL2_              (0x0045u)  /* Port 5 Selection 2*/
DEFC(   P5SEL2             , P5SEL2_)
#endif
#else // __IAR_SYSTEMS_ICC__
#ifdef __GNUC__
#ifndef P1SEL2_
  #define P1SEL2_             0x0041  /* Port 1 Selection 2*/
  sfrb(P1SEL2, P1SEL2_);
#endif
#ifndef P5SEL2_
  #define P5SEL2_             0x0045  /* Port 5 Selection 2*/
  sfrb(P5SEL2, P5SEL2_);
#endif
#endif // __GNUC__
#endif // __IAR_SYSTEMS_ICC__

//===========================================================
// Functions
//===========================================================

void initPlatform(void);

//===========================================================
// Data types and constants
//===========================================================

//#define EXT_FLASH_CHIP FLASH_CHIP_AT25DF

#ifndef RADIO_CHIP
#define RADIO_CHIP RADIO_CHIP_CC2420
#endif

#ifndef ACCEL_CHIP
#define ACCEL_CHIP ACCEL_CHIP_ADXL345
#endif


//
// TMP sensor pins
// 
#define TMP102_PWR_PORT  5
#define TMP102_PWR_PIN   0

//
// User button pins
//
#define USER_BUTTON_PORT 2
#define USER_BUTTON_PIN  5


// serial pins, for sw serial
#define UART0_TX_PORT 3
#define UART0_TX_PIN  4
#define UART0_RX_PORT 3
#define UART0_RX_PIN  5

#define UART1_TX_PORT 3
#define UART1_TX_PIN  6
#define UART1_RX_PORT 3
#define UART1_RX_PIN  7


#endif
