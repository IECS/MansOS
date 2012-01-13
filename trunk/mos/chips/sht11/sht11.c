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

//-------------------------------------------
// Driver for SHT11 Humidity sensor
//-------------------------------------------

#include "sht11.h"
#include <hil/udelay.h>

#define SHT11_SEND_START_SEQ() \
    SHT11_SDA_OUT(); \
    SHT11_SDA_HI(); \
    SHT11_CLK_HI(); \
    SHT11_SDA_LO(); \
    SHT11_CLK_LO(); \
    SHT11_CLK_HI(); \
    SHT11_SDA_HI(); \
    SHT11_CLK_LO();

// pull data low, create clock cycle
#define SHT11_SEND_ACK() \
    SHT11_SDA_OUT(); \
    SHT11_SDA_LO(); \
    SHT11_CLK_HI(); \
    SHT11_CLK_LO();

// wait for operation (measurement) to complete
// SHT11 signals completion of measurement by pulling data low
// TODO: should add maximal timeout to this!
#define SHT11_WAIT() \
    while (SHT11_SDA_GET()) {}

// perform 9 clock cycles while holding data high
// wait 15ms afterwards
// 11ms required for sensor to start up, wait 15ms just to be sure
void sht11_conn_reset() {
    SHT11_SDA_OUT();
    SHT11_SDA_HI();
    SHT11_CLK_LO();
    register uint_t i;
    for (i = 0; i < 9; ++i) {
        SHT11_CLK_HI();
        SHT11_CLK_LO();
    }
    mdelay(15);
}


// Warning: Send MSB first!!!
void sht11_send_byte(uint8_t b) {
    register uint_t i;
    for (i = 0; i < 8; ++i) {
        // set data line, pull clock high and low
        SHT11_SDA_SET(b & 0x80);
        b <<= 1;
        SHT11_CLK_HI();
        SHT11_CLK_LO();
    }
}

uint8_t sht11_recv_byte() {
    SHT11_SDA_IN();
    register uint_t i;
    uint8_t res = 0;
    for (i = 0; i < 8; ++i) {
        res <<= 1;
        SHT11_CLK_HI();
        res |= SHT11_SDA_GET();
        SHT11_CLK_LO();
    }
    return res;
}

// pull clk high, measure data line, pull clk low again
// valid ack, when data line is pulled low by sht11
uint_t sht11_recv_ack() {
    SHT11_SDA_IN();
    SHT11_SDA_HI();
    SHT11_CLK_HI();
    uint_t res = !SHT11_SDA_GET();
    SHT11_CLK_LO();
    return res;
}

// send read cmd, return result
uint16_t sht11_cmd(uint_t cmd) {
    if (cmd != SHT11_CMD_TEMP
        && cmd != SHT11_CMD_HUM
        && cmd != SHT11_CMD_RESET) {
        return 0xffff;
    }

    SHT11_SEND_START_SEQ();
    sht11_send_byte(cmd);

    if (!sht11_recv_ack()) {
        return 0xfffe;
    }

    uint16_t res = 0;
    if (cmd == SHT11_CMD_TEMP || cmd == SHT11_CMD_HUM) {
        SHT11_WAIT();

        res = sht11_recv_byte() << 8;
        SHT11_SEND_ACK();
        res |= sht11_recv_byte();
    }
    // CRC not used

    return res;
}
