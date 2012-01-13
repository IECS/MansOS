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

//  Low level driver for the Himax LCD controller.
// 
//  You can #define either GUI_PORTRAIT or GUI_LANDSCAPE
//  Runtime switching between Landscape or Portrait is not supported
// 


#include "seePoga_hal.h"

// Uncomment the below to enable sharing the LCD data port
// for example, when shared with LEDs
//#define SEEPOGA_SHARE_DATA_PORT


// Uncomment the below to flip the LCD screen upside down.
//#define GUI_UPSIDEDOWN

#define GUI_PORTRAIT      // Comment this for Landscape

#ifdef GUI_LANDSCAPE
#undef GUI_PORTRAIT
#endif


//==============================================================
//      Prototypes
//==============================================================

void write(uint16_t idx, uint16_t data);
void writeGram(GuiColor_t* data, uint16_t len);
void writeGramAt(uint16_t idx, GuiColor_t* data, uint16_t len);
void fillGram(GuiColor_t fill, uint16_t len);
void read(uint16_t idx, uint16_t data);

//==============================================================
//      Data
//==============================================================
#ifdef SEEPOGA_SHARE_DATA_PORT
    uint8_t savedDisplayDataPort;
#endif

#ifdef SEEPOGA_SHARE_DATA_PORT
#define SAVE_LDDATA  savedDisplayDataPort = LDDATA;
#define RESTORE_LDDATA   LDDATA = savedDisplayDataPort;  
#else
#define SAVE_LDDATA
#define RESTORE_LDDATA
#endif


char lcdState;    // LCD state
char lcdBkLtOn;   // LCD Backlight state


uint16_t gScrAddr;  // Current address on the screen (VRAM)
const uint16_t cScrWidthPix  = 128;  // Width of the screen (pixels)
const uint16_t cScrWidthAddr = 256;  // Double the width
const uint16_t cScrHeightPix = 160;  // Height of the screen


//------------------------------------------------------------
// Initialize the component.
//------------------------------------------------------------
bool himax_init()
{
    // Busses
    PORT_AS_OUTPUT(LDVCC);
    PORT_AS_OUTPUT(LDVCI);
    PORT_AS_OUTPUT(LDBKLT);

    PIN_CLEAR(LDVCI);
    clrPin(LDVCC);
    clrPin(LDBKLT);

    PIN_AS_OUTPUT(LDWR);
    PIN_AS_OUTPUT(LDRD);
    PIN_AS_OUTPUT(LDRS);
    PIN_AS_OUTPUT(LDCS);
    PIN_AS_OUTPUT(LDRST);

    atomic {
      //Init the data line
      PORT_AS_OUTPUT( LDDATA );
      PORT_AS_DATA( LDDATA );
      PORT_WRITE(LDDATA, 0xff);

    }


    call GuiDeviceInput.init();

    //........Other inits...........
    atomic {
        lcdState = LCD_FIRST;
        lcdBkLtOn = 0;

        // LCD API variables
        gScrAddr = 0;
    }

    return SUCCESS;
}


//------------------------------------------------------------
// Start Component
//------------------------------------------------------------
bool himax_start()
{
    // Turn power on
    call GuiDevice.powerOn();
    call GuiDevice.backlightOn();
    return SUCCESS;
} 

//------------------------------------------------------------
// Stop Component
//------------------------------------------------------------
bool himax_stop() 
{
    // Turn power off
    call GuiDevice.backlightOff();
    call GuiDevice.powerOff();
    lcdState = LCD_FIRST;
    return SUCCESS;
}


//===============================================================
//      LCD low level internal routines
//===============================================================

//-------------------------------------------------------
//  Write one byte to the LCD data bus. Toggle the controls.
//-------------------------------------------------------
void lcdWriteByte(uint8_t x)
{
    LDDATA = x;

    TOSH_CLR_LDWR_PIN();    // will write
    TOSH_wait_250ns();      // Wait at least 90ns
    TOSH_SET_LDWR_PIN();    // WR up (write now!)
    TOSH_wait_250ns();      // Wait at least 550ns
}

