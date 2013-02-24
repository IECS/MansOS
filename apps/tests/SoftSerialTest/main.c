#include <stdmansos.h>

//-------------------------------------------
void appMain(void)
{
    serialInit(0, 9600, 0);
    mdelay(1);
    PRINTF("hello world\n");
//    PRINTF("wwwwwwwwww");
}
