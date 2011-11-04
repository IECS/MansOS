#include "i2c_soft.h"
#include "stdmansos.h"

// Command to read ADS1114 register
#define ADS_READ_FLAG 0x91
// Command to write ADS1114 register
#define ADS_WRITE_FLAG 0x90

// Write ADS1114 register
bool writeAdsRegister(uint8_t reg, uint16_t val){
    uint8_t err = false;
    Handle_t intHandle;
    ATOMIC_START(intHandle);
    i2cStart();
    err |= i2cWriteByte(ADS_WRITE_FLAG);
    err |= i2cWriteByte(reg);
    err |= i2cWriteByte(val >> 8);
    err |= i2cWriteByte(val & 0xff);
    i2cStop();
    ATOMIC_END(intHandle);
    //PRINTF("Writed:%#x%x\n",val >> 8,val & 0xff);
    return err == 0;
}

// Read ADS1114 register
bool readAdsRegister(uint8_t reg, uint16_t *val){
    uint8_t err = false;
    *val = 0;
    Handle_t intHandle;
    ATOMIC_START(intHandle);
    i2cStart();
    err |= i2cWriteByte(ADS_WRITE_FLAG);
    err |= i2cWriteByte(reg);
    i2cStop();
    i2cStart();
    err |= i2cWriteByte(ADS_READ_FLAG);
    // *val = (i2cReadByte(I2C_ACK) << 8) | i2cReadByte(I2C_ACK) ;
    uint8_t hi = i2cReadByte(I2C_ACK);
    uint8_t lo = i2cReadByte(I2C_NO_ACK);
    *val = (hi << 8) | lo ;
    //I2C_SDA_LO();
    //I2C_SCL_LO();
    //udelay(4);
    i2cStop();
    ATOMIC_END(intHandle);
    //PRINTF("recieved %#x%x\n",hi,lo);
    return err == 0;
}