//-------------------------------------------------------
// LCD write data at index(register)
// Assume all control signals high, and LD lines are output
//-------------------------------------------------------
void write(uint16_t idx, uint16_t data)
{
    // Transfer Upper 8 bits first, then lower 8 bits
    uint8_t dh, dl;

    atomic {
        SAVE_LDDATA;

        dl = ((data) & 0x00ff);              
        dh = (((data)>>8) & 0x00ff); 

        // set up LCD for writing idx
        TOSH_CLR_LDRS_PIN();    // index first
        TOSH_CLR_LDWR_PIN();    // will write (possibly redundant)
        TOSH_CLR_LDCS_PIN();    // select LCD

        // Write index
        lcdWriteByte((uint8_t)0);
        lcdWriteByte((uint8_t)idx);

        TOSH_SET_LDRS_PIN();    // now transfer data
        
        // Write data/instruction
        lcdWriteByte((uint8_t)dh);
        lcdWriteByte((uint8_t)dl);

        TOSH_SET_LDCS_PIN();    // writing done - deselect LCD

        RESTORE_LDDATA;
    }
}

//-------------------------------------------------------
// LCD write GRAM
// Assume all control signals high, and LD lines are output
//-------------------------------------------------------
void writeGram(GuiColor_t* data, uint16_t len)
{
    uint8_t dh, dl;
    uint16_t ii;
    uint16_t *ip;

    ip = data;
    atomic {
        SAVE_LDDATA;

        // set up LCD for writing idx
        TOSH_CLR_LDRS_PIN();    // index first
        TOSH_CLR_LDWR_PIN();    // will write (possibly redundant)
        TOSH_CLR_LDCS_PIN();    // select LCD

        // Write index
        lcdWriteByte((uint8_t)0x00);
        lcdWriteByte((uint8_t)0x22);

        TOSH_SET_LDRS_PIN();    // now transfer data

        for(ii=0; ii<len; ii++){
            // Write data
            dl = (uint8_t)(*ip);              
            dh = (uint8_t)((*ip)>>8); 

            lcdWriteByte((uint8_t)dh);
            lcdWriteByte((uint8_t)dl);

            ip++;
        }
        TOSH_SET_LDCS_PIN();    // writing done - deselect LCD

        RESTORE_LDDATA;
    }
}

//-------------------------------------------------------
// Write a stream of data to the video ram at the given address
//-------------------------------------------------------
void writeGramAt(uint16_t idx, GuiColor_t* data, uint16_t len)
{
    write( 0x21, idx );
    writeGram( data, len );  
}

//-------------------------------------------------------
//      Fill VRAM with one color
//-------------------------------------------------------
void fillGram(GuiColor_t fill, uint16_t len)
{
    uint8_t dh, dl;
    uint16_t ii;

    dl = (uint8_t)(fill);              
    dh = (uint8_t)((fill)>>8); 

    atomic {
        SAVE_LDDATA;

        // set up LCD for writing idx
        TOSH_CLR_LDRS_PIN();    // index first
        TOSH_CLR_LDWR_PIN();    // will write (possibly redundant)
        TOSH_CLR_LDCS_PIN();    // select LCD

        // Write index
        lcdWriteByte((uint8_t)0x00);
        lcdWriteByte((uint8_t)0x22);

        TOSH_SET_LDRS_PIN();    // now transfer data

        for(ii=0; ii<len; ii++){
            // Write data
            lcdWriteByte((uint8_t)dh);
            lcdWriteByte((uint8_t)dl);
        }
        TOSH_SET_LDCS_PIN();    // writing done - deselect LCD

        RESTORE_LDDATA;
    }
}

//-------------------------------------------------------
//      Read VRAM data
//      Assume all control signals high, and LD lines are output
//      TODO: not implemented yet.
//-------------------------------------------------------
void read(uint16_t idx, uint16_t data)
{
    return;
}


