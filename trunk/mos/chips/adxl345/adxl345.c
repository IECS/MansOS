/*
 * Copyright (c) 2012 the MansOS team. All rights reserved.
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
 *         Device drivers for adxl345 accelerometer in Zolertia Z1.
 * \author
 *         Marcus Lundén, SICS <mlunden@sics.se>
 *         Enric M. Calvo, Zolertia <ecalvo@zolertia.com>
 *
 * Modified by:
 *         Girts Strazdins
 *         Atis Elsts, EDI <atis.elsts@gmail.com>
 */


#include <stdmansos.h>
#include "adxl345.h"
#include <platform.h>
#include <platforms/z1/i2cmaster.h>

#define ADXL345_DEBUG 0

#if ADXL345_DEBUG
#include <lib/dprint.h>
#define ADXLPRINTF(...) PRINTF(__VA_ARGS__)
#else
#define ADXLPRINTF(...) do {} while (0)
#endif

#ifndef PLATFORM_Z1 
#error Zolertia-specific I2C used in this file, bailing out!
#endif

/* ADXL345 registers */
#define ADXL345_DEVID           0x00    // read only
/* registers 0x01 to 0x1C are reserved, do not access */
#define ADXL345_THRESH_TAP      0x1D
#define ADXL345_OFSX            0x1E
#define ADXL345_OFSY            0x1F
#define ADXL345_OFSZ            0x20
#define ADXL345_DUR             0x21
#define ADXL345_LATENT          0x22
#define ADXL345_WINDOW          0x23
#define ADXL345_THRESH_ACT      0x24
#define ADXL345_THRESH_INACT    0x25
#define ADXL345_TIME_INACT      0x26
#define ADXL345_ACT_INACT_CTL   0x27
#define ADXL345_THRESH_FF       0x28
#define ADXL345_TIME_FF         0x29
#define ADXL345_TAP_AXES        0x2A
#define ADXL345_ACT_TAP_STATUS  0x2B    // read only
#define ADXL345_BW_RATE         0x2C
#define ADXL345_POWER_CTL       0x2D
#define ADXL345_INT_ENABLE      0x2E
#define ADXL345_INT_MAP         0x2F
#define ADXL345_INT_SOURCE      0x30    // read only
#define ADXL345_DATA_FORMAT     0x31
#define ADXL345_DATAX0          0x32    // read only, LSByte X, two's complement
#define ADXL345_DATAX1          0x33    // read only, MSByte X
#define ADXL345_DATAY0          0x34    // read only, LSByte Y
#define ADXL345_DATAY1          0x35    // read only, MSByte X
#define ADXL345_DATAZ0          0x36    // read only, LSByte Z
#define ADXL345_DATAZ1          0x37    // read only, MSByte X
#define ADXL345_FIFO_CTL        0x38
#define ADXL345_FIFO_STATUS     0x39    // read only

/* Suggested defaults according to the data sheet etc */
#define ADXL345_THRESH_TAP_DEFAULT      0x48    // 4.5g (0x30 == 3.0g) (datasheet: 3g++)
#define ADXL345_OFSX_DEFAULT            0x00    // for individual units calibration purposes
#define ADXL345_OFSY_DEFAULT            0x00
#define ADXL345_OFSZ_DEFAULT            0x00
#define ADXL345_DUR_DEFAULT             0x20    // 20 ms (datasheet: 10ms++)
#define ADXL345_LATENT_DEFAULT          0x50    // 100 ms (datasheet: 20ms++)
#define ADXL345_WINDOW_DEFAULT          0xFF    // 320 ms (datasheet: 80ms++)
#define ADXL345_THRESH_ACT_DEFAULT      0x15    // 1.3g (62.5 mg/LSB)
#define ADXL345_THRESH_INACT_DEFAULT    0x08    // 0.5g (62.5 mg/LSB)
#define ADXL345_TIME_INACT_DEFAULT      0x02    // 2 s (1 s/LSB)
#define ADXL345_ACT_INACT_CTL_DEFAULT   0xFF    // all axis involved, ac-coupled
#define ADXL345_THRESH_FF_DEFAULT       0x09    // 563 mg
#define ADXL345_TIME_FF_DEFAULT         0x20    // 160 ms
#define ADXL345_TAP_AXES_DEFAULT        0x07    // all axis, no suppression

