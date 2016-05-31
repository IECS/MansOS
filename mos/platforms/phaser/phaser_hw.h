/* 
 * Hardware specifics for Phaser directional communications motes.
 */
  
#ifndef _PHASER_HW_H_
#define _PHASER_HW_H_

// #include "gpio_hal.h"
#include "digital.h"


//========================================================
// Phaser version v1d
//========================================================

// Phase shift control potys and pins
//--------------------------------------------------------

// Serial/parallel mode pin
#define PHASER_RFSP_PORT 2
#define PHASER_RFSP_PIN  7

// Serial data in pin
#define PHASER_RFSI_PORT 2
#define PHASER_RFSI_PIN  6

// Serial clock pin
#define PHASER_RFCLK_PORT 2
#define PHASER_RFCLK_PIN  3

// Serial interface latch enable pin
#define PHASER_RFALE_PORT 2
#define PHASER_RFALE_PIN  0
#define PHASER_RFBLE_PORT 2
#define PHASER_RFBLE_PIN  1

// Channel A phase parallel port: D0..D8:
// P6.0, P6.1, P6.2, P6.3, P6.4, P6.5, P6.6, P6.7, P1.6

// Channel B phase parallel port: D0..D8:
// P5.0, P5.1, P5.2, P5.3, P5.4, P5.5, P5.6, P5.7, P1.7

#define PHASER_A_PORT 6
#define PHASER_A_PORT8 1
#define PHASER_A_PIN8 6

#define PHASER_B_PORT 5
#define PHASER_B_PORT8 1
#define PHASER_B_PIN8 7

#define PHASER_A_A0 0
#define PHASER_A_A1 1
#define PHASER_A_A2 2
#define PHASER_A_A3 3

#define PHASER_B_A0 0
#define PHASER_B_A1 1
#define PHASER_B_A2 2
#define PHASER_B_A3 3

// API 
//------------------------------------------
#define PHASER_

// Init - default to parallel mode
#define PHASER_INIT()   PHASER_SET_PARALLEL();

// Set mode to parallel interface. Default all phase channels to 0.
#define PHASER_SET_PARALLEL()    \
    pinAsOutput(PHASER_RFSP_PORT, PHASER_RFSP_PIN); \
    pinClear(PHASER_RFSP_PORT, PHASER_RFSP_PIN); \
    portAsOutput(PHASER_A_PORT);  \
    portAsOutput(PHASER_B_PORT);  \
    pinAsOutput(PHASER_A_PORT8, PHASER_A_PIN8); \
    pinAsOutput(PHASER_B_PORT8, PHASER_B_PIN8); \
    PHASER_A_SET_EXT(0,0); \
    PHASER_B_SET_EXT(0,0); \

// Set mode to serial interface
// Note, the 
#define PHASER_SET_SERIAL()      \
    portAsInput(PHASER_A_PORT);  \
    portAsInput(PHASER_B_PORT);  \
    \
    pinAsOutput(PHASER_A_PORT, PHASER_A_A0); \
    pinAsOutput(PHASER_A_PORT, PHASER_A_A1); \
    pinAsOutput(PHASER_A_PORT, PHASER_A_A2); \
    pinAsOutput(PHASER_A_PORT, PHASER_A_A3); \
    pinClear(PHASER_A_PORT, PHASER_A_A0); \
    pinClear(PHASER_A_PORT, PHASER_A_A1); \
    pinClear(PHASER_A_PORT, PHASER_A_A2); \
    pinClear(PHASER_A_PORT, PHASER_A_A3); \
    \
    pinAsOutput(PHASER_B_PORT, PHASER_B_A0); \
    pinAsOutput(PHASER_B_PORT, PHASER_B_A1); \
    pinAsOutput(PHASER_B_PORT, PHASER_B_A2); \
    pinAsOutput(PHASER_B_PORT, PHASER_B_A3); \
    pinClear(PHASER_B_PORT, PHASER_B_A0); \
    pinClear(PHASER_B_PORT, PHASER_B_A1); \
    pinClear(PHASER_B_PORT, PHASER_B_A2); \
    pinClear(PHASER_B_PORT, PHASER_B_A3); \
    \
    pinAsOutput(PHASER_RFCLK_PORT, PHASER_RFCLK_PIN); \
    pinAsOutput(PHASER_RFSI_PORT, PHASER_RFSI_PIN); \
    pinAsOutput(PHASER_RFALE_PORT, PHASER_RFALE_PIN); \
    pinAsOutput(PHASER_RFBLE_PORT, PHASER_RFBLE_PIN); \
    pinClear(PHASER_RFCLK_PORT, PHASER_RFCLK_PIN); \
    pinClear(PHASER_RFSI_PORT, PHASER_RFSI_PIN); \
    pinClear(PHASER_RFALE_PORT, PHASER_RFALE_PIN); \
    pinClear(PHASER_RFBLE_PORT, PHASER_RFBLE_PIN); \
    pinAsOutput(PHASER_RFSP_PORT, PHASER_RFSP_PIN); \
    pinSet(PHASER_RFSP_PORT, PHASER_RFSP_PIN); \


// Set phase usong only 8 bits
#define PHASER_A_SET(d07)             \
    portWrite(PHASER_A_PORT, d07);            

#define PHASER_B_SET(d07)             \
    portWrite(PHASER_B_PORT, d07);            

// Set the OPT bit
#define PHASER_A_SET_OPT(d8)  pinWrite(PHASER_A_PORT8, PHASER_A_PIN8, d8);
#define PHASER_B_SET_OPT(d8)  pinWrite(PHASER_B_PORT8, PHASER_B_PIN8, d8);

//Set phase using 8+1 (OPT) bit
#define PHASER_A_SET_EXT(d07, d8)  \
    PHASER_A_SET(d07);      \
    PHASER_A_SET_OPT(d8);

#define PHASER_B_SET_EXT(d07, d8)  \
    PHASER_B_SET(d07);      \
    PHASER_B_SET_OPT(d8);

#endif // _PHASER_HW_H_
