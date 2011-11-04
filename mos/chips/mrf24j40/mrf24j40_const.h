/*******************************************************************
 *
 *                  MRF24J40 Radio Defines
 *
 *********************************************************************
 * FileName:        MRF24J40.h
 * Dependencies:    None
 * Processor:       PIC18/PIC24
 * Complier:        MCC18 v1.00.50 or higher, C30 3.10 or higher
 *                  HITECH PICC-18 V8.10PL1 or higher
 * Company:         Microchip Technology, Inc.
 *
Software License Agreement

The software supplied herewith by Microchip Technology Incorporated
(the Company) for its PICmicro® Microcontroller is intended and
supplied to you, the Companys customer, for use solely and
exclusively on Microchip PICmicro Microcontroller products. The
software is owned by the Company and/or its supplier, and is
protected under applicable copyright laws. All rights are reserved.
Any use in violation of the foregoing restrictions may subject the
user to criminal sanctions under applicable laws, as well as to
civil liability for the breach of the terms and conditions of this
license.

THIS SOFTWARE IS PROVIDED IN AN AS IS CONDITION. NO WARRANTIES,
WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
**********************************************************************/

#ifndef MANSOS_MRF24J40_CONST_H
#define MANSOS_MRF24J40_CONST_H

typedef union _MRF24J40_IFS
{
    uint8_t value;
    struct _MRF24J40_IFS_bits
    {
        uint8_t RF_TXIF :1;
        uint8_t :2;
        uint8_t RF_RXIF :1;
        uint8_t RF_SECIF :1;
        uint8_t :1;
        uint8_t RF_WAKEIF :1;
        uint8_t :1;
    } bits;
} MRF24J40_IFREG;

#define MRF24J40_RX_IF   0x08
#define MRF24J40_TX_IF   0x01
#define MRF24J40_SEC_IF  0x10
#define MRF24J40_WAKE_IF 0x40


extern uint8_t currentRxByte;
extern uint8_t currentTxByte;
extern uint8_t phyIsIdle;


//long address registers
#define RFCON0 (0x200) // RFCTRL0
#define RFCON1 (0x201) // RFCTRL1
#define RFCON2 (0x202) // RFCTRL2
#define RFCON3 (0x203) // RFCTRL3
#define RFCON5 (0x205) // RFCTRL5
#define RFCON6 (0x206) // RFCTRL6
#define RFCON7 (0x207) // RFCTRL7
#define RFCON8 (0x208) // RFCTRL8
#define SLPCAL0 (0x209)
#define SLPCAL1 (0x20a)
#define SLPCAL2 (0x20b)
#define RFSTATE (0x20f)
#define RSSI (0x210)
#define SLPCON0 (0x211) // CLKINTCR
#define SLPCON1 (0x220) // CLKCTRL
#define WAKETIMEL (0x222)
#define WAKETIMEH (0x223)
#define REMCNTL (0x224)
#define REMCNTH (0x225)
#define MAINCNT0 (0x226)
#define MAINCNT1 (0x227)
#define MAINCNT2 (0x228)
#define MAINCNT3 (0x229)
#define TESTMODE (0x22f)

//definitions for FIFOs in the memory
#define TX_NORMAL_FIFO  (0x000)
#define TX_BEACON_FIFO  (0x080)
#define TX_GTS1_FIFO    (0x100)
#define TX_GTS2_FIFO    (0x180)
#define RX_FIFO         (0x300)
#define SECURITY_KEY_FIFO   (0x280)

