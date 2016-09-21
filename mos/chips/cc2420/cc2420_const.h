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

/*
 * Portions copyright (c) 2005, Swedish Institute of Computer Science
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. No names of the contributors  
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef MANSOS_CC2420_CONST_H
#define MANSOS_CC2420_CONST_H

/*
 * All constants are from the Chipcon CC2420 Data Sheet that at one
 * point in time could be found at
 * http://www.chipcon.com/files/CC2420_Data_Sheet_1_4.pdf
 *
 * The page numbers below refer to pages in this document.
 */


/* Page 48. */
#define CC2420_RSSI_OFFSET -45

/* Page 27. */
enum cc2420_status_byte {
  CC2420_XOSC16M_STABLE = 6,
  CC2420_TX_UNDERFLOW   = 5,
  CC2420_ENC_BUSY   = 4,
  CC2420_TX_ACTIVE  = 3,
  CC2420_LOCK       = 2,  /* PLL is in lock */
  CC2420_RSSI_VALID = 1,
};

/* Page 27. */
enum cc2420_memory_size {
  CC2420_RAM_SIZE   = 368,
  CC2420_FIFO_SIZE  = 128,
};

/* Page 29. */
enum cc2420_address {
  CC2420RAM_TXFIFO  = 0x000,
  CC2420RAM_RXFIFO  = 0x080,
  CC2420RAM_KEY0    = 0x100,
  CC2420RAM_RXNONCE = 0x110,
  CC2420RAM_SABUF   = 0x120,
  CC2420RAM_KEY1    = 0x130,
  CC2420RAM_TXNONCE = 0x140,
  CC2420RAM_CBCSTATE    = 0x150,
  CC2420RAM_IEEEADDR    = 0x160,
  CC2420RAM_PANID   = 0x168,
  CC2420RAM_SHORTADDR   = 0x16A,
};

/* Page 60. */
enum cc2420_register {
  CC2420_SNOP       = 0x00,
  CC2420_SXOSCON    = 0x01,
  CC2420_STXCAL     = 0x02,
  CC2420_SRXON      = 0x03,
  CC2420_STXON      = 0x04,
  CC2420_STXONCCA   = 0x05,
  CC2420_SRFOFF     = 0x06,
  CC2420_SXOSCOFF   = 0x07,
  CC2420_SFLUSHRX   = 0x08,
  CC2420_SFLUSHTX   = 0x09,
  CC2420_SACK       = 0x0A,
  CC2420_SACKPEND   = 0x0B,
  CC2420_SRXDEC     = 0x0C,
  CC2420_STXENC     = 0x0D,
  CC2420_SAES       = 0x0E,
  CC2420_foo        = 0x0F,
  CC2420_MAIN       = 0x10,
  CC2420_MDMCTRL0   = 0x11,
  CC2420_MDMCTRL1   = 0x12,
  CC2420_RSSI       = 0x13,
  CC2420_SYNCWORD   = 0x14,
  CC2420_TXCTRL     = 0x15,
  CC2420_RXCTRL0    = 0x16,
  CC2420_RXCTRL1    = 0x17,
  CC2420_FSCTRL     = 0x18,
  CC2420_SECCTRL0   = 0x19,
  CC2420_SECCTRL1   = 0x1A,
  CC2420_BATTMON    = 0x1B,
  CC2420_IOCFG0     = 0x1C,
  CC2420_IOCFG1     = 0x1D,
  CC2420_MANFIDL    = 0x1E,
  CC2420_MANFIDH    = 0x1F,
  CC2420_FSMTC      = 0x20,
  CC2420_MANAND     = 0x21,
  CC2420_MANOR      = 0x22,
  CC2420_AGCCTRL    = 0x23,
  CC2420_AGCTST0    = 0x24,
  CC2420_AGCTST1    = 0x25,
  CC2420_AGCTST2    = 0x26,
  CC2420_FSTST0     = 0x27,
  CC2420_FSTST1     = 0x28,
  CC2420_FSTST2     = 0x29,
  CC2420_FSTST3     = 0x2A,
  CC2420_RXBPFTST   = 0x2B,
  CC2420_FSMSTATE   = 0x2C,
  CC2420_ADCTST     = 0x2D,
  CC2420_DACTST     = 0x2E,
  CC2420_TOPTST     = 0x2F,
  CC2420_RESERVED   = 0x30,
  /* 0x31 - 0x3D not used */
  CC2420_TXFIFO     = 0x3E,
  CC2420_RXFIFO     = 0x3F,
};

