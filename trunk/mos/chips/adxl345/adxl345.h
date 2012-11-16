/**
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

#ifndef MANSOS_ADXL345_H
#define MANSOS_ADXL345_H

#include <kernel/defines.h>

//---------------------------------------------------------------
// Global constants
//---------------------------------------------------------------
// ADXL345 I2C slave address must be defined in HAL level
#ifndef ADXL345_SLAVE_ADDR
#define ADXL345_SLAVE_ADDR (0x53 << 1)
#endif

//---------------------------------------------------------------
// Global types
//---------------------------------------------------------------
typedef enum {
    ADXL345_X_AXIS = 0,
    ADXL345_Y_AXIS = 2,
    ADXL345_Z_AXIS = 4,
} AdxlAxis_t;

/* g-range for DATA_FORMAT register */
typedef enum {
    ADXL345_RANGE_2G    = 0x00,
    ADXL345_RANGE_4G    = 0x01,
    ADXL345_RANGE_8G    = 0x02,
    ADXL345_RANGE_16G   = 0x03,
} AdxlGRange_t;

//---------------------------------------------------------------
// Global functions
//---------------------------------------------------------------
/**
 * Initialize modules and pin directions.
 * Slave device I2C address must be specified in HAL layer
 * Initializes I2C bus automatically
 * Does not power up the sensor
 * Returns 0 on success, ERR_MISSING_COMPONENT, if I2C or GPIO is missing
 */
uint8_t adxl345Init(void);

/**
 * Read acceleration on specified axis
 */
int16_t adxl345ReadAxis(AdxlAxis_t axis);

/**
 * Set G range for the sensor. Return 0 on success, EINVAL on incorrect grange
 */
uint8_t adxl345SetGRange(AdxlGRange_t grange);


#endif