//-------------------------------------------------------
//      Setup register values after power-on 
//-------------------------------------------------------
void powerSetup()
{
    // Power-On setup  (page 128, HX8310-A Datasheet)

    write(0x00, 0x0001); // Restart the oscilator
    TOSH_uwait(10000);

    // --mark1--
    // DTE = "0", D1-0 = "00", GON = "0"
    write(0x07, 0x0000); 
    // PON = "0"
    write(0x0d, 0x0000); 
    // VCOMG = "0"
    write(0x0e, 0x0000); 

    // --1ms or more from mark1
    // --mark2--
    TOSH_uwait(10000);
    // --10ms or more from mark2

    //              (Assume VCI = 3V)
    // Set VC2-0    (VC="001" -> REGP := 0.92 x VCI = 0.92 x 3 = 2.76V)
    write(0x0c, 0x0001); 
    // Set VRH3-0   (VRH="0100" -> VGAM1OUT := 1.75 x REGP = 4.83V)
    // PON = "0"
    write(0x0d, 0x0004);  // PON=0, VHR=04h, VDH=VciOUT*1.75 = 4.638V

    // Set VCM4-0, VDV4-0       // VDV=VDH*1.17=5.4264V, VcomH2=VDH*0.98=4.5452V
    write(0x0e, 0x341e);  // VCOMG=1, VDV=14h, VCM=1Eh, (VDH=VGAM1OUT)
                                   
    // DK = "1", Set DC12-10
    write(0x09, 0x000c);  // SAP="100", DC1x="000", DK="1"

    // BT2-0 = "000", Set DC02-00, AP2-0
    write(0x03, 0x0010);  // BT=000b, DC=000b, AP=100b, SLP=0, STB=0

    // PON="1"
    write(0x0d, 0x0014);  // PON=1, VRH=04h
    // write(0x0d, 0x061b);  // PON=1, VRH=0Bh

    // --mark3--
    TOSH_uwait(20000);
    TOSH_uwait(20000);
    // --40ms from mark3

    //  Set BT 2-0
    // DK = "0"
    write(0x09, 0x0004);  // SAP="100", DC1x="000", DK="0"
    // VCOMG = "1"
    write(0x0e, 0x341e);  // VCOMG=1, VDV=14h, VCM=1Eh, (VDH=VGAM1OUT)


    // Do other mode settings here, as needed.
    // ...

    TOSH_uwait(20000);
    TOSH_uwait(20000);
    TOSH_uwait(20000);
    // --100ms from mark3

    // Set SAP2-0   // (Display ON sequence)
    write(0x09, 0x0004);  // SAP="100", DC1x="000", DK="0"

    // DTE = "1", D1-0 = "11", GON = "1"
    write(0x07, 0x0033); 
}

//-------------------------------------------------------
//      Setup chip parameters
//-------------------------------------------------------
void chipSetup()
{
    // Chip setup
    // Screen direction: GS=1 => Bottom-Up, GS=0 =>Top-Down

  // R01:  0000 0 SM GS SS 000 nl4..nl0
  //    SS = LCD source driver output shift direction
  //    GS = LCD gate   driver output shift direction
  //    SM = Scan order of gate driver

#ifdef GUI_PORTRAIT
    //Portrait
#ifdef GUI_UPSIDEDOWN
    write(0x01, 0x0313);  // SS=1, GS=1, NL=13h (160 lines)
#else
    write(0x01, 0x0113);  // SS=1, GS=0, NL=13h (160 lines)
#endif
#else
    //Landscape
#ifdef GUI_UPSIDEDOWN
    write(0x01, 0x0113);  // SS=1, GS=0, NL=13h (160 lines)
#else
    write(0x01, 0x0313);  // SS=1, GS=1, NL=13h (160 lines)
#endif
#endif

    write(0x02, 0x0700);  // FLD=01, B/C=1, EOR=1, NW=00h

    //...........................................................
    // R05: Entry Mode:   000 BGR  00 HVM 0  00 ID1 ID0  AM 000
    //  BGR = BGR - reverse (18) RGB bit order from MPU
    //  HVM = HighSpeedWrite =1 = four words written to RAM at a time
    //  ID = after-write: ID=1 = increment, ID=0 = decrement
    //  AM = Write: 1=vert 0=horiz

#ifdef GUI_PORTRAIT
    //Portrait
    write(0x05, 0x0030);  // HWM=0, I/D=11b, AM=0, LG=000b

#else
    //Landscape
    //write(0x05, 0x0030);  // HWM=0, I/D=11b, AM=0, LG=000b
    write(0x05, 0x0038);  // HWM=0, I/D=11b, AM=1, LG=000b
    //write(0x05, 0x0018);  // HWM=0, I/D=01b, AM=1, LG=000b
    //write(0x05, 0x0028);  // HWM=0, I/D=10b, AM=1, LG=000b
#endif

    //...........................................................
    write(0x06, 0x0000);  // CP=000h
    write(0x0b, 0x0000);  // N0=00b, STD=00b, EQ=00b, DIV=00b, RTN=0000b
    write(0x0f, 0x0000);  // SCN=0000b

    write(0x11, 0x0000);  // VL=00h
    write(0x14, 0x9f00);  // SE1=9Fh, SS1=00h
    write(0x15, 0x9f00);  // SE2=..h, SS2=00h  <-- user defined
    write(0x16, 0x7f00);  // HEA=7Fh, HSA=00h
    write(0x17, 0x9f00);  // VEA=9Fh, VSA=00h

    TOSH_uwait(20000);

    // Gamma setting
    write(0x30, 0x0000);  // 
    write(0x31, 0x0605);  // 
    write(0x32, 0x0407);  // 
    write(0x33, 0x0104);  // 
    write(0x34, 0x0203);  // 
    write(0x35, 0x0303);  // 
    write(0x36, 0x0707);  // 
    write(0x37, 0x0300);  // 
    write(0x3f, 0x0000);  // 
}

