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

//*******************************************************************************
//  MSP-MMC Demo - Demo to show how to use the MMC library
//
//  Description; This example shows the correct setup and usage of the MMC
//  Library
//
//
//  S. Schauer
//  Texas Instruments, Inc
//  March 2008
//  V1.1: Updated for IAR Embedded Workbench Version: 5.10
//  V1.2: fixed fill of buffer (buffer size overrun)
//******************************************************************************

/* ***********************************************************
* THIS PROGRAM IS PROVIDED "AS IS". TI MAKES NO WARRANTIES OR
* REPRESENTATIONS, EITHER EXPRESS, IMPLIED OR STATUTORY, 
* INCLUDING ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS 
* FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR 
* COMPLETENESS OF RESPONSES, RESULTS AND LACK OF NEGLIGENCE. 
* TI DISCLAIMS ANY WARRANTY OF TITLE, QUIET ENJOYMENT, QUIET 
* POSSESSION, AND NON-INFRINGEMENT OF ANY THIRD PARTY 
* INTELLECTUAL PROPERTY RIGHTS WITH REGARD TO THE PROGRAM OR 
* YOUR USE OF THE PROGRAM.
*
* IN NO EVENT SHALL TI BE LIABLE FOR ANY SPECIAL, INCIDENTAL, 
* CONSEQUENTIAL OR INDIRECT DAMAGES, HOWEVER CAUSED, ON ANY 
* THEORY OF LIABILITY AND WHETHER OR NOT TI HAS BEEN ADVISED 
* OF THE POSSIBILITY OF SUCH DAMAGES, ARISING IN ANY WAY OUT 
* OF THIS AGREEMENT, THE PROGRAM, OR YOUR USE OF THE PROGRAM. 
* EXCLUDED DAMAGES INCLUDE, BUT ARE NOT LIMITED TO, COST OF 
* REMOVAL OR REINSTALLATION, COMPUTER TIME, LABOR COSTS, LOSS 
* OF GOODWILL, LOSS OF PROFITS, LOSS OF SAVINGS, OR LOSS OF 
* USE OR INTERRUPTION OF BUSINESS. IN NO EVENT WILL TI'S 
* AGGREGATE LIABILITY UNDER THIS AGREEMENT OR ARISING OUT OF 
* YOUR USE OF THE PROGRAM EXCEED FIVE HUNDRED DOLLARS 
* (U.S.$500).
*
* Unless otherwise stated, the Program written and copyrighted 
* by Texas Instruments is distributed as "freeware".  You may, 
* only under TI's copyright in the Program, use and modify the 
* Program without any charge or restriction.  You may 
* distribute to third parties, provided that you transfer a 
* copy of this license to the third party and the third party 
* agrees to these terms by its first use of the Program. You 
* must reproduce the copyright notice and any other legend of 
* ownership on each copy or partial copy, of the Program.
*
* You acknowledge and agree that the Program contains 
* copyrighted material, trade secrets and other TI proprietary 
* information and is protected by copyright laws, 
* international copyright treaties, and trade secret laws, as 
* well as other intellectual property laws.  To protect TI's 
* rights in the Program, you agree not to decompile, reverse 
* engineer, disassemble or otherwise translate any object code 
* versions of the Program to a human-readable form.  You agree 
* that in no event will you alter, remove or destroy any 
* copyright notice included in the Program.  TI reserves all 
* rights not specifically granted under this license. Except 
* as specifically provided herein, nothing in this agreement 
* shall be construed as conferring by implication, estoppel, 
* or otherwise, upon you, any license or other right under any 
* TI patents, copyrights or trade secrets.
*
* You may not use the Program in non-TI devices.
* ********************************************************* */


#include <msp430x16x.h>
#include "mmc.h"
#include <stdio.h>
#include "mansos.h"
#include "udelay.h"
#include "sleep.h"

#define CPU_FREQUENCY 4096
#define CPU_MULTIPLIER 4

// DCO calibration approach is borrowed from TOS1.x
enum {
    ACLK_CALIB_PERIOD = 8,
    ACLK_KHZ = 32,
    TARGET_DCO_KHZ = CPU_FREQUENCY, // prescribe the cpu clock rate in kHz
    TARGET_DCO_DELTA = (TARGET_DCO_KHZ / ACLK_KHZ) * ACLK_CALIB_PERIOD,
};