//short address registers for reading
#define READ_RXMCR (0x00)
#define READ_PANIDL (0x02)
#define READ_PANIDH (0x04)
#define READ_SADRL (0x06)
#define READ_SADRH (0x08)
#define READ_EADR0 (0x0A)
#define READ_EADR1 (0x0C)
#define READ_EADR2 (0x0E)
#define READ_EADR3 (0x10)
#define READ_EADR4 (0x12)
#define READ_EADR5 (0x14)
#define READ_EADR6 (0x16)
#define READ_EADR7 (0x18)
#define READ_RXFLUSH (0x1a)
#define READ_ORDER (0x20)
#define READ_TXMCR (0x22)
#define READ_ACKTMOUT (0x24)
#define READ_ESLOTG1 (0x26)
#define READ_SYMTICKL (0x28)
#define READ_SYMTICKH (0x2A)
#define READ_PACON0 (0x2C)
#define READ_PACON1 (0x2E)
#define READ_PACON2 (0x30)
#define READ_TXBCON0 (0x34)
#define READ_TXNCON (0x36)  // TXNMTRIG
#define READ_TXG1CON (0x38)
#define READ_TXG2CON (0x3A)
#define READ_ESLOTG23 (0x3C)
#define READ_ESLOTG45 (0x3E)
#define READ_ESLOTG67 (0x40)
#define READ_TXPEND (0x42)
#define READ_WAKECON (0x44)
#define READ_FRMOFFSET (0x46)
#define READ_TXSTAT (0x48)
#define READ_TXBCON1 (0x4A)
#define READ_GATECLK (0x4C)
#define READ_TXTIME (0x4E)
#define READ_HSYMTMRL (0x50)
#define READ_HSYMTMRH (0x52)
#define READ_SOFTRST (0x54)
#define READ_SECON0 (0x58)
#define READ_SECON1 (0x5A)
#define READ_TXSTBL (0x5C)
#define READ_RXSR (0x60)
#define READ_INTSTAT (0x62) // ISRSTS
#define READ_INTCON (0x64)  // INTMSK
#define READ_GPIO (0x66)
#define READ_TRISGPIO (0x68)
#define READ_SLPACK (0x6A)
#define READ_RFCTL (0x6C)
#define READ_SECCR2 (0x6E)
#define READ_BBREG0	(0x70)
#define READ_BBREG1 (0x72)
#define READ_BBREG2 (0x74)
#define READ_BBREG3 (0x76)
#define READ_BBREG4 (0x78)
#define READ_BBREG6 (0x7C)
#define READ_CCAEDTH (0x7E) // RSSITHCCA

//short address registers for writing

#define WRITE_RXMCR (0x01)
#define WRITE_PANIDL (0x03)
#define WRITE_PANIDH (0x05)
#define WRITE_SADRL (0x07)
#define WRITE_SADRH (0x09)
#define WRITE_EADR0 (0x0B)
#define WRITE_EADR1 (0x0D)
#define WRITE_EADR2 (0x0F)
#define WRITE_EADR3 (0x11)
#define WRITE_EADR4 (0x13)
#define WRITE_EADR5 (0x15)
#define WRITE_EADR6 (0x17)
#define WRITE_EADR7 (0x19)
#define WRITE_RXFLUSH (0x1B)
#define WRITE_ORDER (0x21)
#define WRITE_TXMCR (0x23)
#define WRITE_ACKTMOUT (0x25)
#define WRITE_ESLOTG1 (0x27)
#define WRITE_SYMTICKL (0x29)
#define WRITE_SYMTICKH (0x2B)
#define WRITE_PACON0 (0x2D)
#define WRITE_PACON1 (0x2F)
#define WRITE_PACON2 (0x31)
#define WRITE_TXBCON0 (0x35)
#define WRITE_TXNCON (0x37)
#define WRITE_TXG1CON (0x39)
#define WRITE_TXG2CON (0x3B)
#define WRITE_ESLOTG23 (0x3D)
#define WRITE_ESLOTG45 (0x3F)
#define WRITE_ESLOTG67 (0x41)
#define WRITE_TXPEND (0x43)
#define WRITE_WAKECON (0x45)
#define WRITE_FRMOFFSET (0x47)
#define WRITE_TXSTAT (0x49)
#define WRITE_TXBCON1 (0x4B)
#define WRITE_GATECLK (0x4D)
#define WRITE_TXTIME (0x4F)
#define WRITE_HSYMTMRL (0x51)
#define WRITE_HSYMTMRH (0x53)
#define WRITE_SOFTRST (0x55)
#define WRITE_SECCON0 (0x59)
#define WRITE_SECCON1 (0x5B)
#define WRITE_TXSTBL (0x5D)
#define WRITE_RXSR (0x61)
#define WRITE_INTSTAT (0x63)
#define WRITE_INTCON (0x65)
#define WRITE_GPIO (0x67)
#define WRITE_TRISGPIO (0x69)
#define WRITE_SLPACK (0x6B)
#define WRITE_RFCTL (0x6D)
#define WRITE_SECCR2 (0x6F)
#define WRITE_BBREG0 (0x71)
#define WRITE_BBREG1 (0x73)
#define WRITE_BBREG2 (0x75)
#define WRITE_BBREG3 (0x77)
#define WRITE_BBREG4 (0x79)
#define WRITE_BBREG6 (0x7D)
#define WRITE_CCAEDTH (0x7F)

#define CHANNEL_11 0x00
#define CHANNEL_12 0x10
#define CHANNEL_13 0x20
#define CHANNEL_14 0x30
#define CHANNEL_15 0x40
#define CHANNEL_16 0x50
#define CHANNEL_17 0x60
#define CHANNEL_18 0x70
#define CHANNEL_19 0x80
#define CHANNEL_20 0x90
#define CHANNEL_21 0xa0
#define CHANNEL_22 0xb0
#define CHANNEL_23 0xc0
#define CHANNEL_24 0xd0
#define CHANNEL_25 0xe0
#define CHANNEL_26 0xf0

#endif
