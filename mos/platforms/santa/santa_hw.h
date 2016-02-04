/* 
 * Message heder for SAntA directional motes
 */
  
#ifndef _santa_hw_h_
#define _santa_hw_h_

// #include "gpio_hal.h"
#include "digital.h"

// This is a unique ID for each antena (1-8) as labeled on the antena
#ifndef NODE_ID
#define NODE_ID 2
#endif

// Santa note has 6 LEDs, as defined in platforms/santa/ledslist.h
// LED names: blueLed, greenLed, redLed, blueLed2, greenLed2, redLed2
// LED aliases: led0 .. led5


//========================================================
// Prepare for Santa configuration pin manipulation macros (private)
//========================================================
// This will create the following inline functions for every pin X:
//      uint_t SantaPinXMask() 
//      uint_t SantaPinXGet() 
//      void SantaPinXSet(uint8_t val)
//      void SantaPinXHi() 
//      void SantaPinXLow() 
//      void SantaPinXInit() 
//========================================================

#define SATNTA_PIN_DEFINE(idx, mask, port, pin) SATNTA_PIN_DEFINE2(idx, mask, port, pin)
#define SATNTA_PIN_DEFINE2(idx, mask, port, pin)  \
    static inline uint_t SantaPin##idx##Mask(void)    { return ((uint_t)(mask)); }    \
    static inline uint_t SantaPin##idx##Get(void)     { return (pinRead(port, pin) ? 1:0); } \
    static inline void SantaPin##idx##Set(uint8_t val) { pinWrite( port, pin, ((val)?1:0) ); } \
    static inline void SantaPin##idx##Hi(void)        { SantaPin##idx##Set(1); }   \
    static inline void SantaPin##idx##Low(void)       { SantaPin##idx##Set(0); }   \
    static inline void SantaPin##idx##Init(void)      { pinAsOutput( port, pin ); SantaPin##idx##Low(); }   \


// Define all antena configuration pins
SATNTA_PIN_DEFINE(0, 0x01, 6,0);
SATNTA_PIN_DEFINE(1, 0x02, 6,1);
SATNTA_PIN_DEFINE(2, 0x04, 6,2);
SATNTA_PIN_DEFINE(3, 0x08, 6,3);
SATNTA_PIN_DEFINE(4, 0x10, 6,7);
SATNTA_PIN_DEFINE(5, 0x20, 6,6);


//========================================================
//  Santa configuration pin macros start here (public)
//========================================================

// Initialize Santa configuration pins
 #define SantaPinInit() \
    SantaPin0Init(); \
    SantaPin1Init(); \
    SantaPin2Init(); \
    SantaPin3Init(); \
    SantaPin4Init(); \
    SantaPin5Init(); 


// Set one pin to a value
#define SantaPinSet(idx, val)  \
    switch(idx){                          \
        case 0: SantaPin0Set(val); break; \
        case 1: SantaPin1Set(val); break; \
        case 2: SantaPin2Set(val); break; \
        case 3: SantaPin3Set(val); break; \
        case 4: SantaPin4Set(val); break; \
        case 5: SantaPin5Set(val); break; \
    }

// Get one pin value
#define SantaPinGet(idx)  ( \
    (idx)==0 ? SantaPin0Get() : \
    (idx)==1 ? SantaPin1Get() : \
    (idx)==2 ? SantaPin2Get() : \
    (idx)==3 ? SantaPin3Get() : \
    (idx)==4 ? SantaPin4Get() : \
    (idx)==5 ? SantaPin5Get() : \
    0)


// Set one pin to hi or low
#define SantaPinHi(idx)  SantaPinSet(idx, 1)
#define SantaPinLow(idx)  SantaPinSet(idx, 0)

// Set pin configuration from a bitmask
#define SantaPinSetCfg(mask)    \
    SantaPin0Set( mask & SantaPin0Mask() ); \
    SantaPin1Set( mask & SantaPin1Mask() ); \
    SantaPin2Set( mask & SantaPin2Mask() ); \
    SantaPin3Set( mask & SantaPin3Mask() ); \
    SantaPin4Set( mask & SantaPin4Mask() ); \
    SantaPin5Set( mask & SantaPin5Mask() ); 

// Get pin configuration, return a bitmask
#define SantaPinGetCfg()  (  \
    ( SantaPin0Mask() ) +    \
    ( SantaPin1Mask() ) +    \
    ( SantaPin2Mask() ) +    \
    ( SantaPin3Mask() ) +    \
    ( SantaPin4Mask() ) +    \
    ( SantaPin5Mask() ) )


#endif  // _santa_hw_h_