//===============================================================
//      GuiDevice interface implementation
//===============================================================

//-------------------------------------------------------
// Init the Lcd hardware and interface
//-------------------------------------------------------
command result_t GuiDevice.reset(){

    lcdState = LCD_BUSY;

    //>>>Profiling A: A..A = 0us
    // Init control lines
    TOSH_SET_LDCS_PIN();    // deselect LCD
    TOSH_SET_LDRD_PIN();    // disable reading
    TOSH_SET_LDWR_PIN();    // disable writing
    TOSH_SET_LDRS_PIN();    // disable command

    TOSH_SET_LDRST_PIN();
    TOSH_uwait(1000);  // optional wait

    // Reset
    //>>>Profiling B: A..B = 2.5us
    TOSH_CLR_LDRST_PIN();
    TOSH_uwait(10000);
    TOSH_SET_LDRST_PIN();
    TOSH_uwait(10000);

    lcdState = LCD_RESET;
    return SUCCESS;
}


//-------------------------------------------------------
//      Turn on the display
//-------------------------------------------------------
command result_t GuiDevice.displayOn()
{
    // Display on  (page 126, HX8310-A Datasheet)
    lcdState = LCD_BUSY;

//  Should we Set AP2-0 ?
//    write(0x03, 0x0010);  // BT=000b, DC=000b, AP=100b, SLP=0, STB=0

    // Set SAP2-0  
    write(0x09, 0x0004);  // SAP="100", (DC1x="000", DK="0")

    // GON = "0", DTE = "0", D1-0 = "01"
    write(0x07, 0x0005);  // GON=0 DTE=0, REV=1, D=01b
    TOSH_uwait(20000);  // wait 40ms (2 frames)
    TOSH_uwait(20000);

    // GON = "1", DTE = "0", D1-0 = "01"
    write(0x07, 0x0025);  // GON=1 DTE=0, REV=1, D=01b
    TOSH_uwait(20000);  // wait 40ms (2 frames)
    TOSH_uwait(20000);

    // GON = "1", DTE = "0", D1-0 = "11"
    write(0x07, 0x0027);  // GON=1 DTE=0, REV=1, D=11b
    TOSH_uwait(20000);  // wait 40ms (2 frames)
    TOSH_uwait(20000);

    // GON = "1", DTE = "1", D1-0 = "11"
    write(0x07, 0x0037);  // GON=1 DTE=1, REV=1, D=11b
    TOSH_uwait(20000);  // wait 40ms (2 frames)
    TOSH_uwait(20000);

    lcdState = LCD_DISPLAY_ON;
    return SUCCESS;
}

