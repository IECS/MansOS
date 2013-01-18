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

#ifndef MANSOS_SDCARD_CONST_H
#define MANSOS_SDCARD_CONST_H

#include <stdint.h>

// Based on the document:
//
// SD Specifications
// Part 1
// Physical Layer
// Simplified Specification
// Version 3.01
// May 18, 2010
//
// http://www.sdcard.org/developers/tech/sdcard/pls/simplified_specs

//------------------------------------------------------------------------------

// SD card commands
/** GO_IDLE_STATE - init card in spi mode if CS low */
#define CMD_GO_IDLE_STATE 0x00
/** SEND_OP_COND */
#define CMD_SEND_OP_COND  0x01
/** SEND_IF_COND - verify SD Memory Card interface operating condition.*/
#define CMD_SEND_IF_COND 0x08
/** SEND_CSD - read the Card Specific Data (CSD register) */
#define CMD_SEND_CSD 0x09
/** SEND_CID - read the card identification information (CID register) */
#define CMD_SEND_CID 0x0A
/** STOP_TRANSMISSION - end multiple block read sequence */
#define CMD_STOP_TRANSMISSION 0x0C
/** SEND_STATUS - read the card status register */
#define CMD_SEND_STATUS 0x0D
/** READ_SINGLE_BLOCK - read a single data block from the card */
#define CMD_READ_SINGLE_BLOCK 0x11
/** READ_MULTIPLE_BLOCK - read a multiple data blocks from the card */
#define CMD_READ_MULTIPLE_BLOCK 0x12
/** WRITE_BLOCK - write a single data block to the card */
#define CMD_WRITE_BLOCK 0x18
/** WRITE_MULTIPLE_BLOCK - write blocks of data until a STOP_TRANSMISSION */
#define CMD_WRITE_MULTIPLE_BLOCK 0x19
/** ERASE_WR_BLK_START - sets the address of the first block to be erased */
#define CMD_ERASE_WR_BLK_START 0x20
/** ERASE_WR_BLK_END - sets the address of the last block of the continuous
    range to be erased*/
#define CMD_ERASE_WR_BLK_END 0x21
/** ERASE - erase all previously selected blocks */
#define CMD_ERASE 0x26
/** APP_CMD - escape for application specific command */
#define CMD_APP_CMD 0x37
/** READ_OCR - read the OCR register of a card */
#define CMD_READ_OCR 0x3A
/** SET_WR_BLK_ERASE_COUNT - Set the number of write blocks to be
     pre-erased before writing */
#define ACMD_SET_WR_BLK_ERASE_COUNT 0x17
/** SD_SEND_OP_COMD - Sends host capacity support information and
    activates the card's initialization process */
#define ACMD_SD_SEND_OP_COMD 0x29

//------------------------------------------------------------------------------

/** status for card in the ready state */
#define R1_READY_STATE 0x00
/** status for card in the idle state */
#define R1_IDLE_STATE 0x01
/** status bit for illegal command */
#define  R1_ILLEGAL_COMMAND 0x04
/** start data token for read or write single block*/
#define DATA_START_BLOCK 0xFE
/** stop token for write multiple blocks*/
#define STOP_TRAN_TOKEN 0xFD
/** start data token for write multiple blocks*/
#define WRITE_MULTIPLE_TOKEN 0xFC
/** mask for data response tokens after a write block operation */
#define DATA_RES_MASK 0x1F
/** write data accepted token */
#define DATA_RES_ACCEPTED 0x05

//------------------------------------------------------------------------------

/** Card IDentification (CID) register */
typedef struct CID {
  // byte 0
  /** Manufacturer ID */
  unsigned char mid;
  // byte 1-2
  /** OEM/Application ID */
  char oid[2];
  // byte 3-7
  /** Product name */
  char pnm[5];
  // byte 8
  /** Product revision least significant digit */
  unsigned char prv_m : 4;
  /** Product revision most significant digit */
  unsigned char prv_n : 4;
  // byte 9-12
  /** Product serial number */
  uint32_t psn;
  // byte 13
  /** Manufacturing date year low digit */
  unsigned char mdt_year_high : 4;
  /** not used */
  unsigned char reserved : 4;
  // byte 14
  /** Manufacturing date month */
  unsigned char mdt_month : 4;
  /** Manufacturing date year low digit */
  unsigned char mdt_year_low :4;
  // byte 15
  /** not used always 1 */
  unsigned char always1 : 1;
  /** CRC7 checksum */
  unsigned char crc : 7;
} cid_t;

//------------------------------------------------------------------------------

