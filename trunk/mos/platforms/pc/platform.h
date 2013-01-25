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

#ifndef _PLATFORM_PC_H_
#define _PLATFORM_PC_H_

#include "sem_hal.h"
#include "leds_hal.h"
#include "adc_hal.h"
#include "timers_hal.h"
#include "ints_hal.h"

#include <arch/null_spi.h>

#include <digital.h>

//
// As stdio.h cannot be included: define some of
// frequently used function prototypes.
//
#ifndef _STDIO_H
extern int sprintf(const char *str, const char *format, ...);
extern int snprintf(const char *str, size_t size, const char *format, ...);
#endif
extern void perror(const char *s);


void initPlatform(void);

#ifndef PRINT_BUFFER_SIZE
#define PRINT_BUFFER_SIZE 127
#endif

// LEDs: all present! Defined in pc/ledslist.h

// number of USARTs
#define SERIAL_COUNT 1
// use the only "USART" for PRINTF
#define PRINTF_SERIAL_ID 0

// SD card ID
#define SDCARD_SPI_ID 0

#define RADIO_CHIP  RADIO_CHIP_SOFTWARE // simulation

#endif
