//----------------------------------------------------------------------------
// Test digital acceleration sensor ADXL345 with I2C interface
//----------------------------------------------------------------------------

#include "stdmansos.h"
#include "dprint.h"
#include <adxl345/adxl345.h>

//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    // wait a while after programming the mote - to be able to start
    // serial port listening app on the PC
    msleep(3000);
    PRINTF("ADXL345 test app\n");

    adxl345_init();
    adxl345_setGRange(ADXL345_RANGE_4G);
    // allow sensor to init
    msleep(1000);

    while (1) {
        int16_t acc_x = adxl345_readAxis(ADXL345_X_AXIS);
        int16_t acc_y = adxl345_readAxis(ADXL345_Y_AXIS);
        int16_t acc_z = adxl345_readAxis(ADXL345_Z_AXIS);

        PRINTF("%u;%u;%u\n", acc_x, acc_y, acc_z);
        msleep(100);
    }
}