//-------------------------------------------------------
//      Turn off the display
//-------------------------------------------------------
command result_t GuiDevice.displayOff(){

    // Display off (page 126, HX8310-A Datasheet)
    lcdState = LCD_BUSY;

    // GON = "1", DTE = "1", D1-0 = "10"
    write(0x07, 0x0036);  // 
    TOSH_uwait(20000);  // wait 40ms (2 frames)
    TOSH_uwait(20000);

    // GON = "1", DTE = "0", D1-0 = "10"
    write(0x07, 0x0026);  // 
    TOSH_uwait(20000);  // wait 40ms (2 frames)
    TOSH_uwait(20000);

    // GON = "0", DTE = "0", D1-0 = "00"
    write(0x07, 0x0004);  // 

    TOSH_uwait(20000);

    lcdState = LCD_DISPLAY_OFF;
    return SUCCESS;
}


//-------------------------------------------------------
//      Power on the device
//-------------------------------------------------------
command result_t GuiDevice.powerOn(){
    lcdState = LCD_BUSY;

    // Make sure the power is on
    TOSH_SET_LDVCC_PIN();
    TOSH_SET_LDVCI_PIN();

    lcdBkLtOn = 1;

    // Reset and init
    TOSH_uwait(10000);   // optional wait
    call GuiDevice.reset();
    TOSH_uwait(10000);
    powerSetup();
    TOSH_uwait(20000);
    TOSH_uwait(20000);
    chipSetup();

    write(0x21, 0x0000);  // 

//...
// RAM Write
//    write(0x22, 0x0000);  
//    TOSH_uwait(10000);
//...

    call GuiDevice.displayOn();

    lcdState = LCD_ON;
//    signal GuiDevice.initDone();
    return SUCCESS;
}

//-------------------------------------------------------
//-------------------------------------------------------
command result_t GuiDevice.powerOff()
{
    lcdState = LCD_BUSY;

    // PowerOff Sequence (page 128)
    // DTE="1", D1-0 = "11"
    // GON = "1"

    // Set EQ = "0"
    write(0x0b, 0x0000);  // EQ=0

    // insert DisplayOff sequence
    call GuiDevice.displayOff();

    // (Power supply halt bits)
    // SAP2-0 = "000"
    write(0x09, 0x0000);  // 
    // AP2-0 = "000"
    write(0x03, 0x0000);  // 
    // PON = "0"
    write(0x0d, 0x0004);  // PON=0, VRH=04h
    // VCOMG = "0"
    write(0x0e, 0x141e);  // VCOMG=0, VDV=14h, VCM=1Eh

    // Turn of power voltages
    TOSH_CLR_LDVCI_PIN();
    TOSH_CLR_LDVCC_PIN();

    lcdState = LCD_OFF;
    return SUCCESS;
}

//-------------------------------------------------------
//-------------------------------------------------------
command result_t GuiDevice.sleep(){
    call GuiDevice.powerOff();
    lcdState = LCD_BUSY;
    //read first?
    write(0x03, 0x0002);  // SLP=1
    write(0x0a, 0x0100);  // 
    TOSH_uwait(20000);

    lcdState = LCD_SLEEP;
    return SUCCESS;
}

//-------------------------------------------------------
//-------------------------------------------------------
command result_t GuiDevice.sleepWakeUp(){
    //read first?
    lcdState = LCD_BUSY;
    write(0x03, 0x2010);  // SLP=0
    write(0x0a, 0x0100);  // serial tx to chip2
    TOSH_uwait(20000);
    powerSetup();
    call GuiDevice.displayOn();

    lcdState = LCD_ON;
    return SUCCESS;
}

//-------------------------------------------------------
//-------------------------------------------------------
command result_t GuiDevice.standBy(){
    call GuiDevice.powerOff();
    //read first?
    lcdState = LCD_BUSY;
    write(0x03, 0x0001);  // STB=1

    lcdState = LCD_STANDBY;
    return SUCCESS;
}

//-------------------------------------------------------
//-------------------------------------------------------
command result_t GuiDevice.standByWakeUp(){
    write(0x00, 0x01);  // Start oscillator
    TOSH_uwait(20000);
    //read first?
    write(0x03, 0x2010);  // STB=0
    powerSetup();
    call GuiDevice.displayOn();

    lcdState = LCD_ON;
    return SUCCESS;
}