/* Page 69. */
enum cc2420_secctrl0 {
  CC2420_SECCTRL0_NO_SECURITY       = 0x0000,
  CC2420_SECCTRL0_CBC_MAC       = 0x0001,
  CC2420_SECCTRL0_CTR           = 0x0002,
  CC2420_SECCTRL0_CCM           = 0x0003,

  CC2420_SECCTRL0_SEC_M_IDX     = 2,

  CC2420_SECCTRL0_RXKEYSEL0     = 0x0000,
  CC2420_SECCTRL0_RXKEYSEL1     = 0x0020,

  CC2420_SECCTRL0_TXKEYSEL0     = 0x0000,
  CC2420_SECCTRL0_TXKEYSEL1     = 0x0040,

  CC2420_SECCTRL0_SAKEYSEL0     = 0x0000,
  CC2420_SECCTRL0_SAKEYSEL1     = 0x0080,

  CC2420_SECCTRL0_SEC_CBC_HEAD      = 0x0100,
  CC2420_SECCTRL0_RXFIFO_PROTECTION = 0x0200,
};


// ------------------------------------

#define FOOTER1_CRC_OK      0x80
#define FOOTER1_LQI         0x7f

// MDMCTRL0 constants
enum  {
    MDMCTRL0_ACCEPT_RESERVED_FRAMES = 1 << 13,
    MDMCTRL0_PAN_COORDINATOR = 1 << 12,
    MDMCTRL0_ADDR_DECODE = 1 << 11, // (default on)
    // bits 8 to 10: CCA hysteresis (default 2 dB)
    // bits 6 to 7: CCA mode
    //       0: reserved
    //       1: channel clear if RSSI below threshold-hysteresis 
    //       2: channel clear if not receiving valid frame
    //       3: channel clear if both (default)
    MDMCTRL0_AUTOCRC = 1 << 5,
    MDMCTRL0_AUTOACK = 1 << 4,
    // bits 0 to 3: preamble length (in bytes)
    //       0: 1 byte
    //       1: 2 bytes
    //       2: 3 bytes (default), etc.
};

#define CCA_MODE_RSSI  (1 << 6)
#define CCA_MODE_FRAME (2 << 6)
#define CCA_MODE_COMBINED (CCA_MODE_RSSI | CCA_MODE_FRAME)
#define CCA_MODE_MASK  (3 << 6)

// MDMCTRL1 constants
enum  {
    // bits 6 to 10: demodulator correlator threshold (default 20)
    MDMCTRL1_DEMOD_AVG_MODE = 1 << 5,
    MDMCTRL1_MODULATION_MODE = 1 << 4, // IEEE 802.14.5 compliant or not
    // bits 2 to 3: Tx test mode (default 0)
    // bits 0 to 1: Rx test mode (default 0)
};

#define CORR_THRESHOLD_DEFAULT  (20 << 6)

#define TX_TEST_MODE_MASK   (3 << 2)
#define TX_TEST_MODE_NONE   (0 << 2) // normal operation
#define TX_TEST_MODE_SERIAL (1 << 2)
#define TX_TEST_MODE_CYCLIC (2 << 2)
#define TX_TEST_MODE_RANDOM (3 << 2)

#define RX_TEST_MODE_MASK   3
#define RX_TEST_MODE_NONE   0 // normal operation
#define RX_TEST_MODE_SERIAL 1
#define RX_TEST_MODE_CYCLIC 2

// RXCTRL1 constants
enum  {
    RXCTRL1_RXBPF_LOCUR = 1 << 13,
    RXCTRL1_RXBPF_MIDCUR = 1 << 12,
};

// IOCFG0 constants
#define IOCFG0_FIFOP_THR(n) ((n) & 0x7f)

// SECCTRL0 constants
enum {
    SECCTRL0_RXFIFO_PROTECTION = 1 << 9,
};

#define CCA_THRESHOLD_DEFAULT  -32  // approximately -77 dBm

#define DACTST_CONTINUOUS_WAVE 0x1800


#endif
