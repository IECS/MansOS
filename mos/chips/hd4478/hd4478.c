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

#include "hd4478.h"

// -------------------------------
// internal function declaration
// -------------------------------
void hd4478Write(bool isCmd, bool isByte, bool cbf, uint8_t input);
uint8_t hd4478Read(bool waitBefore, bool rs);

#define hd4478WriteCmd(cmd) hd4478Write(1, 1, 1, cmd)

// -----------------------
// shortcuts
// -----------------------
#define HD4478_RS_OUT() pinAsOutput(HD4478_RS_PORT, HD4478_RS_PIN)
#define HD4478_RW_OUT() pinAsOutput(HD4478_RW_PORT, HD4478_RW_PIN)
#define HD4478_EN_OUT() pinAsOutput(HD4478_EN_PORT, HD4478_EN_PIN)
#define HD4478_DATA4_OUT() pinAsOutput(HD4478_DATA4_PORT, HD4478_DATA4_PIN)
#define HD4478_DATA5_OUT() pinAsOutput(HD4478_DATA5_PORT, HD4478_DATA5_PIN)
#define HD4478_DATA6_OUT() pinAsOutput(HD4478_DATA6_PORT, HD4478_DATA6_PIN)
#define HD4478_DATA7_OUT() pinAsOutput(HD4478_DATA7_PORT, HD4478_DATA7_PIN)
#define HD4478_DATA4_IN() pinAsInput(HD4478_DATA4_PORT, HD4478_DATA4_PIN)
#define HD4478_DATA5_IN() pinAsInput(HD4478_DATA5_PORT, HD4478_DATA5_PIN)
#define HD4478_DATA6_IN() pinAsInput(HD4478_DATA6_PORT, HD4478_DATA6_PIN)
#define HD4478_DATA7_IN() pinAsInput(HD4478_DATA7_PORT, HD4478_DATA7_PIN)
#define HD4478_DATA4_HI() pinSet(HD4478_DATA4_PORT, HD4478_DATA4_PIN)
#define HD4478_DATA4_LO() pinClear(HD4478_DATA4_PORT, HD4478_DATA4_PIN)
#define HD4478_DATA5_HI() pinSet(HD4478_DATA5_PORT, HD4478_DATA5_PIN)
#define HD4478_DATA5_LO() pinClear(HD4478_DATA5_PORT, HD4478_DATA5_PIN)
#define HD4478_DATA6_HI() pinSet(HD4478_DATA6_PORT, HD4478_DATA6_PIN)
#define HD4478_DATA6_LO() pinClear(HD4478_DATA6_PORT, HD4478_DATA6_PIN)
#define HD4478_DATA7_HI() pinSet(HD4478_DATA7_PORT, HD4478_DATA7_PIN)
#define HD4478_DATA7_LO() pinClear(HD4478_DATA7_PORT, HD4478_DATA7_PIN)
#define HD4478_EN_HI() pinSet(HD4478_EN_PORT, HD4478_EN_PIN)
#define HD4478_EN_LO() pinClear(HD4478_EN_PORT, HD4478_EN_PIN)

// ****************************************************************************************
// Note: if you use long wires for HD47880 connection or experience any other unexplainable
// errors on the screen - use longer delay here (3us instead of 1us). Datasheet says it
// should be ok to use 1us here.
// ****************************************************************************************
#define HD4478_EN_TOGGLE() \
    HD4478_EN_HI(); \
    udelay(3); \
    HD4478_EN_LO(); \
    udelay(3);
#define HD4478_RS_HI() pinSet(HD4478_RS_PORT, HD4478_RS_PIN)
#define HD4478_RS_LO() pinClear(HD4478_RS_PORT, HD4478_RS_PIN)
#define HD4478_RW_HI() pinSet(HD4478_RW_PORT, HD4478_RW_PIN)
#define HD4478_RW_LO() pinClear(HD4478_RW_PORT, HD4478_RW_PIN)
#define HD4478_DATA4_GET() pinRead(HD4478_DATA4_PORT, HD4478_DATA4_PIN)
#define HD4478_DATA5_GET() pinRead(HD4478_DATA5_PORT, HD4478_DATA5_PIN)
#define HD4478_DATA6_GET() pinRead(HD4478_DATA6_PORT, HD4478_DATA6_PIN)
#define HD4478_DATA7_GET() pinRead(HD4478_DATA7_PORT, HD4478_DATA7_PIN)
#define HD4478_DATA4_SET(b) pinWrite(HD4478_DATA4_PORT, HD4478_DATA4_PIN, b)
#define HD4478_DATA5_SET(b) pinWrite(HD4478_DATA5_PORT, HD4478_DATA5_PIN, b)
#define HD4478_DATA6_SET(b) pinWrite(HD4478_DATA6_PORT, HD4478_DATA6_PIN, b)
#define HD4478_DATA7_SET(b) pinWrite(HD4478_DATA7_PORT, HD4478_DATA7_PIN, b)

