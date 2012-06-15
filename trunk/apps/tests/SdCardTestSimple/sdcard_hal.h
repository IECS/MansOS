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

#ifndef MANSOS_SDCARD_HAL_H
#define MANSOS_SDCARD_HAL_H

#include <msp430x16x.h>

//----------------------------------------------------------------------------
//  This include file contains definitions specific to the hardware board.
//----------------------------------------------------------------------------
// ********************************************************
//
//   
//
//            MSP430F169                  MMC Card 
//         -----------------          -----------------
//     /|\|              XIN|-   /|\ |                 |
//      | |                 |     |  |                 |
//      --|RST          XOUT|-    |--|Pin4/Vcc         |
//        |                 |        |                 |
//        |                 |        |                 |
//        |            P5.6 |<-------|Pin6/CD          |
//        |            P5.0 |------->|Pin1/CS          |
//        |                 |        |                 |
//        |      P5.2/SOMI1 |------->|Pin7/DOUT        |
//        |      P5.1/SIMO1 |<-------|Pin2/DIN         |
//        |      P5.3/UCLK1 |------->|Pin5/CLK         |
//        |                 |        |                 |
//        |                 |     |--|Pin3/GND         |
//                                |
//                                =
//
//  Pin configuration at MSP430F169:
//  --------------------------------
//  MSP430F169      MSP Pin        MMC             MMC Pin
//  -------------------------------------------------------------
//  P5.0              48           ChipSelect       1
//  P5.2 / SOMI       46           DataOut          7
//                                 GND              3 (0 V)
//                                 VDD              4 (3.3 V)
//  P5.3 / UCLK1      47           Clock            5
//  P5.6              44           CardDetect       6
//  P5.1 / SIMO       45           DataIn           2
//  -------------------------------------------------------------


// SPI port definitions
#define SPI_PxSEL         P5SEL
#define SPI_PxDIR         P5DIR
#define SPI_PxIN          P5IN
#define SPI_PxOUT         P5OUT
#define SPI_SIMO          0x02
#define SPI_SOMI          0x04
#define SPI_UCLK          0x08

//----------------------------------------------------------------------------
// SPI/UART port selections.  Select which port will be used for the interface 
//----------------------------------------------------------------------------
#define SPI_SER_INTF      SER_INTF_USART1  // Interface to MMC


// SPI port definitions
#define MMC_PxSEL         SPI_PxSEL
#define MMC_PxDIR         SPI_PxDIR
#define MMC_PxIN          SPI_PxIN
#define MMC_PxOUT         SPI_PxOUT      
#define MMC_SIMO          SPI_SIMO
#define MMC_SOMI          SPI_SOMI
#define MMC_UCLK          SPI_UCLK

// Chip Select
#define MMC_CS_PxOUT      P5OUT
#define MMC_CS_PxDIR      P5DIR
#define MMC_CS            0x01

// Card Detect
#define MMC_CD_PxIN       P5IN
#define MMC_CD_PxDIR      P5DIR
#define MMC_CD            0x40

// Card Select
#define CS_LOW()    do {                            \
        MMC_CS_PxOUT &= ~MMC_CS;                    \
    } while(0);

// Card Deselect
#define CS_HIGH()   do {                            \
        while (!halSPITXDONE);                      \
        MMC_CS_PxOUT |= MMC_CS;                     \
    } while(0);

#define DUMMY_CHAR 0xFF


// --------------------------- from FATlib


/* Memory card sector size */
#define SECTOR_SIZE 512


/**
 * MMC/SD card SPI mode commands
 **/
