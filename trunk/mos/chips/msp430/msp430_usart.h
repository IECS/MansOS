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

#ifndef MSP430_USART_H
#define MSP430_USART_H

#include <kernel/defines.h>

//===========================================================
// Data types and constants
//===========================================================
// SPI Serial Clock (SCK) on P3.3, MOSI on P3.1, MISO on P3.2
// I2C Serial Clock (SCL) on P3.3, SDA on P3.1
// USART0 TX on P3.4, RX on P3.5
// USART1 TX on P3.6, RX on P3.7
#define UTXD0_PORT 3
#define URXD0_PORT 3
#define UTXD1_PORT 3
#define URXD1_PORT 3
#define HW_SCK_PORT   3
#define HW_SCL_PORT   3
#define HW_SDA_PORT   3
#define HW_MOSI_PORT  3
#define HW_MISO_PORT  3

#define HW_MOSI_PIN   1
#define HW_SDA_PIN    1
#define HW_MISO_PIN   2
#define HW_SCK_PIN    3
#define HW_SCL_PIN    3
#define UTXD0_PIN  4
#define URXD0_PIN  5
#define UTXD1_PIN  6
#define URXD1_PIN  7

// available USART count
#define USART_COUNT 2

// use USART 1 for PRINTF
#define PRINTF_USART_ID 1

// USART buffers, defined in .c file
extern volatile uint8_t * const UxTXBUF[USART_COUNT];
extern volatile const uint8_t * const UxRXBUF[USART_COUNT];

// Not on all platforms SPI and I2C is part of USART
uint_t msp430USARTInitSPI(uint8_t id, uint_t spiBusMode);
uint_t msp430USARTInitI2C(uint8_t id);


#endif // !MSP430_USART_H