#define HD4478_EN_SET(b) pinWrite(HD4478_EN_PORT, HD4478_EN_PIN, b)
#define HD4478_RW_SET(b) pinWrite(HD4478_RW_PORT, HD4478_RW_PIN, b)
#define HD4478_RS_SET(b) pinWrite(HD4478_RS_PORT, HD4478_RS_PIN, b)

#define HD4478_RS_DATA    1
#define HD4478_RS_COMMAND 0


// ---------------------------------
// public function implementation
// ---------------------------------


/* Initialize HD44780 LCD display (reset, set 4-bit mode,
 * initialize default values).
 * Use simple busy wait instead of busy-flag checking, as it does not work at
 * this stage
 */
void hd4478Init()
{
    HD4478_DATA4_LO();
    HD4478_DATA5_LO();
    HD4478_DATA6_LO();
    HD4478_DATA7_LO();
    HD4478_RS_LO();
    HD4478_RW_LO();
    HD4478_EN_LO();

    HD4478_DATA4_OUT();
    HD4478_DATA5_OUT();
    HD4478_DATA6_OUT();
    HD4478_DATA7_OUT();
    HD4478_RS_OUT();
    HD4478_RW_OUT();
    HD4478_EN_OUT();

    mdelay(500);

    hd4478Write(1, 0, 0, 0x03);  // interface 8-bit long
    mdelay(10);                  // wait for more than 4.1ms
    hd4478Write(1, 0, 0, 0x03);
    udelay(300);                 // wait for more than 100us
    hd4478Write(1, 0, 0, 0x03);
    udelay(100);
    hd4478Write(1, 0, 0, 0x02);   // func set: 4-bit interface

    // function set: 4-bit, 2-line (default), 5x7
    hd4478Write(1, 1, 0, HD4478_FUNCTION | HD4478_FN_DEFAULT);
    hd4478Write(1, 1, 0, HD4478_DISP_OFF);                    // display OFF
    hd4478Write(1, 1, 0, HD4478_CLR);                         // clear display

    mdelay(3);

    // entry mode: increment,
    hd4478Write(1, 1, 0, HD4478_ENTRY_MODE | HD4478_ENTRY_INC);
    // the screen does not move, writing like this->>>
    hd4478WriteCmd(HD4478_ON | HD4478_ON_DISPLAY);
    hd4478Clear();                      // clear LCD and cursor home (optional)
}

/*
 * Print one character on the LCD
 * @param data - the character to be displayed
 */
void hd4478PrintByte(uint8_t data) {
    hd4478Write(0, 1, 1, data);
}

/*
 * Print a null-terminated string on the LCD
 * @param str - the string to be displayed
 */
void hd4478Print(char *str)
{
    while (*str != '\0')
    {
        hd4478PrintByte(*str);
        str++;
    }
}

// just a wrapper
void hd4478SendCmd(uint8_t cmd) {
    hd4478WriteCmd(cmd);
}

void hd4478GoToXY(uint8_t x, uint8_t y)
{
    if (y == 1) x += 0x40; // each line adds 0x40 to address
    hd4478WriteCmd(HD4478_DDRAM | x);
    //TODO: (else if) for 4 line displays
}

// Clear HD44780 display (AC->0) and return home
void hd4478Clear(void)
{
    hd4478WriteCmd(HD4478_CLR);  // clear LCD
    hd4478WriteCmd(HD4478_HOME); // return home
}


// ---------------------------------
// internal function implementation
// ---------------------------------

uint8_t hd4478CheckBusyFlag(void)
{
    unsigned char bf;
    HD4478_RS_LO();
    HD4478_RW_HI();       // RW = read (1)
    _NOP();
    HD4478_EN_HI();

    HD4478_DATA7_IN();
    udelay(1);
    bf = HD4478_DATA7_GET();
    HD4478_EN_TOGGLE();

    HD4478_EN_TOGGLE();     // for second nibble
    udelay(1);
    HD4478_DATA7_OUT();
    return bf;
}

