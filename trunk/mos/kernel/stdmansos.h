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

#ifndef _STD_MANSOS_H_
#define _STD_MANSOS_H_

// include all the things that could be useful

#include "mansos.h"

#include <hil/sleep.h>
#include <hil/alarms.h>
#include <hil/gpio.h>
#ifdef USE_ADC
#include <hil/adc.h>
#endif
#ifdef USE_LEDS
#include <hil/leds.h>
#endif
#ifdef USE_RADIO
#include <hil/radio.h>
#endif
#ifdef USE_SERIAL
#include <hil/usart.h>
#endif
#include <hil/udelay.h>
#include <hil/timers.h>
#include <hil/errors.h>
#ifdef USE_PRINT
#include <lib/dprint.h>
#endif
#ifdef USE_ADDRESSING
#include <net/addr.h>
#endif
#ifdef USE_HUMIDITY
#include <hil/humidity.h>
#endif
#include <hil/light.h>
#ifdef USE_LEDS
#include <hil/blink.h>
#endif

#endif
