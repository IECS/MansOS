//-------------------------------------------
//      Cooperative Blink application using WAIT_TASK
//-------------------------------------------
#include "mansos.h"
#include "leds.h"
#include "coop_scheduler.h"

DEF_TASK(my_task_1) {
    BEGIN_TASK;

    while(1) {
        redLedOn();
        WAIT_TASK(500); // Yield control for at least 500 ms.
        redLedOff();
        WAIT_TASK(500);
    }

    END_TASK;
}

DEF_TASK(my_task_2) {
    BEGIN_TASK;
    
    while(1) {
        toggleGreenLed();
        YIELD_TASK(); // Yield control for it to be restored as soon as posible.
    }

    END_TASK;
}


//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain() {
    REGISTER_TASK(&my_task_1);
    REGISTER_TASK(&my_task_2);
}
