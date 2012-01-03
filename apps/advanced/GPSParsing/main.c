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

//-----------------------------------------------------------------------------
// Read GPS module attached to USART0 at 1520, parse GPGGA commands,
// output on USART1 at 38400
//-----------------------------------------------------------------------------

#include "stdmansos.h"
#include "nmea/nmea_stream.h"
#include "nmea/nmea.h"

static GPSFix_t fix;

#define GPS_INIT() \
    pinAsOutput(GPS_PDN_PORT, GPS_PDN_PIN); \
    pinAsData(GPS_PDN_PORT, GPS_PDN_PIN);

#define GPS_PDN_PORT 2
#define GPS_PDN_PIN 6

#define GPS_OFF() pinSet(GPS_PDN_PORT, GPS_PDN_PIN);
#define GPS_ON() pinClear(GPS_PDN_PORT, GPS_PDN_PIN);

void printBuf(void *buf)
{
    // print cmd in the buffer
    // PRINTF("%s\n", buf);
    PRINTF("%i-%i-%i %i:%i:%i\t", fix.d.year, fix.d.mon, fix.d.day,
            fix.h, fix.m, fix.s);
    PRINTF("%i %i.%i\t", fix.lat.deg, fix.lat.min, fix.lat.mindec);
    PRINTF("%i %i.%i\t", fix.lon.deg, fix.lon.min, fix.lon.mindec);
    PRINTF("%i %i %i\n", fix.q.satcnt, fix.q.fix, fix.q.dop);
}

#define CHECK_CMD(cmd) \
    if (nmeaBufState[NMEA_CMD_##cmd] == BS_READY) { \
        nmeaBufState[NMEA_CMD_##cmd] = BS_PROCESSING; \
        parse##cmd(nmeaBuf[NMEA_CMD_##cmd] + 1, \
            MAX_NMEA_CMD_SIZE - 1, &fix); \
        printBuf(nmeaBuf[NMEA_CMD_##cmd]); \
        nmeaBufState[NMEA_CMD_##cmd] = BS_EMPTY; \
    }

void charRecv(uint8_t b) {
    redLedToggle();
}


//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    PRINTF("GPS started\n");

    GPS_INIT();
    GPS_ON();

    PRINT_INIT(256); // inits UART1 automatically
    if (USARTInit(0, 115200, 0)) redLedOn();
    if (USARTEnableRX(0)) redLedOn();

    // set NMEA stream parsing routine as UART0 callback
    USARTSetReceiveHandle(0, nmeaCharRecv);
    // USARTSetReceiveHandle(0, charRecv);

    // wait for NMEA command buffers to become ready
    while (1) {
        busyWait(100000);
        CHECK_CMD(GGA);
        CHECK_CMD(GSA);
        CHECK_CMD(RMC);
    }
}