static uint16_t testCalibBusywaitDelta(uint16_t calib)
{
    int8_t aclk_count = 2;
    uint16_t dco_prev = 0;
    uint16_t dco_curr = 0;

    // Set DCO calibration
    BCSCTL1 = (BCSCTL1 & ~0x07) | ((calib >> 8) & 0x07);
    DCOCTL = calib & 0xff;

    while (aclk_count--) {
        TBCCR0 = TBR + ACLK_CALIB_PERIOD; // set next interrupt
        TBCCTL0 &= ~CCIFG; // clear pending interrupt
        while ((TBCCTL0 & CCIFG) == 0); // busy wait
        dco_prev = dco_curr;
        dco_curr = TAR;
    }

    return dco_curr - dco_prev;
}

void msp430CalibrateDCO(void)
{
    uint16_t calib;
    uint16_t step;

    // Timer source is SMCLK, continuous mode
    TACTL = TASSEL1 | MC1;
    TBCTL = TBSSEL0 | MC1;
    BCSCTL1 = XT2OFF | RSEL2;
    BCSCTL2 = 0;
    TBCCTL0 = CM0;

    // Binary search for RSEL, DCO, DCOMOD.
    // It's okay that RSEL isn't monotonic.
    for (calib = 0, step = 0x800; step != 0; step >>= 1) {
        // if the step is not past the target, commit it
        if (testCalibBusywaitDelta(calib | step) <= TARGET_DCO_DELTA) {
            calib |= step;
        }
    }

    // if DCOx is 7 (0xe0 in calib),
    // then the 5-bit MODx is not useable, set it to 0
    if ((calib & 0xe0) == 0xe0) {
        calib &= ~0x1f;
    }

    // Set DCO calibration
    BCSCTL1 = (BCSCTL1 & ~0x07) | ((calib >> 8) & 0x07);
    DCOCTL = calib & 0xff;
}

void initClocks(void)
{
    // Reset timers and clear interrupt vectors
    TACTL = TACLR;
//    TAIV = 0;
    TBCTL = TBCLR;
//    TBIV = 0;

    // IE1.OFIE = 0; no interrupt for oscillator fault
    IE1 &= ~OFIE;

    msp430CalibrateDCO();
}

// -- LEDs code

#define LEDS_PORT 5

#define LEDS_RED_PIN 4
#define LEDS_GREEN_PIN 5
#define LEDS_BLUE_PIN 6

#define LEDS_ALL_PIN_MASK \
    ((1 << LEDS_RED_PIN)                        \
            | (1 << LEDS_GREEN_PIN)             \
            | (1 << LEDS_BLUE_PIN))             \

#define PIN_AS_OUTPUT(portnum, pinbit)          \
    P##portnum##DIR |= pinbit
#define PIN_SET(portnum, pinbit)                \
    P##portnum##OUT |= pinbit      
#define PIN_CLEAR(portnum, pinbit)              \
    P##portnum##OUT &= ~(pinbit)
#define PIN_TOGGLE(portnum, pinbit)             \
    P##portnum##OUT ^= pinbit

#define pinSet(po, pi) PIN_SET(po, 1 << pi)
#define pinSetMask(po, pm) PIN_SET(po, pm)
#define pinClear(po, pi) PIN_CLEAR(po, 1 << pi)
#define pinClearMask(po, pm) PIN_CLEAR(po, pm)
#define pinToggle(po, pi) PIN_TOGGLE(po, 1 << pi)
#define pinToggleMask(po, pm) PIN_TOGGLE(po, pm)
#define pinAsOutput(po, pi) PIN_AS_OUTPUT(po, 1 << pi)
#define pinAsOutputMask(po, pm) PIN_AS_OUTPUT(po, pm)

#define toggleRedLed()   pinToggle(LEDS_PORT, LEDS_RED_PIN)
#define toggleGreenLed() pinToggle(LEDS_PORT, LEDS_GREEN_PIN)
#define toggleBlueLed()  pinToggle(LEDS_PORT, LEDS_BLUE_PIN)
#define toggleAllLeds()  pinToggleMask(LEDS_PORT, LEDS_ALL_PIN_MASK)

