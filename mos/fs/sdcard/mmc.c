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

// ***********************************************************
// File: mmc.c 
// Description: Library to access a MultiMediaCard 
//              functions: init, read, write ...
//  C. Speck / S. Schauer
//  Texas Instruments, Inc
//  June 2005
//
// Version 1.1
//   corrected comments about connection the MMC to the MSP430
//   increased timeout in mmcGetXXResponse
//
// ***********************************************************
// MMC Lib
// ***********************************************************


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


#include "mmc.h"
#include "sdcard_hal.h"
#include <string.h>

// XXX
#include <lib/dprint.h>
#include <hil/udelay.h>

// XXX: new spi code defines this!
#undef spiReadByte

// Initialize MMC card
char mmcInit(void)
{
    //raise CS and MOSI for 80 clock cycles
    //SendByte(0xff) 10 times with CS high
    //RAISE CS
    int i;

    // Port x Function           Dir       On/Off
    //         mmcCS         Out       0 - Active 1 - none Active
    //         Dout          Out       0 - off    1 - On -> init in SPI_Init
    //         Din           Inp       0 - off    1 - On -> init in SPI_Init
    //         Clk           Out       -                 -> init in SPI_Init
    //         mmcCD         In        0 - card inserted

    // Init Port for MMC (default high)
    MMC_PxOUT |= MMC_SIMO + MMC_UCLK;
    MMC_PxDIR |= MMC_SIMO + MMC_UCLK;


    // Chip Select
    MMC_CS_PxOUT |= MMC_CS;
    MMC_CS_PxDIR |= MMC_CS;

    // Card Detect
    MMC_CD_PxDIR &=  ~MMC_CD;
  
    // Init SPI Module
    halSPISetup();

    // Enable secondary function
#if SPI_SER_INTF != SER_INTF_BITBANG
    MMC_PxSEL |= MMC_SIMO + MMC_SOMI + MMC_UCLK;
#endif  
  
    //initialization sequence on PowerUp
    CS_HIGH();
    for (i=0;i<=9;i++)
        spiReadByte();

    return (mmcGoIdle());
}


// set MMC in Idle mode
char mmcGoIdle()
{
    char response=0x01;
    CS_LOW();

    //Send Command 0 to put MMC in SPI mode
    mmcSendCmd(MMC_GO_IDLE_STATE,0,0x95);
    //Now wait for READY RESPONSE
    if (mmcGetResponse()!=0x01)
        return MMC_INIT_ERROR;

    while (response==0x01){
        CS_HIGH();
        spiReadByte();
        CS_LOW();
        mmcSendCmd(MMC_SEND_OP_COND,0x00,0xff);
        response=mmcGetResponse();
    }
    CS_HIGH();
    spiReadByte();
    return (MMC_SUCCESS);
}

// mmc Get Responce
char mmcGetResponse(void)
{
    //Response comes 1-8bytes after command
    //the first bit will be a 0
    //followed by an error code
    //data will be 0xff until response
    int i=0;

    char response;

    while (i<=64) {
        response = spiReadByte();
        if (response == 0x00) break;
        if (response == 0x01) break;
        i++;
    }
    return response;
}

char mmcGetXXResponse(const char resp)
{
    //Response comes 1-8bytes after command
    //the first bit will be a 0
    //followed by an error code
    //data will be 0xff until response
    int i=0;

    char response;

    while (i<=1000) {
        response = spiReadByte();
        if (response==resp) return MMC_SUCCESS;
        i++;
    }
    return MMC_OTHER_ERROR;
}

// Check if MMC card is still busy
char mmcCheckBusy(void)
{
    //Response comes 1-8bytes after command
    //the first bit will be a 0
    //followed by an error code
    //data will be 0xff until response
    int i=0;

    char response;
    char rvalue;
    while (i<=64) {
        response=spiReadByte();
        response &= 0x1f;
        switch(response) {
        case 0x05: rvalue=MMC_SUCCESS; break;
        case 0x0b: return(MMC_CRC_ERROR);
        case 0x0d: return(MMC_WRITE_ERROR);
        default:
            rvalue = MMC_OTHER_ERROR;
            break;
        }
        if (rvalue==MMC_SUCCESS) break;
        i++;
    }
    i = 0;
    do {
        response = spiReadByte();
        i++;
    } while (response==0);
    return response;
}
// The card will respond with a standard response token followed by a data
// block suffixed with a 16 bit CRC.