#define CMD0 0x40    // software reset
#define CMD1 0x41    // brings card out of idle state
#define CMD2 0x42    // not used in SPI mode
#define CMD3 0x43    // not used in SPI mode
#define CMD4 0x44    // not used in SPI mode
#define CMD5 0x45    // Reserved
#define CMD6 0x46    // Reserved
#define CMD7 0x47    // not used in SPI mode
#define CMD8 0x48    // Reserved
#define CMD9 0x49    // ask card to send card speficic data (CSD)
#define CMD10 0x4A    // ask card to send card identification (CID)
#define CMD11 0x4B    // not used in SPI mode
#define CMD12 0x4C    // stop transmission on multiple block read
#define CMD13 0x4D    // ask the card to send it's status register
#define CMD14 0x4E    // Reserved
#define CMD15 0x4F    // not used in SPI mode
#define CMD16 0x50    // sets the block length used by the memory card
#define CMD17 0x51    // read single block
#define CMD18 0x52    // read multiple block
#define CMD19 0x53    // Reserved
#define CMD20 0x54    // not used in SPI mode
#define CMD21 0x55    // Reserved
#define CMD22 0x56    // Reserved
#define CMD23 0x57    // Reserved
#define CMD24 0x58    // writes a single block
#define CMD25 0x59    // writes multiple blocks
#define CMD26 0x5A    // not used in SPI mode
#define CMD27 0x5B    // change the bits in CSD
#define CMD28 0x5C    // sets the write protection bit
#define CMD29 0x5D    // clears the write protection bit
#define CMD30 0x5E    // checks the write protection bit
#define CMD31 0x5F    // Reserved
#define CMD32 0x60    // Sets the address of the first sector of the erase group
#define CMD33 0x61    // Sets the address of the last sector of the erase group
#define CMD34 0x62    // removes a sector from the selected group
#define CMD35 0x63    // Sets the address of the first group
#define CMD36 0x64    // Sets the address of the last erase group
#define CMD37 0x65    // removes a group from the selected section
#define CMD38 0x66    // erase all selected groups
#define CMD39 0x67    // not used in SPI mode
#define CMD40 0x68    // not used in SPI mode
#define CMD41 0x69    // Reserved
#define CMD42 0x6A    // locks a block
// CMD43 ... CMD57 are Reserved
#define CMD58 0x7A    // reads the OCR register
#define CMD59 0x7B    // turns CRC off
// CMD60 ... CMD63 are not used in SPI mode


//----------------------------------------------------------------------------
//  These constants are used to identify the chosen SPI and UART
//  interfaces.
//----------------------------------------------------------------------------
#define SER_INTF_NULL    0
#define SER_INTF_USART0  1
#define SER_INTF_USART1  2
#define SER_INTF_USCIA0  3
#define SER_INTF_USCIA1  4
#define SER_INTF_USCIB0  5
#define SER_INTF_USCIB1  6
#define SER_INTF_USI     7
#define SER_INTF_BITBANG 8


