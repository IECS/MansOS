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


//==============================================================
//      Define pins  
//==============================================================

// The platform must define pins


// // Hardware interface for the LCD
// TOSH_ASSIGN_PIN(LD0, 5,0);
// TOSH_ASSIGN_PIN(LD1, 5,1);
// TOSH_ASSIGN_PIN(LD2, 5,2);
// TOSH_ASSIGN_PIN(LD3, 5,3);
// TOSH_ASSIGN_PIN(LD4, 5,4);
// TOSH_ASSIGN_PIN(LD5, 5,5);
// TOSH_ASSIGN_PIN(LD6, 5,6);
// TOSH_ASSIGN_PIN(LD7, 5,7);

// #define LDDATA P5OUT    //PORT5

// TOSH_ASSIGN_PIN(LDWR, 1,6);
// TOSH_ASSIGN_PIN(LDRD, 1,5);
// TOSH_ASSIGN_PIN(LDRS, 1,7);

// TOSH_ASSIGN_PIN(LDCS, 6,7);
// TOSH_ASSIGN_PIN(LDRST, 6,3);

// // Power control:
// // Backlight 
// TOSH_ASSIGN_PIN(LDBKLT, 6,6);
// TOSH_ASSIGN_PIN(LDVCC, 6,4);
// TOSH_ASSIGN_PIN(LDVCI, 6,5);


//==============================================================
// LCD parameters and data types
//==============================================================

// LCD panel dimensions
/*
#define LCD_SIZE_X      (128)
#define LCD_SIZE_Y      (160)
#define LCD_SIZE_X2     (LCD_SIZE_X + LCD_SIZE_X)

#define LCD_FONT_HEIGHT (8)

// Video RAM buffer (cache)
#define VRAM_BUFF_HEIGHT        (LCD_FONT_HEIGHT)
#define VRAM_BUFF_SIZE          (LCD_SIZE_X * VRAM_BUFF_HEIGHT)   // in pixels
#define VRAM_BUFF_SIZE2         (VRAM_BUFF_SIZE + VRAM_BUFF_SIZE) // in bytes
*/


typedef enum {
    LCD_FIRST, 
    LCD_BUSY, 
    LCD_RESET,
    LCD_OFF,
    LCD_ON, 
    LCD_SLEEP,
    LCD_STANDBY,
    LCD_DISPLAY_OFF,
    LCD_DISPLAY_ON,
    LCD_LAST_STATE
} LcdState_t;