// read a size Byte big block beginning at the address.
char mmcReadBlock(uint32_t address, uint16_t count, uint8_t *pBuffer)
{
    char rvalue = MMC_RESPONSE_ERROR;
    unsigned long alignedAddress = address & ~(SECTOR_SIZE - 1);
    uint16_t i;
    uint16_t diff = (uint16_t) (address - alignedAddress);
    bool restarted = false;

    PRINTF("MRB: address=%lu\n", address);
    PRINTF("MRB: alignedAddress=%lu\n", alignedAddress);
    PRINTF("MRB: diff=%u\n", diff);
//    PRINTF("MRB: count=%u\n", count);

    if (count + diff > SECTOR_SIZE) {
        return MMC_OTHER_ERROR;
    }


  restart:
    // Set the block length to read
//    if (mmcSetBlockLength(SECTOR_SIZE) == MMC_SUCCESS) {   // block length could be set
        // CS = LOW (on)
        CS_LOW();
        // send read command MMC_READ_SINGLE_BLOCK=CMD17
        mmcSendCmd (MMC_READ_SINGLE_BLOCK, alignedAddress, 0xFF);
        // Send 8 Clock pulses of delay, check if the MMC acknowledged the read block command
        // it will do this by sending an affirmative response
        // in the R1 format (0x00 is no errors)
        if (mmcGetResponse() == 0x00) {
            // now look for the data token to signify the start of
            // the data
            if (mmcGetXXResponse(MMC_START_DATA_BLOCK_TOKEN) == MMC_SUCCESS) {
                // clock the actual data transfer and receive the bytes;
                // spi_read automatically finds the Data Block
                //spiReadFrame(pBuffer, count, address - alignedAddress);
                for (i = 0; i < diff; i++) {
                    spiReadByte();
                }
                for (i = 0; i < count; i++) {
                    pBuffer[i] = spiReadByte();
                }
                for (i = SECTOR_SIZE - (count + diff); i > 0; i--) {
                    spiReadByte();
                }
                // get CRC bytes (not really needed by us, but required by MMC)
                spiReadByte();
                spiReadByte();
                rvalue = MMC_SUCCESS;
            }
            else {
                // the data token was never received
                rvalue = MMC_DATA_TOKEN_ERROR;      // 3
            }
        }
        else {
            // the MMC never acknowledge the read command
            rvalue = MMC_RESPONSE_ERROR;          // 2
        }
//    }
//    else {
//        rvalue = MMC_BLOCK_SET_ERROR;           // 1
//    }
    CS_HIGH();
    // Send 8 Clock pulses of delay.
    spiSendByte(DUMMY_CHAR);
    if (rvalue && !restarted) {
        mdelay(100);
        restarted = true;
        PRINTF("mrb: restart\n");
        goto restart;
    }
    return rvalue;
}

char mmcWritePartialBlock(uint32_t address, uint16_t count, const uint8_t *pBuffer)
{
    uint8_t tmpBuf[SECTOR_SIZE];
    char rvalue = MMC_RESPONSE_ERROR;
    uint32_t alignedAddress = address & ~(SECTOR_SIZE - 1);
    if ((alignedAddress != address) || (count != SECTOR_SIZE)) {
        uint16_t diff = (uint16_t) (address - alignedAddress);
        if (count + diff > SECTOR_SIZE) return MMC_OTHER_ERROR;
        rvalue = mmcReadBlock(alignedAddress, SECTOR_SIZE, tmpBuf);
        if (rvalue != MMC_SUCCESS) return rvalue;
        memcpy(tmpBuf + diff, pBuffer, count);
        pBuffer = tmpBuf;
    }

    // Set the block length to write
//    if (mmcSetBlockLength(SECTOR_SIZE) == MMC_SUCCESS) {  // block length could be set
        CS_LOW();
        // send write command
        mmcSendCmd (MMC_WRITE_BLOCK, alignedAddress, 0xFF);

        // check if the MMC acknowledged the write block command
        // it will do this by sending an affirmative response
        // in the R1 format (0x00 is no errors)
        if (mmcGetXXResponse(MMC_R1_RESPONSE) == MMC_SUCCESS) {
            spiSendByte(DUMMY_CHAR);
            // send the data token to signify the start of the data
            spiSendByte(0xfe);

            // clock the actual data transfer and transmitt the bytes
            spiSendFrame(pBuffer, SECTOR_SIZE);

            // put CRC bytes (not really needed by us, but required by MMC)
            spiSendByte(DUMMY_CHAR);
            spiSendByte(DUMMY_CHAR);
            // read the data response xxx0<status>1 : status 010: Data accected, status 101: Data
            //   rejected due to a crc error, status 110: Data rejected due to a Write error.
            mmcCheckBusy();
            rvalue = MMC_SUCCESS;
        }
        else {
            // the MMC never acknowledge the write command
            rvalue = MMC_RESPONSE_ERROR;   // 2
        }
//    }
//    else {
//        rvalue = MMC_BLOCK_SET_ERROR;   // 1
//    }
    // give the MMC the required clocks to finish up what ever it needs to do
    //  for (i = 0; i < 9; ++i)
    //    spiSendByte(0xff);

    CS_HIGH();
    // Send 8 Clock pulses of delay.
    spiSendByte(DUMMY_CHAR);
    return rvalue;
}

