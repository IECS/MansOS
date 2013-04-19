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

#include <adc.h>
#include <digital.h>
#include <print.h>
#include <delay.h>

#define enableAdcPin(port, pin) \
    pinAsFunction(port, pin);   \
    pinAsInput(port, pin);


void adcInit(void)
{
    hplAdcInit();

    // XXX: needed for lynx board to work without specific initialization
#if 0
    hplAdcUseSupplyRef();
    hplAdcOn();

#ifdef P6SEL
    enableAdcPin(6, 0);
    enableAdcPin(6, 1);
    enableAdcPin(6, 2);
    enableAdcPin(6, 3);
    enableAdcPin(6, 4);
    enableAdcPin(6, 5);
    enableAdcPin(6, 6);
    enableAdcPin(6, 7);
#endif
#endif
}

void adcSetChannel(uint8_t ch)
{
    bool wasOn = hplAdcIsOn();
    if (wasOn) hplAdcOff();
    hplAdcSetChannel(ch);
    if (wasOn) hplAdcOn();
}

uint16_t adcRead(uint8_t ch)
{
    uint16_t retval;
    bool wasOn = hplAdcIsOn();

    // set the right channel
    if (hplAdcGetChannel() != ch) {
        if (wasOn) hplAdcOff();
        hplAdcSetChannel(ch);
        if (wasOn) hplAdcOn();
    }

     // turn it on
    if (!wasOn) {
        hplAdcOn();
    }

    // ask the HW for the value
    hplAdcStartConversion();
    // poll while its ready
    while (hplAdcIsBusy()) {}
    // read the value
    retval = hplAdcGetVal();

    // reset back to original state
    if (!wasOn) {
        hplAdcOff();
    }

    return retval;
}
