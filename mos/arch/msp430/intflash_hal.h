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

#ifndef INT_FLASH_MSP430_HAL_H
#define INT_FLASH_MSP430_HAL_H

#include <msp430/msp430_flash.h>

#define intFlashErase(address, length) msp430flashErase(address, length)
#define intFlashWrite(address, buffer, length)  msp430flashWrite(address, buffer, length)
#define intFlashWriteBlock(address, buffer, length)  msp430flashWriteBlock(address, buffer, length)
#define intFlashRead(address, buffer, length)  msp430flashRead(address, buffer, length)

#define INT_FLASH_SIZE          MSP430_FLASH_SIZE
#define INT_FLASH_SEGMENT_SIZE  MSP430_FLASH_SEGMENT_SIZE
#define INT_FLASH_BLOCK_SIZE    MSP430_FLASH_BLOCK_SIZE
#define INT_FLASH_START         MSP430_FLASH_START
#define INT_FLASH_INFOMEM_START MSP430_FLASH_INFOMEM_START

#endif
