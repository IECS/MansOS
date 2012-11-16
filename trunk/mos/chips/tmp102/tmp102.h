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

/**
 * \file
 *         Device drivers header file for tmp102 temperature sensor in Zolertia Z1 WSN Platform.
 * \author
 *         Enric M. Calvo, Zolertia <ecalvo@zolertia.com>
 *         Marcus Lundén, SICS <mlunden@sics.se>
 *
 * Modified by:
 *         Girts Strazdins
 *         Atis Elsts, EDI <atis.elsts@gmail.com>
 */

#ifndef MANSOS_TMP102_H
#define MANSOS_TMP102_H

#include <platform.h>

// TMP102 pins and I2C slave address must be defined in HAL level
#ifndef TMP102_ADDR
#define TMP102_ADDR 0x48
#endif

/**
 * Initialize pin directions, do not power up the sensor
 */
void tmp102Init(void);

/**
 * Read temperature and convert it to celsius degrees, discard decimal part
 * Returns 0 on error
 */
int16_t tmp102ReadDegrees(void);

/**
 * Enable power for the sensor.
 */
#define tmp102On() \
    pinSet(TMP102_PWR_PORT, TMP102_PWR_PIN)

/**
 * Disable power for the sensor.
 */
#define tmp102Off() \
    pinClr(TMP102_PWR_PORT, TMP102_PWR_PIN)

/**
 * Read raw temperature value. Returns 0 on error
 */
#define tmp102ReadRaw() \
    tmp102_readReg(TMP102_TEMP)


//---------------------------------------------------------------
// Internal functions
//---------------------------------------------------------------
/**
 * Read TMP102 register
 */
uint16_t tmp102_readReg(uint8_t regAddr);

#endif
