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

#include <stdio.h>
#include <kernel/stdtypes.h>

#include "leds.h"

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


// Local LED states
typedef struct {
    unsigned id;
    bool isOn;
    char *color;
    char *name;
    char *upcaseName;
} PCLed_t;

static PCLed_t ledData[LED_COUNT] = {
    { RED_LED_NR,    false, TERM_RED,    "red", "RED" },
    { GREEN_LED_NR,  false, TERM_GREEN,  "grn", "GRN" },
    { BLUE_LED_NR,   false, TERM_BLUE,   "blu", "BLU" },
    { YELLOW_LED_NR, false, TERM_YELLOW, "ylw", "YLW" }
};

bool printingSupressed;

// internal functions
PCLed_t *getLedStruct(unsigned id); // get local LED structure by its id (nr)
void togglePcLed(unsigned id); // toggle led by given id (nr)
void pcLedOn(unsigned id); // turn on led by given id (nr)
void pcLedOff(unsigned id); // turn off led by given id (nr)
bool pcIsLedOn(unsigned id); // return true, when led given by id (nr) is on


void printLeds()
{
    if (printingSupressed) return;

    printf("LEDs:");
    unsigned i;
    for (i = 0; i < LED_COUNT; ++i) {
        if (ledData[i].isOn) {
            printf(" %s+%s+%s", ledData[i].color, ledData[i].upcaseName,
                    TERM_RESET);
        } else {
            printf(" -%s-", ledData[i].name);
        }
    }
    printf("\n");
    fflush(stdout);
}

PCLed_t *getLedStruct(unsigned id) {
    unsigned i;
    for (i = 0; i < LED_COUNT; ++i) {
        if (ledData[i].id == id) return &ledData[i];
    }
    return NULL;
}

void togglePcLed(unsigned id) {
    PCLed_t *led = getLedStruct(id);
    if (led) {
        led->isOn = !led->isOn;
        printLeds();
    }
}

void pcLedOn(unsigned id) {
    PCLed_t *led = getLedStruct(id);
    if (led) {
        led->isOn = true;
        printLeds();
    }
}

void pcLedOff(unsigned id) {
    PCLed_t *led = getLedStruct(id);
    if (led) {
        led->isOn = false;
        printLeds();
    }
}

bool pcIsLedOn(unsigned id) {
    PCLed_t *led = getLedStruct(id);
    return led ? led->isOn : false;
}

// -----------------------------

void pinSetPc(int port, int pin)
{
    if (port != LED_PORT_PC) return;
    switch (pin) {
    case LEDS_RED_PIN:
        pcLedOn(RED_LED_NR);
        break;
    case LEDS_GREEN_PIN:
        pcLedOn(GREEN_LED_NR);
        break;
    case LEDS_BLUE_PIN:
        pcLedOn(BLUE_LED_NR);
        break;
    case LEDS_YELLOW_PIN:
        pcLedOn(YELLOW_LED_NR);
        break;
    }
}

void pinClearPc(int port, int pin)
{
    if (port != LED_PORT_PC) return;
    switch (pin) {
    case LEDS_RED_PIN:
        pcLedOff(RED_LED_NR);
        break;
    case LEDS_GREEN_PIN:
        pcLedOff(GREEN_LED_NR);
        break;
    case LEDS_BLUE_PIN:
        pcLedOff(BLUE_LED_NR);
        break;
    case LEDS_YELLOW_PIN:
        pcLedOff(YELLOW_LED_NR);
        break;
    }
}

int pinReadPc(int port, int pin)
{
    if (port != LED_PORT_PC) return 0;
    switch (pin) {
    case LEDS_RED_PIN:
        return pcIsLedOn(RED_LED_NR);
    case LEDS_GREEN_PIN:
        return pcIsLedOn(GREEN_LED_NR);
    case LEDS_BLUE_PIN:
        return pcIsLedOn(BLUE_LED_NR);
    case LEDS_YELLOW_PIN:
        return pcIsLedOn(YELLOW_LED_NR);
    default:
        return 0;
    }
}

void suppressLedOutput(bool yes)
{
    if (printingSupressed != yes) {
        printingSupressed = yes;
        if (!printingSupressed) {
            printLeds();
        }
    }
}
