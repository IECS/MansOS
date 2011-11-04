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

#ifndef MANSOS_HD4478_H
#define MANSOS_HD4478_H

//-------------------------------------------
// Driver for HD4478 LCD controller
// http://www.sparkfun.com/datasheets/LCD/HD44780.pdf
// Code partially copied from HD4478 LCD library by Peter Fleury
// <pfleury@gmx.ch>  http://jump.to/fleury
//-------------------------------------------

#include "gpio.h"
#include "udelay.h"

// if this macro-flag is 1, wait until operation is finished
// use simple delay (max us according to datasheet) otherwise
#ifndef HD4478_CHECK_BUSY_FLAG
  #define HD4478_CHECK_BUSY_FLAG 1
#endif

// Using 4-bit mode
// PINS: DATA4-DATA7, RS, E, RW

// Warning: RW pin is also used to read status, do not hardwire it to ground!

// either use default pinout, or pins must be defined in the config file
#ifdef USE_DEFAULT_HD4478_PINOUT
  #include "hd4478_hal.h"
#endif

#if !defined(HD4478_RS_PORT) || !defined(HD4478_RS_PIN)
#error HD4478_RS_PORT and HD4478_RS_PIN must be defined in config file
#endif
#if !defined(HD4478_RW_PORT) || !defined(HD4478_RW_PIN)
#error HD4478_RW_PORT and HD4478_RW_PIN must be defined in config file
#endif
#if !defined(HD4478_EN_PORT) || !defined(HD4478_EN_PIN)
#error HD4478_EN_PORT and HD4478_EN_PIN must be defined in config file
#endif
#if !defined(HD4478_DATA4_PORT) || !defined(HD4478_DATA4_PIN)
#error HD4478_DATA4_PORT and HD4478_DATA4_PIN must be defined in config file
#endif
#if !defined(HD4478_DATA5_PORT) || !defined(HD4478_DATA5_PIN)
#error HD4478_DATA5_PORT and HD4478_DATA5_PIN must be defined in config file
#endif
#if !defined(HD4478_DATA6_PORT) || !defined(HD4478_DATA6_PIN)
#error HD4478_DATA6_PORT and HD4478_DATA6_PIN must be defined in config file
#endif
#if !defined(HD4478_DATA7_PORT) || !defined(HD4478_DATA7_PIN)
#error HD4478_DATA7_PORT and HD4478_DATA7_PIN must be defined in config file
#endif

// -----------------------
// public functions
// -----------------------

/*
 * Initialize HD44780 LCD display: reset, set 4-bit mode
 */
void hd4478Init();

/*
 * Turn the LCD on
 */
#define hd4478On() hd4478SendCmd(HD4478_DISP_ON)

/*
 * Turn the LCD off
 */
#define hd4478Off() hd4478SendCmd(HD4478_DISP_ON)

/*
 * Print one character on the LCD
 * @param data - the character to be displayed
 */
void hd4478PrintByte(uint8_t data);

/*
 * Print a null-terminated string on the LCD
 * @param str - the string to be displayed
 */
void hd4478Print(char *str);

/*
 * Send command to the LCD
 */
void hd4478SendCmd(uint8_t cmd);


void hd4478GoToXY(uint8_t x, uint8_t y);  // move cursor
void hd4478Clear();        // clear HD44780 display (AC->0) and return home

// additional functions as macro shortcuts
#define hd4478MoveCursorLeft() hd4478SendCmd(HD4478_MOVE_CURSOR_LEFT)
#define hd4478MoveCursorRight() hd4478SendCmd(HD4478_MOVE_CURSOR_RIGHT)
#define hd4478MoveScreenLeft() hd4478SendCmd(HD4478_MOVE_SCREEN_LEFT)
#define hd4478MoveScreenRight() hd4478SendCmd(HD4478_MOVE_SCREEN_RIGHT)

// see modes below
#define hd4478SetDisplay(mode) hd4478SendCmd(HD4478_DISP_OFF | mode)
#define hd4478SetEntryMode(mode) hd4478SendCmd(HD4478_ENTRY_MODE | mode)

// -----------------------
// internal stuff
// -----------------------
// HD4478 pins must be defined in HAL level

