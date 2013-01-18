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
 * Humidity & temperature sensor reading
 */

#ifndef MANSOS_HUMIDITY_H
#define MANSOS_HUMIDITY_H

#include <kernel/stdtypes.h>

//===========================================================
// Data types and constants
//===========================================================


//===========================================================
// Procedures
//===========================================================

// such functions are accessible (defined in platform-specific part)
extern inline void humidityInit(void);      // init humidity sensor, do not turn it on
extern inline void humidityOn(void);        // turn on humidity sensor
extern inline void humidityOff(void);       // turn off humidity sensor
extern inline uint16_t humidityRead();      // read humidity value
extern inline bool humidityIsError(void);

// Humidity sensors also provide temperature reading
extern inline uint16_t temperatureRead(void);  // read temperature value from humidity sensor

// include the definitions of these functions/macros
#include "humidity_hal.h"

#endif
