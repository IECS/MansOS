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

#ifndef ADC_PINS_H
#define ADC_PINS_H

// Testbed platform uses ADS8638 analog/digital converter

#define ADS8638_CS_PORT 3
#define ADS8638_CS_PIN  0

// Testbed platform uses DAC7718 digital/analog converter

#define DAC7718_CS_PORT   1
#define DAC7718_CS_PIN    1

#define DAC7718_RST_PORT  1
#define DAC7718_RST_PIN   1

// selects output voltage after reset
#define DAC7718_RSTSEL_PORT 1
#define DAC7718_RSTSEL_PIN  1

// fix DAC latch (active low)
#define DAC7718_LDAC_PORT 1
#define DAC7718_LDAC_PIN  1

// when 0, all V_out pins set to ground
#define DAC7718_CLR_PORT  1
#define DAC7718_CLR_PIN   1

// when 0, wake up SPI to normal mode from sleep
#define DAC7718_WAKEUP_PORT 1
#define DAC7718_WAKEUP_PIN  1

// use binary two's complement? (if IOV_dd)
#define DAC7718_BTC_PORT 1
#define DAC7718_BTC_PIN  1

// digital ground
// #define DAC7718_DGND_PORT 1
// #define DAC7718_DGND_PIN  1

// // interface power (I/O Voltage digital)
// #define DAC7718_IOVDD_PORT 1
// #define DAC7718_IOVDD_PIN  1

#endif