//-------------------------------------------------------
// Event when Lcd-init done
//-------------------------------------------------------
// default event result_t GuiDevice.initDone(){
//     return SUCCESS;
// }


//===============================================================
//      Video RAM access
//===============================================================
//-------------------------------------------------------
//      Set the current XY coords for the next output
//-------------------------------------------------------
command result_t GuiDevice.setXY(uint8_t x, uint8_t y)
{
#ifdef GUI_PORTRAIT
  // Portrait
  gScrAddr = ( x + (y * cScrWidthAddr) );
#else
  // Landscape
  gScrAddr = ( y + (x * cScrWidthAddr) );
#endif
  
  // TODO: check for out of bounds
  write(0x21, (gScrAddr) );  
  
  return SUCCESS;
}

//-------------------------------------------------------
//      Get the current XY coords
//-------------------------------------------------------
command result_t GuiDevice.getXY(uint8_t *x, uint8_t *y)
{
  // TODO: optimization possible

#ifdef GUI_PORTRAIT
  //Portrait
  *y = gScrAddr / cScrWidthAddr;
  *x = gScrAddr - (*y * cScrWidthAddr);

#else
  //Landscape
  *x = gScrAddr / cScrWidthAddr;
  *y = gScrAddr - (*x * cScrWidthAddr);
#endif

  return SUCCESS;
}

//-------------------------------------------------------
//      Get the dimensions of the screen
//-------------------------------------------------------
command result_t GuiDevice.getWH(uint8_t *width, uint8_t *height)
{
#ifdef GUI_PORTRAIT
    //Portrait
    *width  = cScrWidthPix;
    *height = cScrHeightPix;

#else
    //Landscape
    *width  = cScrHeightPix;
    *height = cScrWidthPix;
#endif

    return SUCCESS;
}

//-------------------------------------------------------
//      Set VRAM address for the next output
//-------------------------------------------------------
command result_t GuiDevice.setAddr(uint16_t addr)
{
    gScrAddr = addr;
    // TODO: check for out of bounds
    return SUCCESS;
}

//-------------------------------------------------------
//      Get current VRAM output address 
//-------------------------------------------------------
command result_t GuiDevice.getAddr(uint16_t *addr)
{
    *addr = gScrAddr;
    return SUCCESS;
}


//-------------------------------------------------------
// Fill LCD screen buffer: from current position for N pixels
//-------------------------------------------------------
command result_t GuiDevice.clear(GuiColor_t color)
{
    uint16_t k;
    if( lcdState != LCD_ON ){
        return FAIL;
    }
    
    // flush vram to the whole screen
    gScrAddr = 0;
    write(0x21, 0);  // set start at top

    for( k = 0; k < cScrHeightPix; k++ ){
        fillGram(color, cScrWidthPix);
    }
    return SUCCESS;
}


//-------------------------------------------------------
//      Fill N pixels on the screen
//-------------------------------------------------------
command result_t GuiDevice.fill(GuiColor_t color, uint8_t numPixels)
{
    fillGram(color, numPixels);
    return SUCCESS;
}

//-------------------------------------------------------
//      Copy N pixels from the buffer to the screen
//-------------------------------------------------------
command result_t GuiDevice.copy(GuiColor_t *source, uint8_t numPixels)
{
    writeGram(source, numPixels);
    return SUCCESS;
}


//===============================================================
//      Devices external to the display
//===============================================================

//-------------------------------------------------------
//      Turn on the Backlight
//-------------------------------------------------------
command result_t GuiDevice.backlightOn()
{
    TOSH_SET_LDBKLT_PIN();
    lcdBkLtOn = 1;
    return SUCCESS;
}

//-------------------------------------------------------
//      Turn off the Backlight
//-------------------------------------------------------
command result_t GuiDevice.backlightOff()
{
    TOSH_CLR_LDBKLT_PIN();
    lcdBkLtOn = 0;
    return SUCCESS;
}

//===============================================================
//      Keyboard Support and Interrupts
//===============================================================


