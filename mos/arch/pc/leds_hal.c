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

#include <stdio.h>
#include <kernel/stdtypes.h>

#include "leds.h"
#include <print.h>

#define DEBUG_PINS(_)
//#define DEBUG_PINS(name) PRINTF( #name " pin %d:%d \n", port, pin);


#define TERM_ESC "\033"
#define TERM_DULL "0"
#define TERM_FG_RED "31"
#define TERM_FG_GREEN "32"
#define TERM_FG_YELLOW "33"
#define TERM_FG_BLUE "34"
#define TERM_FG_WHITE "37"
#define TERM_BG_NULL "00"
#define TERM_RED TERM_ESC "\[" TERM_DULL ";" TERM_FG_RED "m"
#define TERM_GREEN TERM_ESC "\[" TERM_DULL ";" TERM_FG_GREEN "m"
#define TERM_BLUE TERM_ESC "\[" TERM_DULL ";" TERM_FG_BLUE "m"
#define TERM_YELLOW TERM_ESC "\[" TERM_DULL ";" TERM_FG_YELLOW "m"
#define TERM_NORMAL "\[" TERM_ESC "[m]"
#define TERM_RESET TERM_ESC "[" TERM_DULL ";" TERM_FG_WHITE ";" TERM_BG_NULL "m"
#define TERM_CURSOR_BACK(places) TERM_ESC "[" #places "D"


// How to display on and off
#define PC_LED_ON_PREFIX    TERM_YELLOW "+"
#define PC_LED_ON_POSTFIX  "+" TERM_RESET
#define PC_LED_OFF_PREFIX   TERM_BLUE "-"
#define PC_LED_OFF_POSTFIX  "-" TERM_RESET


// Local LED states
typedef struct {
    unsigned id;
    bool isOn;
    char *name;
    char *upcaseName;
} PCLed_t;


static PCLed_t ledData[LEDS_COUNT] = {
#define DOIT(name) { name##_mask, false, #name,  #name },
#include "ledslist.h"
};

bool printingSupressed;


// Internal functions (id == virtual_pin, led number)
//PCLed_t *getLedStruct(uint_t id); // get local LED structure by its id (nr)
//void pcLedOn(uint_t id); // turn on led by given id (nr)
//void pcLedOff(uint_t id); // turn off led by given id (nr)
//bool pcIsLedOn(uint_t id); // return true, when led given by id (nr) is on

//----------------------------------------------------------
PCLed_t *getLedStruct(uint_t id) 
{
    if (id >= 0 && id < LEDS_COUNT) {
        return &ledData[id];
    }
    return NULL;
}

//----------------------------------------------------------
void printLeds()
{
    if (printingSupressed) return;

    PRINTF("LEDs:");
    uint_t i;
    for (i = 0; i < LEDS_COUNT; i++) {
        if (ledData[i].isOn) 
        {
            PRINTF(" %s%s%s", 
                   PC_LED_ON_PREFIX, 
                   ledData[i].upcaseName,
                   PC_LED_ON_POSTFIX
                   );
        } else 
        {
            PRINTF(" %s%s%s", 
                   PC_LED_OFF_PREFIX, 
                   ledData[i].name,
                   PC_LED_OFF_POSTFIX
                   );
        }
    }
    PRINTF("\n");
    // fflush(stdout);
}

//----------------------------------------------------------
void pcLedOn(uint_t id) {
    PCLed_t *led = getLedStruct(id);
    if (led) {
        led->isOn = true;
        printLeds();
    }
}

//----------------------------------------------------------
void pcLedOff(uint_t id) {
    PCLed_t *led = getLedStruct(id);
    if (led) {
        led->isOn = false;
        printLeds();
    }
}

//----------------------------------------------------------
bool pcIsLedOn(uint_t id) {
    PCLed_t *led = getLedStruct(id);
    return led ? led->isOn : false;
}


//----------------------------------------------------------
//----------------------------------------------------------
void pinSetPc(int port, int pin)
{
    DEBUG_PINS(Set);

    if (port != LEDS_PORT) return;
    pcLedOn(pin);
}

//----------------------------------------------------------
void pinClearPc(int port, int pin)
{
    DEBUG_PINS(Clr);
 
    if (port != LEDS_PORT) return;
    pcLedOff(pin);
}

//----------------------------------------------------------
int pinReadPc(int port, int pin)
{
    DEBUG_PINS(Red);

    if (port != LEDS_PORT) return 0;
    return pcIsLedOn(pin);
}

//----------------------------------------------------------
void suppressLedOutput(bool yes)
{
    if( yes != printingSupressed ){
            printLeds();
    }
    printingSupressed = yes;
}
