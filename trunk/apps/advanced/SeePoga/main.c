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

//-------------------------------------------
//      SeePoga demo aplication: telosb + SeePoga
//-------------------------------------------

#include <string.h>
#include <stdio.h>

#include "mansos.h"
#include "leds.h"
#include "sleep.h"

//#include "seepoga.h"
#include "devmgr.h"
#include "guiText.h"

#include "dprint.h"
#define DBG_BUFFER_LEN 400

#define REPEAT_RATE 400

//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    static devMgrErr_t ret;
    
    devParams_t params;
    static uint16_t adcVal;

    char * testMsg = "Test message";

    PRINT_INIT(DBG_BUFFER_LEN);

    // ADC value will be stored in adcVal
    params.data = testMsg;

    toggleRedLed();

    ret = devCall(DEV_GUI, GUI_TEXT + GUI_WRITE, DMF_READ, &params);
    PRINTLN( testMsg );

    msleep(REPEAT_RATE);
    toggleRedLed();
}