/* The adxl345 has programmable sample rates, but unexpected results may occur if the wrong
  rate and I2C bus speed is used (see datasheet p 17). Sample rates in Hz. This
  setting does not change the internal sampling rate, just how often it is piped
  to the output registers (ie the interrupt features use the full sample rate
  internally).
  */
#define ADXL345_SRATE_3200      0x0F    // NB don't use at all as I2C data rate<= 400kHz (see datasheet)
#define ADXL345_SRATE_1600      0x0E    // NB don't use at all as I2C data rate<= 400kHz (see datasheet)
#define ADXL345_SRATE_800       0x0D    // when I2C data rate == 400 kHz
#define ADXL345_SRATE_400       0x0C    // when I2C data rate == 400 kHz
#define ADXL345_SRATE_200       0x0B    // when I2C data rate >= 100 kHz
#define ADXL345_SRATE_100       0x0A    // when I2C data rate >= 100 kHz
#define ADXL345_SRATE_50        0x09    // when I2C data rate >= 100 kHz
#define ADXL345_SRATE_25        0x08    // when I2C data rate >= 100 kHz
#define ADXL345_SRATE_12_5      0x07    // 12.5 Hz, when I2C data rate >= 100 kHz
#define ADXL345_SRATE_6_25      0x06    // when I2C data rate >= 100 kHz
#define ADXL345_SRATE_3_13      0x05    // when I2C data rate >= 100 kHz
#define ADXL345_SRATE_1_56      0x04    // when I2C data rate >= 100 kHz
#define ADXL345_SRATE_0_78      0x03    // when I2C data rate >= 100 kHz
#define ADXL345_SRATE_0_39      0x02    // when I2C data rate >= 100 kHz
#define ADXL345_SRATE_0_20      0x01    // when I2C data rate >= 100 kHz
#define ADXL345_SRATE_0_10      0x00    // 0.10 Hz, when I2C data rate >= 100 kHz


#define ADXL345_BW_RATE_DEFAULT         (0x00 | ADXL345_SRATE_100)   // 100 Hz, normal operation
#define ADXL345_POWER_CTL_DEFAULT       0x28      // link bit set, no autosleep, start normal measuring
#define ADXL345_INT_ENABLE_DEFAULT      0x00    // no interrupts enabled
#define ADXL345_INT_MAP_DEFAULT         0x00    // all mapped to int_1

/* NB: In the data format register, data format of axis readings is chosen
  between left or right justify. This affects the position of the MSB/LSB and is
  different depending on g-range and resolution. If changed, make sure this is
  reflected in the _read_axis() function. Also, the resolution can be increased
  from 10 bit to at most 13 bit, but this also changes position of MSB etc on data
  format so check this in read_axis() too. */
#define ADXL345_DATA_FORMAT_DEFAULT     (0x00 | ADXL345_RANGE_2G)    // right-justify, 2g, 10-bit mode, int is active high
#define ADXL345_FIFO_CTL_DEFAULT        0x00    // FIFO bypass mode


/* Default values for adxl345 at startup. This will be sent to the adxl345 in a
    stream at init to set it up in a default state */
static uint8_t adxl345_default_settings[] = {
    /* Note, as the two first two bulks are to be written in a stream, they contain
       the register address as first byte in that section. */
    /* 0--14 are in one stream, start at ADXL345_THRESH_TAP */
    ADXL345_THRESH_TAP,         // NB Register address, not register value!!
    ADXL345_THRESH_TAP_DEFAULT,
    ADXL345_OFSX_DEFAULT,
    ADXL345_OFSY_DEFAULT,
    ADXL345_OFSZ_DEFAULT,
    ADXL345_DUR_DEFAULT,
    ADXL345_LATENT_DEFAULT,
    ADXL345_WINDOW_DEFAULT,
    ADXL345_THRESH_ACT_DEFAULT,
    ADXL345_THRESH_INACT_DEFAULT,
    ADXL345_TIME_INACT_DEFAULT,
    ADXL345_ACT_INACT_CTL_DEFAULT,
    ADXL345_THRESH_FF_DEFAULT,
    ADXL345_TIME_FF_DEFAULT,
    ADXL345_TAP_AXES_DEFAULT,

    /* 15--19 start at ADXL345_BW_RATE */
    ADXL345_BW_RATE,    // NB Register address, not register value!!
    ADXL345_BW_RATE_DEFAULT,
    ADXL345_POWER_CTL_DEFAULT,
    ADXL345_INT_ENABLE_DEFAULT,
    ADXL345_INT_MAP_DEFAULT,

    /* These two: 20, 21 write separately */
    ADXL345_DATA_FORMAT_DEFAULT,
    ADXL345_FIFO_CTL_DEFAULT
};

