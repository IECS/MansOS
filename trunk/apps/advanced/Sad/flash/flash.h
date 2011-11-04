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

#ifndef MANSOS_FLASH_H
#define MANSOS_FLASH_H

#include <stdint.h>

/** @brief Power down the device. */
#define DEV_MODE_OFF     0
/** @brief Put the device in low power mode, if applicable. */
#define DEV_MODE_IDLE    1
/** @brief Turn on the device. */
#define DEV_MODE_ON      2
//@}


///////////////////////////////////
// PUBLIC Flash Functions
///////////////////////////////////
/** @brief erase the entire flash */
void st_flash_bulk_erase();

/** @brief initialize the st flash, enter deep power down mode */
void st_flash_init();

/** @brief enter low power mode */
void st_flash_deep_powerdown();

/** @brief read a block of data from addr in flash */
void flashRead(uint32_t addr, void* buffer, uint16_t count);

/** @brief write len bytes (len <= 256) to flash, address must be set before */
void flashWrite(const void *buf, uint16_t len);

/** @brief put the flash into deep powerdown mode or power it up */
uint8_t setFlashMode(uint8_t newMode);

void flashSeek(uint32_t addr);


#endif
