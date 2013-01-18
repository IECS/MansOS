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

//-----------------------------------------------------------------------------
// Read GPS NMEA stream, parse NMEA commands. To use, set nmeaCharRecv as
// callback for UART RX
//-----------------------------------------------------------------------------

//#include "mansos.h"
//#include "dprint.h"
//#include "usart.h"
//#include "udelay.h"
//#include "leds.h"
#include "nmea_stream.h"
#include <stdint.h>


enum ParseState {
    RS_WAIT_HEADER = 1,
    RS_PARSE_CMD,
    RS_READ_CMD,
    RS_DONE
};

enum {
    CMD_LEN = 3,
};

// public variables
uint8_t nmeaBuf[NMEA_CMD_COUNT][MAX_NMEA_CMD_SIZE];
uint_t nmeaBufState[NMEA_CMD_COUNT] = { BS_EMPTY, BS_EMPTY };

// private variables
const char NMEAHEAD[] = "$GP";
const char NMEACMD[NMEA_CMD_COUNT][CMD_LEN] = { "GGA", "GSA" };


static uint_t seemsValid[NMEA_CMD_COUNT];

static uint8_t bufPos = 0;

static uint_t readState = RS_WAIT_HEADER;
static uint_t i;

//void printBuf(void *userData);
void nmeaCharRecv(uint8_t b);


// store bytes received from UART in a buffer
// each command type has its own buffer
// update buffer state when cmd ready
// do not use buffers which have already data, which is not yet processed
void nmeaCharRecv(uint8_t b)
{
    static uint_t goodChars = 0;
    static uint_t atLeastOneValid = 0;
    static uint_t actualCmd = -1u;
    switch (readState) {
    case RS_WAIT_HEADER:
        // waiting for header of NMEA cmd
        if (b == NMEAHEAD[goodChars]) {
            // good char received
            if (++goodChars == sizeof(NMEAHEAD) - 1) {
                // whole header ok
                readState = RS_PARSE_CMD;
                goodChars = 0;
                for (i = 0; i < NMEA_CMD_COUNT; ++i) {
                    seemsValid[i] = 1;
                }
            } else {
                // still waiting for some header chars
            }
        } else {
            // bad char received, reset parsing
            goodChars = 0;
        }
        break;
    case RS_PARSE_CMD:
        // look, which commands seem to be valid, scan the remaining chars
        atLeastOneValid = 0;
        for (i = 0; i < NMEA_CMD_COUNT; ++i) {
            if (seemsValid[i]) {
                if (b == NMEACMD[i][goodChars]) {
                    atLeastOneValid = 1;
                } else {
                    // first invalid char for this cmd
                    seemsValid[i] = 0;
                }
            }
        }

        if (atLeastOneValid) {
            ++goodChars;
            if (goodChars == 3) {
                // cmd found, check if buffer is ready (could be, that old
                // cmd is still in it waiting for parsing)
                goodChars = 0;
                for (i = 0; i < NMEA_CMD_COUNT; ++i) {
                    if (seemsValid[i]) {
                        if (nmeaBufState[i] == BS_EMPTY) {
                            // buffer for this cmd ready
                            readState = RS_READ_CMD;
                            actualCmd = i;
                            bufPos = 0;
                        } else {
                            // this cmd is already in the buffer, skip it
                            readState = RS_WAIT_HEADER;
                        }
                        break;
                    }
                }
            }
        } else {
            // another cmd, skip it
            readState = RS_WAIT_HEADER;
            goodChars = 0;
        }
        break;
    case RS_READ_CMD:
        // already got good header, parsing remaining data until the end of cmd
        if (b == NMEAEND) {
            // end of command
            readState = RS_DONE;
            nmeaBufState[actualCmd] = BS_READY;
            readState = RS_WAIT_HEADER;
            goodChars = 0;
        } else {
            // store char in buffer
            if (bufPos < MAX_NMEA_CMD_SIZE - 1) {
                nmeaBuf[actualCmd][bufPos] = b;
            } else if (bufPos < MAX_NMEA_CMD_SIZE) {
                nmeaBuf[actualCmd][bufPos] = 0;
            }
            ++bufPos;
        }
        break;
    default:
        break; // parseState == PS_DONE - cmd received
    }
}
