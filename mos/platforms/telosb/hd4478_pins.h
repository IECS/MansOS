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

#ifndef HD4478_PINS_H
#define HD4478_PINS_H

// TelosB platform default HD4478 LCD driver pinout:
// DATA[4:7]    on P6.0 - P6.3
// RS           on P3.4
// E            on P3.5
// RW           on P2.3

#define HD4478_DATA4_PORT 6
#define HD4478_DATA4_PIN  0
#define HD4478_DATA5_PORT 6
#define HD4478_DATA5_PIN  1
#define HD4478_DATA6_PORT 6
#define HD4478_DATA6_PIN  2
#define HD4478_DATA7_PORT 6
#define HD4478_DATA7_PIN  3

#define HD4478_RS_PORT    3
#define HD4478_RS_PIN     4
#define HD4478_EN_PORT    3
#define HD4478_EN_PIN     5
#define HD4478_RW_PORT    2
#define HD4478_RW_PIN     3

#endif // !TELOSB_HD4478_HAL_H