char mmcWriteBlock(uint32_t address, const uint8_t *pBuffer)
{
    char rvalue = MMC_RESPONSE_ERROR;

    if (address & (SECTOR_SIZE - 1)) {
        return MMC_OTHER_ERROR;
    }

    // Set the block length to write
//    if (mmcSetBlockLength(SECTOR_SIZE) == MMC_SUCCESS) {  // block length could be set
        // CS = LOW (on)
        CS_LOW();
        // send write command
        mmcSendCmd (MMC_WRITE_BLOCK,address, 0xFF);

        // check if the MMC acknowledged the write block command
        // it will do this by sending an affirmative response
        // in the R1 format (0x00 is no errors)
        if (mmcGetXXResponse(MMC_R1_RESPONSE) == MMC_SUCCESS) {
            spiSendByte(DUMMY_CHAR);
            // send the data token to signify the start of the data
            spiSendByte(0xfe);
            // clock the actual data transfer and transmitt the bytes
            spiSendFrame(pBuffer, SECTOR_SIZE);
            // put CRC bytes (not really needed by us, but required by MMC)
            spiSendByte(DUMMY_CHAR);
            spiSendByte(DUMMY_CHAR);
            // read the data response xxx0<status>1 : status 010: Data accected, status 101: Data
            //   rejected due to a crc error, status 110: Data rejected due to a Write error.
            mmcCheckBusy();
            rvalue = MMC_SUCCESS;
        }
        else {
            // the MMC never acknowledge the write command
            rvalue = MMC_RESPONSE_ERROR;   // 2
        }
//    }
//    else {
//        rvalue = MMC_BLOCK_SET_ERROR;   // 1
//    }
    // give the MMC the required clocks to finish up what ever it needs to do
    //  for (i = 0; i < 9; ++i)
    //    spiSendByte(0xff);

    CS_HIGH();
    // Send 8 Clock pulses of delay.
    spiSendByte(DUMMY_CHAR);
    return rvalue;
}

// send command to MMC
void mmcSendCmd(const char cmd, uint32_t data, const char crc)
{
    unsigned char frame[6];
    char temp;
    int i;
    frame[0]=(cmd|0x40);
    for (i=3;i>=0;i--){
        temp = (char)(data>>(8*i));
        frame[4-i] = (temp);
    }
    frame[5] = crc;
    spiSendFrame(frame,6);
}


//--------------- set blocklength 2^n ------------------------------------------
char mmcSetBlockLength(const uint32_t blocklength)
{
    // CS = LOW (on)
    CS_LOW ();
    // Set the block length to read
    mmcSendCmd(MMC_SET_BLOCKLEN, blocklength, 0xFF);

    // get response from MMC - make sure that its 0x00 (R1 ok response format)
    if (mmcGetResponse() != 0x00) {
        mmcInit();
        mmcSendCmd(MMC_SET_BLOCKLEN, blocklength, 0xFF);
        mmcGetResponse();
    }

    CS_HIGH ();

    // Send 8 Clock pulses of delay.
    spiSendByte(DUMMY_CHAR);

    return MMC_SUCCESS;
}


#if 0 // commented out to save code size - AE

