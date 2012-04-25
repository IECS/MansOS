#include <stdmansos.h>

uint16_t array[200];

void appMain(void)
{
    uint16_t i;
    array[0] = 1;
    for (i = 0; ; ++i) {
        ledToggle();
        mdelay(1000);
    }
}