/** CSD for version 1.00 cards */
typedef struct CSDV1 {
  // byte 0
  unsigned char reserved1 : 6;
  unsigned char csd_ver : 2;
  // byte 1
  unsigned char taac;
  // byte 2
  unsigned char nsac;
  // byte 3
  unsigned char tran_speed;
  // byte 4
  unsigned char ccc_high;
  // byte 5
  unsigned char read_bl_len : 4;
  unsigned char ccc_low : 4;
  // byte 6
  unsigned char c_size_high : 2;
  unsigned char reserved2 : 2;
  unsigned char dsr_imp : 1;
  unsigned char read_blk_misalign :1;
  unsigned char write_blk_misalign : 1;
  unsigned char read_bl_partial : 1;
  // byte 7
  unsigned char c_size_mid;
  // byte 8
  unsigned char vdd_r_curr_max : 3;
  unsigned char vdd_r_curr_min : 3;
  unsigned char c_size_low :2;
  // byte 9
  unsigned char c_size_mult_high : 2;
  unsigned char vdd_w_cur_max : 3;
  unsigned char vdd_w_curr_min : 3;
  // byte 10
  unsigned char sector_size_high : 6;
  unsigned char erase_blk_en : 1;
  unsigned char c_size_mult_low : 1;
  // byte 11
  unsigned char wp_grp_size : 7;
  unsigned char sector_size_low : 1;
  // byte 12
  unsigned char write_bl_len_high : 2;
  unsigned char r2w_factor : 3;
  unsigned char reserved3 : 2;
  unsigned char wp_grp_enable : 1;
  // byte 13
  unsigned char reserved4 : 5;
  unsigned char write_partial : 1;
  unsigned char write_bl_len_low : 2;
  // byte 14
  unsigned char reserved5: 2;
  unsigned char file_format : 2;
  unsigned char tmp_write_protect : 1;
  unsigned char perm_write_protect : 1;
  unsigned char copy : 1;
  /** Indicates the file format on the card */
  unsigned char file_format_grp : 1;
  // byte 15
  unsigned char always1 : 1;
  unsigned char crc : 7;
} csd1_t;

//------------------------------------------------------------------------------

/** CSD for version 2.00 cards */
typedef struct CSDV2 {
  // byte 0
  unsigned char reserved1 : 6;
  unsigned char csd_ver : 2;
  // byte 1
  /** fixed to 0X0E */
  unsigned char taac;
  // byte 2
  /** fixed to 0 */
  unsigned char nsac;
  // byte 3
  unsigned char tran_speed;
  // byte 4
  unsigned char ccc_high;
  // byte 5
  /** This field is fixed to 9h, which indicates READ_BL_LEN=512 Byte */
  unsigned char read_bl_len : 4;
  unsigned char ccc_low : 4;
  // byte 6
  /** not used */
  unsigned char reserved2 : 4;
  unsigned char dsr_imp : 1;
  /** fixed to 0 */
  unsigned char read_blk_misalign :1;
  /** fixed to 0 */
  unsigned char write_blk_misalign : 1;
  /** fixed to 0 - no partial read */
  unsigned char read_bl_partial : 1;
  // byte 7
  /** not used */
  unsigned char reserved3 : 2;
  /** high part of card size */
  unsigned char c_size_high : 6;
  // byte 8
  /** middle part of card size */
  unsigned char c_size_mid;
  // byte 9
  /** low part of card size */
  unsigned char c_size_low;
  // byte 10
  /** sector size is fixed at 64 KB */
  unsigned char sector_size_high : 6;
  /** fixed to 1 - erase single is supported */
  unsigned char erase_blk_en : 1;
  /** not used */
  unsigned char reserved4 : 1;
  // byte 11
  unsigned char wp_grp_size : 7;
  /** sector size is fixed at 64 KB */
  unsigned char sector_size_low : 1;
  // byte 12
  /** write_bl_len fixed for 512 byte blocks */
  unsigned char write_bl_len_high : 2;
  /** fixed value of 2 */
  unsigned char r2w_factor : 3;
  /** not used */
  unsigned char reserved5 : 2;
  /** fixed value of 0 - no write protect groups */
  unsigned char wp_grp_enable : 1;
  // byte 13
  unsigned char reserved6 : 5;
  /** always zero - no partial block read*/
  unsigned char write_partial : 1;
  /** write_bl_len fixed for 512 byte blocks */
  unsigned char write_bl_len_low : 2;
  // byte 14
  unsigned char reserved7: 2;
  /** Do not use always 0 */
  unsigned char file_format : 2;
  unsigned char tmp_write_protect : 1;
  unsigned char perm_write_protect : 1;
  unsigned char copy : 1;
  /** Do not use always 0 */
  unsigned char file_format_grp : 1;
  // byte 15
  /** not used always 1 */
  unsigned char always1 : 1;
  /** checksum */
  unsigned char crc : 7;
} csd2_t;

//------------------------------------------------------------------------------

/** union of old and new style CSD register */
typedef union {
  csd1_t v1;
  csd2_t v2;
} csd_t;

#endif