//------------------------------------------------------------
//      Init button hardware
//------------------------------------------------------------
command result_t GuiDeviceInput.init()
{
    //............ Init buttons and interrupts
    // Buttons

    atomic {
      call ButtonAInt.disable();
      call ButtonAPin.makeInput();
      call ButtonAPin.selectIOFunc();
      call ButtonAInt.edge(TRUE);
      call ButtonAInt.clear();
      call ButtonAInt.enable();

      call ButtonBInt.disable();
      call ButtonBPin.makeInput();
      call ButtonBPin.selectIOFunc();
      call ButtonBInt.edge(TRUE);
      call ButtonBInt.clear();
      call ButtonBInt.enable();
    }

#ifdef SeeMoteVersion1a
    call GuiDeviceInput.enableButtonEvents(FALSE);
#else

    // Button-A
#endif

    return SUCCESS;
}

//------------------------------------------------------------
//      Poll button state.
//      if parameter == 0 then 
//              return the state of all buttons
//      else test for the mask 
//              and return TRUE if masked button(s) are down
//------------------------------------------------------------
command uint8_t GuiDeviceInput.isButtonDown(uint8_t buttonMask)
{
    if( buttonMask==0 ){
        // Request for the current button status mask
        if( 0==TOSH_READ_BTN_A_PIN() ) 
            buttonMask |= (GUI_BTN_LEFT | GUI_BTN_UP | GUI_BTN_SELECT);
        if( 0==TOSH_READ_BTN_B_PIN() ) 
            buttonMask |= (GUI_BTN_RIGHT | GUI_BTN_DOWN | GUI_BTN_SELECT);
        return buttonMask;
    }

    // Button A works as LEFT and UP and SELECT
    if( buttonMask & (GUI_BTN_LEFT | GUI_BTN_UP | GUI_BTN_SELECT) ){
        if( TOSH_READ_BTN_A_PIN() ) return FALSE;
        else return TRUE;
    }
    // Button B works as RIGHT and DOWN and SELECT
    if( buttonMask & (GUI_BTN_RIGHT | GUI_BTN_DOWN | GUI_BTN_SELECT) ){
        if( TOSH_READ_BTN_B_PIN() ) return FALSE;
        else return TRUE;
    }
    return FALSE;
}

//------------------------------------------------------------
// Clear any button interrupts or buffered events
//------------------------------------------------------------
command void GuiDeviceInput.clearButtonHistory()
{
  // clear interrupt history
  atomic {
    call ButtonAInt.clear();
    call ButtonBInt.clear();
  }
}

//------------------------------------------------------------
// Enable/disable button events
//------------------------------------------------------------
command void GuiDeviceInput.enableButtonEvents(uint8_t flEnable)
{
    if( flEnable ){
        // Enable keyboard interrupts
      atomic {
        call ButtonAInt.clear();
        call ButtonAInt.enable();
        call ButtonBInt.clear();
        call ButtonBInt.enable();
      }
    }
    else {
      atomic {
        call ButtonAInt.disable();
        call ButtonBInt.disable();
      }
    }
}



//------------------------------------------------------------
//      Tasks for delayed button down report
//------------------------------------------------------------
task void btnA(){
    signal GuiDeviceInput.buttonDown( 
        GUI_BTN_RIGHT | GUI_BTN_UP | GUI_BTN_SELECT );
}
task void btnB(){
    signal GuiDeviceInput.buttonDown( 
        GUI_BTN_LEFT | GUI_BTN_DOWN | GUI_BTN_SELECT );
}


//------------------------------------------------------------
// Interrupt on A button down
//------------------------------------------------------------
async event void ButtonAInt.fired()
{
  atomic {
    post btnA();
    call ButtonAInt.disable();
    call ButtonAInt.clear();
    call ButtonAInt.enable();
  }
}

//------------------------------------------------------------
// Interrupt on B button down
//------------------------------------------------------------
async event void ButtonBInt.fired()
{
  atomic {
    post btnB();
    call ButtonBInt.disable();
    call ButtonBInt.clear();
    call ButtonBInt.enable();
  }
}

//------------------------------------------------------------
//------------------------------------------------------------
default event result_t GuiDeviceInput.buttonDown(uint8_t buttonMask)
{
    return SUCCESS;
}


//===============================================================
//=== END of Implementation =====================================
//===============================================================
}
