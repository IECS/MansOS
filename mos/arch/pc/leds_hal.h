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

// LEDs for the PC platform

#ifndef _LEDS_HAL_H_
#define _LEDS_HAL_H_

void initLedsPc(void); // for kernel only, do not call it in the application

// turn led on
void redLedOnPc(void);
void greenLedOnPc(void);
void blueLedOnPc(void);
void yellowLedOnPc(void);

// turn led off
void redLedOffPc(void);
void greenLedOffPc(void);
void blueLedOffPc(void);
void yellowLedOffPc(void);

// toggle led
void toggleRedLedPc(void);
void toggleGreenLedPc(void);
void toggleBlueLedPc(void);
void toggleYellowLedPc(void);

uint_t isRedLedOnPc(void); // 1, when led is on, 0 otherwise
uint_t isGreenLedOnPc(void);
uint_t isBlueLedOnPc(void);
uint_t isYellowLedOnPc(void);

void suppressLedOutput(bool yes);

#define LED_PORT_PC 255

#define LEDS_RED_PORT     LED_PORT_PC
#define LEDS_RED_PIN      RED_LED_NR
#define LEDS_GREEN_PORT   LED_PORT_PC
#define LEDS_GREEN_PIN    GREEN_LED_NR
#define LEDS_BLUE_PORT    LED_PORT_PC
#define LEDS_BLUE_PIN     BLUE_LED_NR
#define LEDS_YELLOW_PORT  LED_PORT_PC
#define LEDS_YELLOW_PIN   YELLOW_LED_NR

#endif