// platform-independent constants
// Warning: this piece differs from original LCD library, where bit positions
// were defined instead of bit values

/* instruction register bit positions, see HD44780U data sheet */
#define HD4478_CLR              0x01      /* DB0: clear display                  */
#define HD4478_HOME             0x02      /* DB1: return to home position        */
#define HD4478_ENTRY_MODE       0x04      /* DB2: set entry mode                 */
#define HD4478_ENTRY_INC        0x02      /*   DB1: 1=increment, 0=decrement     */
#define HD4478_ENTRY_SHIFT      0x01      /*   DB2: 1=display shift on           */
#define HD4478_ON               0x08      /* DB3: turn lcd/cursor on             */
#define HD4478_ON_DISPLAY       0x04      /*   DB2: turn display on              */
#define HD4478_ON_CURSOR        0x02      /*   DB1: turn cursor on               */
#define HD4478_ON_BLINK         0x01      /*     DB0: blinking cursor ?          */
#define HD4478_MOVE             0x10      /* DB4: move cursor/display            */
#define HD4478_MOVE_DISP        0x08      /*   DB3: move display (0-> cursor) ?  */
#define HD4478_MOVE_RIGHT       0x04      /*   DB2: move right (0-> left) ?      */
#define HD4478_FUNCTION         0x20      /* DB5: function set                   */
#define HD4478_FN_8BIT          0x10      /*   DB4: set 8BIT mode (0->4BIT mode) */
#define HD4478_FN_2LINES        0x08      /*   DB3: two lines (0->one line)      */
#define HD4478_FN_10DOTS        0x04      /*   DB2: 5x10 font (0->5x7 font)      */
#define HD4478_CGRAM            0x40      /* DB6: set CG RAM address             */
#define HD4478_DDRAM            0x80      /* DB7: set DD RAM address             */
#define HD4478_BUSY             0x80      /* DB7: LCD is busy                    */

/* set entry mode: display shift on/off, dec/inc cursor move direction */
#define HD4478_ENTRY_DEC            0x04   /* display shift off, dec cursor move dir */
#define HD4478_ENTRY_DEC_SHIFT      0x05   /* display shift on,  dec cursor move dir */
#define HD4478_ENTRY_INC_           0x06   /* display shift off, inc cursor move dir */
#define HD4478_ENTRY_INC_SHIFT      0x07   /* display shift on,  inc cursor move dir */

/* display on/off, cursor on/off, blinking char at cursor position */
#define HD4478_DISP_OFF             0x08   /* display off                            */
#define HD4478_DISP_ON              0x0C   /* display on, cursor off                 */
#define HD4478_DISP_ON_BLINK        0x0D   /* display on, cursor off, blink char     */
#define HD4478_DISP_ON_CURSOR       0x0E   /* display on, cursor on                  */
#define HD4478_DISP_ON_CURSOR_BLINK 0x0F   /* display on, cursor on, blink char      */

/* move cursor/shift display */
#define HD4478_MOVE_CURSOR_LEFT     0x10   /* move cursor left  (decrement)          */
#define HD4478_MOVE_CURSOR_RIGHT    0x14   /* move cursor right (increment)          */
#define HD4478_MOVE_DISP_LEFT       0x18   /* shift display left                     */
#define HD4478_MOVE_DISP_RIGHT      0x1C   /* shift display right                    */

/* function set: set interface data length and number of display lines */
#define HD4478_FN_4BIT_1LINE  0x20   /* 4-bit interface, single line, 5x7 dots */
#define HD4478_FN_4BIT_2LINES 0x28   /* 4-bit interface, dual line,   5x7 dots */
#define HD4478_FN_8BIT_1LINE  0x30   /* 8-bit interface, single line, 5x7 dots */
#define HD4478_FN_8BIT_2LINES 0x38   /* 8-bit interface, dual line,   5x7 dots */

#define HD4478_MODE_DEFAULT     (HD4478_ENTRY_MODE | HD4478_ENTRY_INC)

#if HD4478_LINES == 1
// one line display
#define HD4478_FN_DEFAULT    HD4478_FN_4BIT_1LINE
#else
// two line display
#define HD4478_FN_DEFAULT    HD4478_FN_4BIT_2LINES
#endif

#endif // !MANSOS_HD4478_H

