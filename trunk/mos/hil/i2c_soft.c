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

//==============================================================================
// Software controlled I2C
//  Prior to use you must define SDA and SCL pins using your specific values 
// #define SDA_PORT 1
// #define SDA_PIN  1
// #define SCL_PORT 1
// #define SCL_PIN  2
//
// Note: enable the pullup resistors, or ensure the hardware has them.
// Note: timing may have to be adjusted for a different microcontroller
//==============================================================================

#include "i2c_soft.h"

//------------------------------------------------------------------------------
// Writes a byte to I2C and checks acknowledge
// returns 0 on success
//------------------------------------------------------------------------------
i2cError_t i2cWriteByte(uint8_t txByte)
{
    uint8_t mask; 
    i2cError_t err=0;
  
    for (mask=0x80; mask>0; mask>>=1)   //shift bit for masking (8 times)
    { 
        if ((mask & txByte) == 0) {
            I2C_SDA_LO();               //write a bit to SDA-Line
        }
        else {
            I2C_SDA_HI();
        }
        wait_1us;
        I2C_SCL_HI();
        wait_5us;
        I2C_SCL_LO();
        wait_1us;
    }
    I2C_SDA_IN();                       //release SDA-line
    I2C_SCL_HI();                       //clk #9 for ack
    wait_5us;
    if (I2C_SDA_GET() != 0) {
        err = I2C_ACK_ERROR;            //check ack from i2c slave
    }
    I2C_SCL_LO();
    wait_5us;
    //wait_20us;                        //delay to see the package on scope
    I2C_SDA_OUT();
    return err;                         //return error code
}

//------------------------------------------------------------------------------
// Reads a byte from I2C
// Returns the byte received
// note: timing (delay) may have to be changed for different microcontroller
//------------------------------------------------------------------------------
uint8_t i2cReadByte(i2cAck_t ack)
{
    uint8_t mask, rxByte=0;

    I2C_SDA_IN();                       //release SDA-line
    for (mask=0x80; mask>0; mask>>=1)   //shift bit for masking 
    { 
        I2C_SCL_HI();                   //start clock on SCL-line
        wait_4us;
        if (I2C_SDA_GET() != 0) {
            rxByte = (rxByte | mask);   //read bit
        }
        I2C_SCL_LO();
        wait_1us;                       //data hold time
    }
    if (ack) {
        I2C_SDA_LO();                   //send acknowledge if necessary
    } else {
        I2C_SDA_HI();
    } 
    wait_1us;      
    I2C_SCL_HI();                       //clk #9 for ack
    wait_5us;      
    I2C_SCL_LO();
    I2C_SDA_IN();                       //release SDA-line
    wait_5us;
    //wait_us(20);                      //delay to see the package on scope
    return rxByte;                      //return error code
}