#define redLedOn()       pinClear(LEDS_PORT, LEDS_RED_PIN)
#define greenLedOn()     pinClear(LEDS_PORT, LEDS_GREEN_PIN)
#define blueLedOn()      pinClear(LEDS_PORT, LEDS_BLUE_PIN)
#define allLedsOn()      pinClearMask(LEDS_PORT, LEDS_ALL_PIN_MASK)

#define redLedOff()      pinSet(LEDS_PORT, LEDS_RED_PIN)
#define greenLedOff()    pinSet(LEDS_PORT, LEDS_GREEN_PIN)
#define blueLedOff()     pinSet(LEDS_PORT, LEDS_BLUE_PIN)
#define allLedsOff()     pinSetMask(LEDS_PORT, LEDS_ALL_PIN_MASK)

void initLEDs()
{
    pinAsOutputMask(LEDS_PORT, LEDS_ALL_PIN_MASK);
    allLedsOff();
}

void blink(uint16_t count)
{
    uint16_t i;
    for (i = 0; i < count; i++) {
        redLedOn();
        mdelay(100);
        redLedOff();
        mdelay(100);
    }
}

void panic()
{
    blink(100);
}


unsigned long cardSize = 0;
unsigned char status = 1;
unsigned int timeout = 0;
int i = 0;

unsigned char buffer[512];

int main(void)
{
    WDTCTL = WDTPW + WDTHOLD;
    initClocks();

    //Initialisation of the MMC/SD-card
    while (status != 0) {                      // if return in not NULL an error did occur and the
        // MMC/SD-card will be initialized again 
        status = mmcInit();
        timeout++;
        if (timeout == 150) {                     // Try 50 times till error
            //printf ("No MMC/SD-card found!! %x\n", status);
            break;
        }
    }

    while ((mmcPing() != MMC_SUCCESS));      // Wait till card is inserted

    initLEDs();

    blink(3);

    // Read the Card Size from the CSD Register
    cardSize = mmcReadCardSize();

    // Clear Sectors on MMC
    for (i = 0; i < 512; i++) buffer[i] = 0;
    if (mmcWriteSector(0, buffer) != MMC_SUCCESS) {                // write a 512 Byte big block beginning at the (aligned) adress
        panic();
    }

    for (i = 0; i < 512; i++) buffer[i] = 0;
    if (mmcWriteSector(1, buffer) != MMC_SUCCESS) {                // write a 512 Byte big block beginning at the (aligned) adress
        panic();
    }

    buffer[1] = 0xff;
    if (mmcReadSector(0, buffer) != MMC_SUCCESS) {
        panic();
    }
    for (i = 0; i < 512; i++) {
        if (buffer[i] != 0) panic();
    }

    buffer[13] = 0xff;
    if (mmcReadSector(1, buffer) != MMC_SUCCESS) {
        panic();
    }
    for (i = 0; i < 512; i++) {
        if (buffer[i] != 0) panic();
    }

    // Write Data to MMC  
    for (i = 0; i < 512; i++) buffer[i] = i;
    mmcWriteSector(0, buffer);                // write a 512 Byte big block beginning at the (aligned) adress

    for (i = 0; i < 512; i++) buffer[i] = i+64;
    mmcWriteSector(1, buffer);                // write a 512 Byte big block beginning at the (aligned) adress

    mmcReadSector(0, buffer);                 // read a size Byte big block beginning at the address.
    for (i = 0; i < 512; i++) {
        if(buffer[i] != (unsigned char)i) panic();
    }

    mmcReadSector(1, buffer);                 // read a size Byte big block beginning at the address.
    for (i = 0; i < 512; i++) {
        if(buffer[i] != (unsigned char)(i+64)) panic();
    }

    for (i = 0; i < 512; i++)
        mmcReadSector(i, buffer);               // read a size Byte big block beginning at the address.


    mmcGoIdle();                              // set MMC in Idle mode

    for (;;) {
        mdelay(1000);
        toggleRedLed();
    }
}