// ------------------------------------------------

static void adxl345_writeReg8(uint8_t regAddr, int8_t regVal)
{
    uint8_t tx_buf[] = {regAddr, regVal};

    i2c_transmitinit(ADXL345_ADDR);
    while (i2c_busy());
    ADXLPRINTF("I2C Ready to TX\n");

    i2c_transmit_n(2, tx_buf);
    while (i2c_busy());
    ADXLPRINTF("WRITE_REG 0x%02X @ reg 0x%02X\n", regVal, regAddr);
}

static void adxl345_writeStream(uint8_t *data, uint8_t len)
{
    i2c_transmitinit(ADXL345_ADDR);
    while (i2c_busy());
    ADXLPRINTF("I2C Ready to TX(stream)\n");

    i2c_transmit_n(len, data); // start tx and send conf reg 
    while (i2c_busy());
    ADXLPRINTF("WRITE_REG %u B to 0x%02X\n", len, data[0]);
}

static uint8_t adxl345_readReg8(uint8_t regAddr)
{
    uint8_t retVal = 0;
    uint8_t rtx = regAddr;
    ADXLPRINTF("READ_REG 0x%02X\n", regAddr);

    /* transmit the register to read */
    i2c_transmitinit(ADXL345_ADDR);
    while (i2c_busy());
    i2c_transmit_n(1, &rtx);
    while (i2c_busy());

    /* receive the data */
    i2c_receiveinit(ADXL345_ADDR);
    while (i2c_busy());
    i2c_receive_n(1, &retVal);
    while (i2c_busy());

    return retVal;
}

static int16_t adxl345_readReg16(uint8_t regAddr)
{
    uint8_t rtx = regAddr;
    ADXLPRINTF("READ_REG16 0x%02X\n", regAddr);

    /* transmit the register to start reading from */
    i2c_transmitinit(ADXL345_ADDR);
    while (i2c_busy());
    i2c_transmit_n(1, &rtx);
    while (i2c_busy());

    /* receive the data */
    i2c_receiveinit(ADXL345_ADDR);
    while (i2c_busy());

    uint8_t tmp[2];
    i2c_receive_n(2, tmp);
    while (i2c_busy());

    return (int16_t)(tmp[0] | (tmp[1]<<8));  
}

/**
 * Initialize modules and pin directions.
 * Slave device I2C address must be specified in HAL layer
 * Initializes I2C bus automatically
 * Does not power up the sensor
 * Returns 0 on success, ERR_MISSING_COMPONENT, if I2C or GPIO is missing
 */
uint8_t adxl345Init(void)
{
    ADXLPRINTF("ADXL345 init\n");

    Handle_t handle;
    INTERRUPT_ENABLED_START(handle);

    // Set up ports and pins for I2C communication
    i2c_enable();

    // Set default register values
    adxl345_writeStream(&adxl345_default_settings[0], 15);
    adxl345_writeStream(&adxl345_default_settings[15], 5);
    adxl345_writeReg8(ADXL345_DATA_FORMAT, adxl345_default_settings[20]);
    adxl345_writeReg8(ADXL345_FIFO_CTL, adxl345_default_settings[21]);

    INTERRUPT_ENABLED_END(handle);

    return 0;
}

/**
 * Read acceleration on specified axis
 */
int16_t adxl345ReadAxis(AdxlAxis_t axis) {
    if (axis > ADXL345_Z_AXIS) return 0;
    return adxl345_readReg16(ADXL345_DATAX0 + axis);
}

/**
 * Set G range for the sensor. Return 0 on success, EINVAL on incorrect grange
 */
uint8_t adxl345SetGRange(AdxlGRange_t grange)
{
    if (grange > ADXL345_RANGE_16G) {
        // invalid g-range.
        ADXLPRINTF("ADXL grange invalid: %u\n", grange);
        return EINVAL;
    }
    /* preserve the previous contents of the register */
    // zero out the last two bits (grange)
    uint8_t r = adxl345_readReg8(ADXL345_DATA_FORMAT) & 0xFC;
    r |= grange;                                      // set new range
    adxl345_writeReg8(ADXL345_DATA_FORMAT, r);
    return 0;
}