/**
 * Loop while LCD is busy
 */
static void hd4478WaitWhileBusy() {
    /* wait until busy flag is cleared */
    while (hd4478CheckBusyFlag()) {}

    HD4478_RW_LO();       // RW = write (0)

    /* the address counter is updated 4us after the busy flag is cleared */
    udelay(4);
}


// In init phase busy flag cannot be checked, simple wait must be used
// In other situations it is up to the user
// cbf - whether to check busy flag (= 1), or use simple busy wait (= 0)
// isByte - when false, only 4 youngest bits (nibble) used
void hd4478Write(bool isCmd, bool isByte, bool cbf, uint8_t input)
{
    HD4478_RS_SET(isCmd);
    HD4478_RW_LO();
    _NOP();
    HD4478_EN_HI();

    if (isByte) {
        // use all 8 bits of input byte
        HD4478_DATA4_SET(input & 0x10);
        HD4478_DATA5_SET(input & 0x20);
        HD4478_DATA6_SET(input & 0x40);
        HD4478_DATA7_SET(input & 0x80);

        HD4478_EN_TOGGLE();
        udelay(10); // 10us needed in init. 1us can be used later. Optimize?
        HD4478_EN_HI();
    }

    HD4478_DATA4_SET(input & 0x01);
    HD4478_DATA5_SET(input & 0x02);
    HD4478_DATA6_SET(input & 0x04);
    HD4478_DATA7_SET(input & 0x08);

    _NOP();
    HD4478_EN_TOGGLE();
    if (cbf && HD4478_CHECK_BUSY_FLAG) {
        // wait for busy flag only when HD4478_CHECK_BUSY_FLAG const set
        hd4478WaitWhileBusy();
    } else {
        udelay(120);
    }
}


// read DDRAM or CGRAM data; has some problems (read the HD44780 datasheet)
// waitBefore - when true, check busy flag before actual reading
// rs - when 1, read ddram, when 0 - address counter
uint8_t hd4478Read(bool waitBefore, bool rs)
{
    uint8_t ddram = 0;

    if (waitBefore) hd4478WaitWhileBusy();

    HD4478_RS_SET(rs);
    HD4478_RW_HI();       // RW = read (1)
    udelay(1);
    HD4478_EN_HI();

    HD4478_DATA4_IN();
    HD4478_DATA5_IN();
    HD4478_DATA6_IN();
    HD4478_DATA7_IN();
    //udelay(1);
    //HD4478_EN_HI();
    udelay(1);

    /* read high nibble first */
    if (HD4478_DATA4_GET()) ddram |= 0x10;
    if (HD4478_DATA5_GET()) ddram |= 0x20;
    if (HD4478_DATA6_GET()) ddram |= 0x40;
    if (HD4478_DATA7_GET()) ddram |= 0x80;

    HD4478_EN_TOGGLE();

    HD4478_EN_HI();
    udelay(1);

    /* read low nibble */
    if (HD4478_DATA4_GET()) ddram |= 0x01;
    if (HD4478_DATA5_GET()) ddram |= 0x02;
    if (HD4478_DATA6_GET()) ddram |= 0x04;
    if (HD4478_DATA7_GET()) ddram |= 0x08;

    HD4478_EN_TOGGLE();
    udelay(1);

    HD4478_RW_LO();
    HD4478_RS_LO();

    // change pins back to output mode
    HD4478_DATA4_OUT();
    HD4478_DATA5_OUT();
    HD4478_DATA6_OUT();
    HD4478_DATA7_OUT();

    udelay(2);

    hd4478WaitWhileBusy();
    return ddram;
}

// Read Address Counter (AC)
#define hd4478ReadAC() hd4478Read(1, 0)

// Writes user-defined character (bitmap) to CGRAM (max 8 user def. chars)
// index - index of char in user char array
void hd4478LoadChar(uint8_t *ptr, uint8_t index)
{
    uint8_t ac = hd4478ReadAC();
    // index in (0..7), 3 bits, << 3 == bits 6 to 3, CGRAM = 0x40 = bit 7
    // so CGRAM | index is ok
    hd4478WriteCmd(HD4478_CGRAM | (index << 3));
    uint8_t row;
    for (row = 0; row < 8; row++)
    {
        hd4478Write(0, 1, 1, ptr[row]); // write data, not cmd
    }
    hd4478WriteCmd(HD4478_DDRAM | ac);
    hd4478WaitWhileBusy();
}