#if SPI_SER_INTF == SER_INTF_USART0
 #define halSPIRXBUF  U0RXBUF
 #define halSPI_SEND(x) U0TXBUF=x
 #define halSPITXREADY  (IFG1&UTXIFG0)         /* Wait for TX to be ready */
 #define halSPITXDONE  (U0TCTL&TXEPT)          /* Wait for TX to finish */
 #define halSPIRXREADY (IFG1&URXIFG0)          /* Wait for TX to be ready */
 #define halSPIRXFG_CLR IFG1 &= ~URXIFG0
 #define halSPI_PxIN  SPI_USART0_PxIN
 #define halSPI_SOMI  SPI_USART0_SOMI

 #elif SPI_SER_INTF == SER_INTF_USART1
 #define halSPIRXBUF  U1RXBUF
 #define halSPI_SEND(x) U1TXBUF=x
 #define halSPITXREADY  (IFG2&UTXIFG1)         /* Wait for TX to be ready */
 #define halSPITXDONE  (U1TCTL&TXEPT)          /* Wait for TX to finish */
 #define halSPIRXREADY (IFG2&URXIFG1)          /* Wait for TX to be ready */
 #define halSPIRXFG_CLR IFG2 &= ~URXIFG1
 #define halSPI_PxIN  SPI_USART1_PxIN
 #define halSPI_SOMI  SPI_USART1_SOMI

 #elif SPI_SER_INTF == SER_INTF_USCIA0
 #define halSPIRXBUF  U0RXBUF
 #define halSPI_SEND(x) U0TXBUF=x
 #define halSPITXREADY  (IFG1&UTXIFG0)         /* Wait for TX to be ready */
 #define halSPITXDONE  (UCA0STAT&UCBUSY)       /* Wait for TX to finish */
 #define halSPIRXREADY (IFG1&URXIFG0)          /* Wait for TX to be ready */
 #define halSPIRXFG_CLR IFG1 &= ~URXIFG0
 #define halSPI_PxIN  SPI_USART0_PxIN
 #define halSPI_SOMI  SPI_USART0_SOMI

 #elif SPI_SER_INTF == SER_INTF_USCIA1
 #define halSPIRXBUF  U0RXBUF
 #define halSPI_SEND(x) U0TXBUF=x
 #define halSPITXREADY  (IFG1&UTXIFG0)         /* Wait for TX to be ready */
 #define halSPITXDONE  (UCA1STAT&UCBUSY)       /* Wait for TX to finish */
 #define halSPIRXREADY (IFG1&URXIFG0)          /* Wait for TX to be ready */
 #define halSPIRXFG_CLR IFG1 &= ~URXIFG0
 #define halSPI_PxIN  SPI_USART0_PxIN
 #define halSPI_SOMI  SPI_USART0_SOMI

 #elif SPI_SER_INTF == SER_INTF_USCIB0
 #define halSPIRXBUF  UCB0RXBUF
 #define halSPI_SEND(x) UCB0TXBUF=x
 #define halSPITXREADY  (UC0IFG&UCB0TXIFG)     /* Wait for TX to be ready */
 #define halSPITXDONE  (UCB0STAT&UCBUSY)       /* Wait for TX to finish */
 #define halSPIRXREADY (UC0IFG&UCB0RXIFG)      /* Wait for TX to be ready */
 #define halSPIRXFG_CLR UC0IFG &= ~UCB0RXIFG
 #define halSPI_PxIN  SPI_USART0_PxIN
 #define halSPI_SOMI  SPI_USART0_SOMI

 #elif SPI_SER_INTF == SER_INTF_USCIB1
 #define halSPIRXBUF  U0RXBUF
 #define halSPI_SEND(x) U0TXBUF=x
 #define halSPITXREADY  (IFG1&UTXIFG0)         /* Wait for TX to be ready */
 #define halSPIRXREADY (IFG1&URXIFG0)          /* Wait for TX to be ready */
 #define halSPIRXFG_CLR IFG1 &= ~URXIFG0
 #define halSPI_PxIN  SPI_USART0_PxIN
 #define halSPI_SOMI  SPI_USART0_SOMI

 #elif SPI_SER_INTF == SER_INTF_USI
 #define halSPIRXBUF  USISRL
 #define halSPI_SEND(x) USISRL = x; USICNT = 8
 #define halSPITXREADY (USICTL1&USIIFG)
 #define halSPITXDONE  (1)
 #define halSPIRXREADY  1
 #define halSPIRXFG_CLR
 #define halSPI_PxIN  SPI_BITBANG_PxIN
 #define halSPI_SOMI  SPI_BITBANG_SOMI

 #elif SPI_SER_INTF == SER_INTF_BITBANG
 #define halSPIRXBUF  spi_bitbang_in_data
 #define halSPI_SEND(x) spi_bitbang_inout(x)
 #define halSPITXREADY  1
 #define halSPITXDONE   1
 #define halSPIRXREADY  1
 #define halSPIRXFG_CLR
 #define halSPI_PxIN  SPI_BITBANG_PxIN
 #define halSPI_SOMI  SPI_BITBANG_SOMI
#endif


// Function Prototypes
void halSPISetup(void);
unsigned char spiSendByte(const unsigned char data);
void spiReadFrame(unsigned char* pBuffer, unsigned int size);
void spiSendFrame(unsigned char* pBuffer, unsigned int size);

#define spiReadByte() spiSendByte(0xFF)


#endif