// Reading the contents of the CSD and CID registers in SPI mode is a simple
// read-block transaction.
char mmcReadRegister (const char cmd_register, const unsigned char length, unsigned char *pBuffer)
{
    unsigned char uc = 0;
    char rvalue = MMC_TIMEOUT_ERROR;

    if (mmcSetBlockLength (length) == MMC_SUCCESS) {
        CS_LOW ();
        // CRC not used: 0xff as last byte
        mmcSendCmd(cmd_register, 0x000000, 0xff);

        // wait for response
        // in the R1 format (0x00 is no errors)
        if (mmcGetResponse() == 0x00) {
            if (mmcGetXXResponse(0xfe) == MMC_SUCCESS) {
                for (uc = 0; uc < length; uc++) {
                    pBuffer[uc] = spiSendByte(DUMMY_CHAR);
                }
            }
            // get CRC bytes (not really needed by us, but required by MMC)
            spiSendByte(DUMMY_CHAR);
            spiSendByte(DUMMY_CHAR);
            rvalue = MMC_SUCCESS;
        }
        else {
            rvalue = MMC_RESPONSE_ERROR;
        }
        // CS = HIGH (off)
        CS_HIGH ();

        // Send 8 Clock pulses of delay.
        spiSendByte(DUMMY_CHAR);
    }
    CS_HIGH ();
    return rvalue;
}

unsigned long mmcReadCardSize(void)
{
    // Read contents of Card Specific Data (CSD)

    unsigned long MMC_CardSize;
    unsigned short i,      // index
            j,      // index
            b,      // temporary variable
            response,   // MMC response to command
            mmc_C_SIZE = 0;

    unsigned char mmc_READ_BL_LEN = 0,  // Read block length
            mmc_C_SIZE_MULT = 0;

    CS_LOW ();

    spiSendByte(MMC_READ_CSD);   // CMD 9
    for (i=4; i>0; i--) {    // Send four dummy bytes
        spiSendByte(0);
    }
    spiSendByte(DUMMY_CHAR);   // Send CRC byte

    response = mmcGetResponse();

    // data transmission always starts with 0xFE
    b = spiSendByte(DUMMY_CHAR);

    if (!response ) {
        while (b != 0xFE) b = spiSendByte(DUMMY_CHAR);
        // bits 127:87
        for (j=5; j>0; j--) {     // Host must keep the clock running for at
            b = spiSendByte(DUMMY_CHAR);
        }

        // 4 bits of READ_BL_LEN
        // bits 84:80
        b = spiSendByte(DUMMY_CHAR);  // lower 4 bits of CCC and
        mmc_READ_BL_LEN = b & 0x0F;
        b = spiSendByte(DUMMY_CHAR);
        // bits 73:62  C_Size
        // xxCC CCCC CCCC CC
        mmc_C_SIZE = (b & 0x03) << 10;
        b = spiSendByte(DUMMY_CHAR);
        mmc_C_SIZE += b << 2;
        b = spiSendByte(DUMMY_CHAR);
        mmc_C_SIZE += b >> 6;
        // bits 55:53
        b = spiSendByte(DUMMY_CHAR);
        // bits 49:47
        mmc_C_SIZE_MULT = (b & 0x03) << 1;
        b = spiSendByte(DUMMY_CHAR);
        mmc_C_SIZE_MULT += b >> 7;
        // bits 41:37
        b = spiSendByte(DUMMY_CHAR);
        b = spiSendByte(DUMMY_CHAR);
        b = spiSendByte(DUMMY_CHAR);
        b = spiSendByte(DUMMY_CHAR);
        b = spiSendByte(DUMMY_CHAR);
    }

    for (j=4; j>0; j--) {             // Host must keep the clock running for at
        b = spiSendByte(DUMMY_CHAR);  // least Ncr (max = 4 bytes) cycles after
    }                                 // the card response is received
    b = spiSendByte(DUMMY_CHAR);
    CS_LOW ();

    MMC_CardSize = (mmc_C_SIZE + 1);
    // power function with base 2 is better with a loop
    // i = (pow(2,mmc_C_SIZE_MULT+2)+0.5);
    for (i = 2,j=mmc_C_SIZE_MULT+2; j>1; j--) {
        i <<= 1;
    }
    MMC_CardSize *= i;
    // power function with base 2 is better with a loop
    //i = (pow(2,mmc_READ_BL_LEN)+0.5);
    for (i = 2,j=mmc_READ_BL_LEN; j>1; j--) {
        i <<= 1;
    }
    MMC_CardSize *= i;
    return MMC_CardSize;
}
#endif


char mmcPing(void)
{
    if (!(MMC_CD_PxIN & MMC_CD))
        return (MMC_SUCCESS);
    else
        return (MMC_INIT_ERROR);
}
