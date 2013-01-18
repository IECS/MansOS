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

//
// Maxim DS18B20 1-Wire Digital Thermometer
//
// Parasite power mode is not supported.
//
// TODO: Maybe split the measurement procedure into two parts (initiate
// conversion and read result).
//

#define DS18B20_CHECK_CRC // Define to compute CRC sum of the received data

#include <string.h>

#include <sleep.h>
#include <lib/codec/crc.h>

#include "ds18b20.h"
#include "platform.h"

#define ONEWIRE_PORT DS18B20_PORT
#define ONEWIRE_PIN  DS18B20_PIN
#include <hil/onewire.h>

// DS18B20-specific commands
enum {
    CMD_MATCH_ROM    = 0x55, // MATCH ROM
    CMD_SKIP_ROM     = 0xCC, // SKIP ROM
    CMD_ALARM_SEARCH = 0xEC, // ALARM SEARCH
    CMD_CONVERT      = 0x44, // CONVERT T
    CMD_WRITE        = 0x4E, // WRITE SCRATCHPAD
    CMD_READ         = 0xBE, // READ SCRATCHPAD
    CMD_COPY         = 0x48, // COPY SCRATCHPAD
    CMD_RECALL       = 0xB8, // RECALL EEPROM
    CMD_READ_POWER   = 0xB4  // READ POWER SUPPLY
};

// See the data sheet
#define MEASURE_WAIT_TIME ((750 >> (3 - DS18B20_RESOLUTION)) + 1)

bool ds18b20Init(void)
{
    Handle_t h;

    if (owreset())
    {
        return false;
    }

    // Set up resolution
    ATOMIC_START(h);
    owwriteb(CMD_SKIP_ROM);
    owwriteb(CMD_WRITE);
    owwriteb(0); // Not used
    owwriteb(0); // Not used
    owwriteb(DS18B20_RESOLUTION << 5); // See the data sheet
    ATOMIC_END(h);

    return true;
}

int16_t ds18b20Measure(void)
{
    Handle_t  h;
    int16_t   res;
    uint8_t  *data = (uint8_t *)&res;
#ifdef DS18B20_CHECK_CRC
    uint8_t   i, crc;
    uint16_t  acc = 0;
#endif

    // Start conversion
    if (owreset())
    {
        // Fail
        return 0xDEAD; // This value is outside of possible measurement range
    }
    ATOMIC_START(h);
    owwriteb(CMD_SKIP_ROM);
    owwriteb(CMD_CONVERT);
    ATOMIC_END(h);

    // Wait
    mdelay(MEASURE_WAIT_TIME);

    // Retrieve result
    if (owreset())
    {
        return 0xDEAD;
    }
    ATOMIC_START(h);
    owwriteb(CMD_SKIP_ROM);
    owwriteb(CMD_READ);
    data[0] = owreadb(); // Little-endian assumed
    data[1] = owreadb();
#ifdef DS18B20_CHECK_CRC
    acc = crc8Add(acc, data[0]);
    acc = crc8Add(acc, data[1]);
    for (i = 2; i < 8; i++)
    {
        acc = crc8Add(acc, owreadb());
    }
    crc = owreadb();
#endif
    ATOMIC_END(h);
#ifdef DS18B20_CHECK_CRC
    if (acc != crc)
    {
        return 0xDEAD;
    }
#endif

    return res;
}
