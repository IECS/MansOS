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

#include "isl29003.h"

/* ISL29003 soft I2C support */

//------------------------------------------------------------------------------
// Writes a byte to I2C and checks acknowledge
// returns 0 on success
//------------------------------------------------------------------------------
i2cError_t IslI2cWriteByte(uint8_t txByte)
{
    uint8_t mask; 
    i2cError_t err=0;
  
    for (mask=0x80; mask>0; mask>>=1)   //shift bit for masking (8 times)
    { 
        if ((mask & txByte) == 0) {
            ISL_I2C_SDA_LO();               //write a bit to SDA-Line
        }
        else {
            ISL_I2C_SDA_HI();
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
uint8_t IslI2cReadByte(i2cAck_t ack)
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
        ISL_I2C_SDA_LO();                   //send acknowledge if necessary
    } else {
        ISL_I2C_SDA_HI();
    } 
    wait_1us;      
    I2C_SCL_HI();                       //clk #9 for ack
    wait_5us;      
    I2C_SCL_LO();
    I2C_SDA_OUT();                      //unrelease SDA-line
    wait_5us;
    //wait_us(20);                      //delay to see the package on scope
    return rxByte;                      //return error code
}
/* End of ISL29003 soft I2C support */

// Enable ISL29003
bool islOn(){
    uint8_t val;
    // Get current register value
    if (readIslRegister(ISL_COMMAND_REGISTER, &val)){
        return false;    // fail!
        }
    // Append on value
    val &= ~ISL_ENABLE_BIT; // binary 0111 1111
    val += ISL_ENABLE_BIT;
    // Update control register
    if (writeIslRegister(ISL_COMMAND_REGISTER, val)){
        return false;    // fail!
    }
    return true;
} 

// Disable ISL29003
bool islOff(){
    uint8_t val;
    // Get current register value
    if (readIslRegister(ISL_COMMAND_REGISTER, &val)){
        return false;    // fail!
    }
    // Append on value
    val &= ~ISL_ENABLE_BIT; // binary 0111 1111
    // Update control register
    if (writeIslRegister(ISL_COMMAND_REGISTER, val)){
        return false;    // fail!
    }
    return true;
} 

// Put ISL29003 to normal mode
bool islWake(){
    uint8_t val;
    // Get current register value
    if (readIslRegister(ISL_COMMAND_REGISTER, &val)){
        return false;    // fail!
    }
    // Append on value
    val &= ~ISL_SLEEP_BIT; // binary 1011 1111
    // Update control register
    if (writeIslRegister(ISL_COMMAND_REGISTER, val)){
        return false;    // fail!
    }
    return true;
} 

// Put ISL29003 to sleep mode
bool islSleep(){
    uint8_t val;
    // Get current register value
    if (readIslRegister(ISL_COMMAND_REGISTER, &val)){
        return false;    // fail!
    }
    // Append on value
    val &= ~ISL_SLEEP_BIT; // binary 1011 1111
    val += ISL_SLEEP_BIT;
    // Update control register
    if (writeIslRegister(ISL_COMMAND_REGISTER, val)){
        return false;    // fail!
    }
    return true;
}

//Check if ISL29003 is ON
bool isIslOn(){
        uint8_t val;
    // Get current register value
    if (readIslRegister(ISL_COMMAND_REGISTER, &val)){
        return false;    // fail!
    }
    return val & ISL_ENABLE_BIT;
}

//Check if ISL29003 is awake
bool isIslWake(){
    uint8_t val;
    // Get current register value
    if (readIslRegister(ISL_COMMAND_REGISTER, &val)){
        return false;    // fail!
    }
    return !(val & ISL_SLEEP_BIT);
}

// Initialize ISL29003, configure and turn it off
bool islInit(){
    // init SDA and SCK pins (defined in config file)
    i2cInit();
    // Clear registers, so no previos problems occur
    writeIslRegister(ISL_COMMAND_REGISTER,0);
    writeIslRegister(ISL_CONTROL_REGISTER,0);
    // configure ISL29003 turn ON:
    // mode                    -> Use ~(DIODE1 - DIODE2)
    // clock_cycles            -> 2^16 = 65,536
    // range_gain            -> 62,272
    // integration_cycles    -> 16
    IslConfigure_t conf = {
        .mode = USE_BOTH_DIODES,
        .clock_cycles = CLOCK_CYCLES_16,
        .range_gain = RANGE_GAIN_62,
        .integration_cycles = INTEGRATION_CYCLES_16,
    };
    return (configureIsl(conf) && islOn() && islWake());
}

// Write ISL29003 register
bool writeIslRegister(uint8_t reg, uint8_t val){
    bool err = false;
    Handle_t intHandle;
    ATOMIC_START(intHandle);
    i2cStart();
    err |= IslI2cWriteByte(ISL_WRITE_FLAG);
    err |= IslI2cWriteByte(reg);
    err |= IslI2cWriteByte(val);
    i2cStop();
    ATOMIC_END(intHandle);
    return err;
}

// Read ISL29003 register
bool readIslRegister(uint8_t reg, uint8_t *val){
    bool err = false;
    Handle_t intHandle;
    ATOMIC_START(intHandle);
    i2cStart();
    err |= IslI2cWriteByte(ISL_WRITE_FLAG);
    err |= IslI2cWriteByte(reg);
    i2cStop();
    i2cStart();
    err |= IslI2cWriteByte(ISL_READ_FLAG); 
    *val = IslI2cReadByte(I2C_ACK);
    ISL_I2C_SDA_LO();
    I2C_SCL_LO();
    udelay(4);
    i2cStop();
    ATOMIC_END(intHandle);
    return err;
}

// Configure ISL29003
bool configureIsl(IslConfigure_t newConf){
    uint8_t val;
    // Get current register value
    if (readIslRegister(ISL_COMMAND_REGISTER, &val)){
        return false;    // fail!
    }
    // Append mode
    if (newConf.mode < 4){
        val &= 0xF3; // binary 1111 0011
        newConf.mode <<= 2;
        val += newConf.mode;
    }
    // Append clock_cycles
    if (newConf.clock_cycles < 4){
        val &= 0xFC; // binary 1111 1100
        val += newConf.clock_cycles;
    }
    // Update command register
    if (writeIslRegister(ISL_COMMAND_REGISTER, val)){
        return false;    // fail!
    }
    // Get current register value
    if (readIslRegister(ISL_CONTROL_REGISTER, &val)){
        return false;    // fail!
    }
    // Append range_gain
    if (newConf.range_gain < 4){
        val &= 0xF3; // binary 1111 0011
        newConf.range_gain <<= 2;
        val += newConf.range_gain;
    }
    // Append range_gain
    if (newConf.integration_cycles < 4){
        val &= 0xFC; // binary 1111 1100
        val += newConf.integration_cycles;
    }
    // Update control register
    if (writeIslRegister(ISL_CONTROL_REGISTER, val)){
        return false;    // fail!
    }
    return true;
}

// Check if ISL29003 have data to give
bool islInterupt(bool clearIfHave){
    uint8_t val;
    /* Get current state. */
    if (readIslRegister(ISL_CONTROL_REGISTER,&val)){
        return true;               //fail! returning true for nonblock in case of isl failure
    }
    /* Check for interrupt. */
    if (val & ISL_INTERUPT_BIT){
        if (clearIfHave){
            if (clearIslInterupt()){
                return true;           //fail! returning true for nonblock in case of isl failure
            }
        }
        return true;
    }
    return false;
}

// Clear ISL29003 interupt bit
bool clearIslInterupt(){
    bool err = false;
    Handle_t intHandle;
    ATOMIC_START(intHandle);
    i2cStart();
    err |= IslI2cWriteByte(ISL_WRITE_FLAG);
    err |= IslI2cWriteByte(ISL_CLEAR_INTERUPT_REGISTER);
    i2cStop();
    ATOMIC_END(intHandle);
    return err;
}

// Read ISL29003 sensor data
bool islRead(uint16_t *data, bool checkInterupt){
    uint8_t val;
    /* Check ISL29003 current state. */
    bool on=isIslOn(), wake=isIslWake();
    /* Enable device if necessary. */
    if (!on){
        islOn();
    }
    if (!wake){
        islWake();
    }
    if (checkInterupt){
        while (!islInterupt(true));
    }
    /* Reads register 5 - MSB */
    if (readIslRegister(0x05, &val)){
        return false;    //fail!
    }
    *data = val<<8;
    /* Reads register 4 - LSB*/
    if(readIslRegister(0x04, &val)){
        return false;    //fail!
    }
    *data+=val;
    /* Hide our tracks... */
    if (!on){
        islOff();
    }
    if (!wake){
        islSleep();
    }
    return true;
}

uint16_t islReadSimple(void)
{
    uint16_t result;
    if (!islRead(&result, true)) {
        result = 0xffff;
    }
    return result;
}
