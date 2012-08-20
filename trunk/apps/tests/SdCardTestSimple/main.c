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
#include "kernel/stdmansos.h"
#include "hil/udelay.h"
#include "hil/blink.h"

unsigned long cardSize = 0;
unsigned char status = 1;
unsigned int timeout = 0;
int i = 0;

unsigned char buffer[512];

void appMain(void)
{
    blink(3, 100);

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

    if (status) {
        blink(1000, 1000);
    } else {
        blink(1000, 100);
    }

    while ((mmcPing() != MMC_SUCCESS));      // Wait till card is inserted

//    blink(3, 100);

    blink(100, 100);

    // mdelay(2000);
    // panic();
    // return;

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
        ledToggle();
    }
}
