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

//==============================================================================
// Hardware controlled SPI, master mode only
// The actual implementation is in HPL. HAL layer provides macros which
// call the real code
//==============================================================================

#ifndef MANSOS_SPI_HW_H
#define MANSOS_SPI_HW_H

#include <stdtypes.h>
#include <platform.h>

/**
 * Initializes SPI bus in either master or slave mode
 * May change Slave-Select pins (platform dependent)!

 * @param   busId   SPI bus ID
 * @param   mode    SPI bus mode: either master or slave
 * @return  0       on success, -1 on error
 */
int8_t hw_spiBusInit(uint8_t busId, SpiBusMode_t spiBusMode);

/**
 * Turn on the SPI bus, provide bus ID (starting from 0)
 *
 * Defined in HPL
 *
 * @param   busId   SPI bus ID
 */
void hw_spiBusOn(uint8_t busId);

/**
 * Turn off the SPI bus, provide bus ID (starting from 0)
 *
 * Defined in HPL
 *
 * @param   busId   SPI bus ID
 */
void hw_spiBusOff(uint8_t busId);

/**
 * Exchange byte with a slave: write a byte to SPI and returns response,
 * received from the slave in full-duplex mode
 * Does not change any Slave-Select pin!
 *
 * Defined in HPL
 *
 * @param   busId   SPI bus ID
 * @param   b       byte to transmit
 * @return          byte received from the slave
 */
uint8_t hw_spiExchByte(uint8_t busId, uint8_t b);

/**
 * Wait end of last SPI reception (block until RX complete)
 *
 * Defined in HPL
 *
 * @param   busId   SPI bus ID
 */
void hw_spiWaitRX(uint8_t busId);

#endif
