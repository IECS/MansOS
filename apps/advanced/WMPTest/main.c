#include "stdmansos.h"
#include "wmp.h"

void appMain(void)
{
//    humidityOn();

//     while (1) {
//         uint16_t light = lightRead();
// //        uint16_t humidity = humidityRead();
// //        uint16_t temperature = temperatureRead();
//         PRINTF("light=%u\n", light);
// //        PRINTF("humidity = %u\n", humidity);
// //        PRINTF("temperature = %u\n", temperature);
// //        msleep(2000); // sleep two seconds
// msleep(200);
//     }
    wmpInit();

    for (;;) {
        PRINTF(".\n");
        mdelay(1000);
    }
}
