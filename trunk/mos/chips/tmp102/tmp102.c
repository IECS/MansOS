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
 *         Device drivers for tmp102 temperature sensor in Zolertia Z1.
 * \author
 *         Enric M. Calvo, Zolertia <ecalvo@zolertia.com>
 *         Marcus Lundén, SICS <mlunden@sics.se>
 *
 * Modified by:
 *         Girts Strazdins
 *         Atis Elsts, EDI <atis.elsts@gmail.com>
 */

#include "tmp102.h"
#include <platform.h>
#include <platforms/z1/i2cmaster.h>

#define TMP102_DEBUG 0

#if TMP102_DEBUG
#include <lib/dprint.h>
#define TMP102PRINTF(...) PRINTF(__VA_ARGS__)
#else
#define TMP102PRINTF(...) do {} while (0)
#endif

#ifndef PLATFORM_ZOLERTIA 
#error Zolertia-specific I2C used in this file, bailing out!
#endif

/* TMP102 registers */
enum TMP102_REGS {
    TMP102_TEMP  =          0x00,    // read only
    TMP102_CONF  =          0x01,
    TMP102_TLOW  =          0x02,
    TMP102_THIGH =          0x03
};

static void tmp102_writeReg(uint8_t regAddr, uint16_t regVal) __attribute__((unused));
static void tmp102_writeReg(uint8_t regAddr, uint16_t regVal)
{
    uint8_t tx_buf[] = {regAddr, 0x00, 0x00};

    tx_buf[1] = (uint8_t) (regVal >> 8);
    tx_buf[2] = (uint8_t) (regVal & 0x00FF);

    i2c_transmitinit(TMP102_ADDR);
    while (i2c_busy());
    TMP102PRINTF("I2C Ready to TX\n");

    i2c_transmit_n(3, tx_buf);
    while (i2c_busy());
    TMP102PRINTF("WRITE_REG 0x%04X @ reg 0x%02X\n", regVal, regAddr);
}

uint16_t tmp102_readReg(uint8_t regAddr)
{
    uint8_t buf[] = { 0x00, 0x00 };
    uint8_t rtx = regAddr;
    TMP102PRINTF("READ_REG 0x%02X\n", regAddr);

    // transmit the register to read 
    i2c_transmitinit(TMP102_ADDR);
    while (i2c_busy());
    i2c_transmit_n(1, &rtx);
    while (i2c_busy());

    // receive the data 
    i2c_receiveinit(TMP102_ADDR);
    while (i2c_busy());
    i2c_receive_n(2, &buf[0]);
    while (i2c_busy());

    return (uint16_t) (buf[0] << 8 | (buf[1]));
}


/**
 * Initialize pin directions.
 * Slave device I2C address and power pin address must be specified
 * I2C bus must be initialized explicitly!
 * Does not power up the sensor
 * Returns ERR_OK on success, ERR_MISSING_COMPONENT, if I2C or GPIO is missing
 */
void tmp102Init(void) {
    TMP102PRINTF("TMP102 init\n");

    // Set power pin direction
    pinAsOutput(TMP102_PWR_PORT, TMP102_PWR_PIN);
    pinAsData(TMP102_PWR_PORT, TMP102_PWR_PIN);
//    pinDisablePullup(TMP102_PWR_PORT, TMP102_PWR_PIN);

    // Set up ports and pins for I2C communication
    i2c_enable();
}

/**
 * Read temperature and convert it to celsius degrees, discard decimal part
 * Returns 0 on error
 */
int16_t tmp102ReadDegrees(void) {
    int16_t sign = 1;
    int16_t abstemp;
    int16_t raw = tmp102ReadRaw();
    if (raw < 0) {
      abstemp = (raw ^ 0xFFFF) + 1;
      sign = -1;
    } else {
      abstemp = raw;
    }

    /* Integer part of the temperature value */
    return (abstemp >> 8) * sign;
}

