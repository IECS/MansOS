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

#ifndef MANSOS_NMEA_STREAM_H
#define MANSOS_NMEA_STREAM_H

// GPS NMEA command parsing
// http://en.wikipedia.org/wiki/NMEA_0183


#include "nmea.h"
#include <stdint.h>


// use buffers, when state == BS_READY. set state to BS_PROCESSING to lock
// buffer content for processing. Release lock by setting state to BS_EMPTY
enum BUFFER_STATE {
    BS_EMPTY = 0,
    BS_READY = 1,
    BS_PROCESSING = 2
};

// Store bytes received (from UART) in a buffer
// each command type has its own buffer
extern uint8_t nmeaBuf[NMEA_CMD_COUNT][MAX_NMEA_CMD_SIZE];
extern uint_t nmeaBufState[NMEA_CMD_COUNT];

#define NMEAEND '*'

// parse NMEA stream, update buffer state when cmd ready
// do not use buffers which have already data, which is not yet processed
// use this function as callback for UART RX
void nmeaCharRecv(uint8_t b);


// usage example:
//USARTInit(0, 115200, 0);
//USARTEnableRX(0);
//USARTSetReceiveHandle(0, nmeaCharRecv);
//while (1) {
//    msleep(1000);
//    if (nmeaBufState[NMEA_CMD_GGA] == BS_READY) {
//        nmeaBufState[NMEA_CMD_GGA] = BS_PROCESSING;
//        parseGGA(nmeaBuf[NMEA_CMD_GGA] + 1, MAX_NMEA_CMD_SIZE - 1, &fix);
//        printBuf(nmeaBuf[NMEA_CMD_GGA]);
//        nmeaBufState[NMEA_CMD_GGA] = BS_EMPTY;
//    }
//    if (nmeaBufState[NMEA_CMD_GSA] == BS_READY) {
//        nmeaBufState[NMEA_CMD_GSA] = BS_PROCESSING;
//        parseGSA(nmeaBuf[NMEA_CMD_GSA] + 1, MAX_NMEA_CMD_SIZE - 1, &fix);
//        printBuf(nmeaBuf[NMEA_CMD_GSA]);
//        nmeaBufState[NMEA_CMD_GSA] = BS_EMPTY;
//    }
//}



#endif /* MANSOS_NMEA_STREAM_H */
