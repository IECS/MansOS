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

#ifndef MANSOS_NMEA_H
#define MANSOS_NMEA_H

#include "nmea_types.h"

// GPS NMEA command parsing
// http://en.wikipedia.org/wiki/NMEA_0183

enum CMDS {
    NMEA_CMD_GGA = 0,
    NMEA_CMD_GSA,
    NMEA_CMD_RMC,
    // max cmd count
    NMEA_CMD_COUNT
};

enum {
    // max size of NMEA commands
    MAX_NMEA_CMD_SIZE = 80
};


// parse GPGGA command starting with time stamp (right after GPGGA,)
// example of cmd in the buffer:
// 063645.000,5658.6597,N,02411.7264,E,1,3,1.40,125.5,M,23.5,M,,
// return 0 on success, error otherwise
// GGA command contains:
// * time (without date)
// * latitude, longitude
// * visible satellite count
// * dilution of position (accuracy)
// GGA command DOES NOT contain 2D/3D fix info and date. Use GPGSA instead
// Use GPRMC to get date
uint_t parseGGA(const void *buf, uint_t len, GPSFix_t *fix);

// parse GPGSA command starting with auto-selection field (right after GPGSA,)
// only Fix type (2D/3D) is checked!
// example of cmd in the buffer:
// A,3,14,20,17,31,19,,,,,,,,3.37,1.20,3.15
uint_t parseGSA(const void *buf, uint_t len, GPSFix_t *fix);

// parse GPRMC command starting with time stamp (right after GPRMC,)
// example of cmd in the buffer:
// 055810.68,A,5623.9911,N,02415.2237,E,46.8,179.0,170210,07,E
// return 0 on success, error otherwise
// RMC contains:
// * time
// * date
// * latitude, longitude
uint_t parseRMC(const void *buf, uint_t len, GPSFix_t *fix);


#endif /* MANSOS_GPS_H */
