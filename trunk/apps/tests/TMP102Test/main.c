//----------------------------------------------------------------------------
// Test digital temperature sensor TMP102 with I2C interface
//----------------------------------------------------------------------------

#include "stdmansos.h"
#include "dprint.h"
#include <tmp102/tmp102.h>

//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    // wait a while after programming the mote - to be able to start
    // serial port listening app on the PC
    PRINTF("TMP102 test app\n");

    // allow sensor to init
    tmp102Init();
    msleep(1000);

    while (1) {
        int16_t temp_c = tmp102ReadDegrees();

        PRINTF("%u C\n", temp_c);
        msleep(1000);
    }
}

