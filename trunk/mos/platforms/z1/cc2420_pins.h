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
 * CC2420 SPI bus configuration for Zolertia Z1 platform
 */

#ifndef CC2420_PINS_H
#define CC2420_PINS_H

#include <digital.h>

// Radio attached to USCI 1 SPI BUS
#define CC2420_SPI_ID   1

// To use soft-SPI for communication with CC2420, uncomment the line below and
// define MISO, MOSI and SCLK pins (see spi_soft.h) in your config file!
// #define CC2420_SPI_ID   SPI_BUS_SW

//
// Almost the same as on Tmote Sky, but not quite
//

/* P1.2 - Input: FIFOP from CC2420 */
#define CC2420_FIFO_P_PORT    1
#define CC2420_FIFO_P_PIN     2
#define CC2420_INT_PORT       CC2420_FIFO_P_PORT
#define CC2420_INT_PIN        CC2420_FIFO_P_PIN

/* P1.3 - Input: FIFO from CC2420 */
#define CC2420_FIFO_PORT      1
#define CC2420_FIFO_PIN       3

/* P1.4 - Input: CCA from CC2420 */
#define CC2420_CCA_PORT       1
#define CC2420_CCA_PIN        4

/* P3.0 - Output: SPI Chip Select (CS_N) */
#define CC2420_CSN_PORT       3
#define CC2420_CSN_PIN        0

/* P4.5 - Output: VREG_EN to CC2420 */
#define CC2420_VREG_EN_PORT   4
#define CC2420_VREG_EN_PIN    5

/* P4.6 - Output: RESET_N to CC2420 */
#define CC2420_RESET_N_PORT   4
#define CC2420_RESET_N_PIN    6

/* P4.1 - Input:  SFD from CC2420 */
#define CC2420_SFD_PORT       4
#define CC2420_SFD_PIN        1

/* Pin status. */

#define CC2420_FIFO_IS_1       pinRead(CC2420_FIFO_PORT, CC2420_FIFO_PIN)
#define CC2420_CCA_IS_1        pinRead(CC2420_CCA_PORT, CC2420_CCA_PIN)
#define CC2420_RESET_IS_1      pinRead(CC2420_RESET_PORT, CC2420_RESET_PIN)
#define CC2420_VREG_IS_1       pinRead(CC2420_VREG_EN_PORT, CC2420_VREG_EN_PIN)
#define CC2420_FIFOP_IS_1      pinRead(CC2420_INT_PORT, CC2420_INT_PIN)
#define CC2420_SFD_IS_1        pinRead(CC2420_SFD_PORT, CC2420_SFD_PIN)

/* The CC2420 reset pin. */
#define CC2420_SET_RESET_INACTIVE()   \
    pinSet(CC2420_RESET_N_PORT, CC2420_RESET_N_PIN)
#define CC2420_SET_RESET_ACTIVE()     \
    pinClear(CC2420_RESET_N_PORT, CC2420_RESET_N_PIN)

/* CC2420 voltage regulator enable pin. */
#define CC2420_SET_VREG_ACTIVE()   \
    pinSet(CC2420_VREG_EN_PORT, CC2420_VREG_EN_PIN)
#define CC2420_SET_VREG_INACTIVE() \
    pinClear(CC2420_VREG_EN_PORT, CC2420_VREG_EN_PIN)

/* CC2420 rising edge trigger for external interrupt 0 (FIFOP). */
#define CC2420_FIFOP_INT_INIT() \
    pinIntRising(CC2420_INT_PORT, CC2420_INT_PIN); \
    CC2420_CLEAR_FIFOP_INT();

/* FIFOP on external interrupt 0. */
#define CC2420_ENABLE_FIFOP_INT() \
    pinEnableInt(CC2420_INT_PORT, CC2420_INT_PIN);
#define CC2420_DISABLE_FIFOP_INT() \
    pinDisableInt(CC2420_INT_PORT, CC2420_INT_PIN);
#define CC2420_CLEAR_FIFOP_INT()   \
    pinClearIntFlag(CC2420_INT_PORT, CC2420_INT_PIN);

/* CC2420 functions */
#define CC2420_SPI_ENABLE()   spiSlaveEnable(CC2420_CSN_PORT, CC2420_CSN_PIN)
#define CC2420_SPI_DISABLE()  spiSlaveDisable(CC2420_CSN_PORT, CC2420_CSN_PIN)

#endif
