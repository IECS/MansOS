//----------------------------------------------------------------------------
// Test digital acceleration sensor ADXL345 with I2C interface
//----------------------------------------------------------------------------

#include "stdmansos.h"
#include <adxl345/adxl345.h>

//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    // wait a while after programming the mote - to be able to start
    // serial port listening app on the PC
    PRINTF("ADXL345 test app\n");

    adxl345Init();
    adxl345SetGRange(ADXL345_RANGE_4G);
    // allow sensor to init
    mdelay(1000);

    while (1) {
        int16_t acc_x = adxl345ReadAxis(ADXL345_X_AXIS);
        int16_t acc_y = adxl345ReadAxis(ADXL345_Y_AXIS);
        int16_t acc_z = adxl345ReadAxis(ADXL345_Z_AXIS);

        PRINTF("%d;%d;%d\n", acc_x, acc_y, acc_z);
        mdelay(1000);
    }
}

