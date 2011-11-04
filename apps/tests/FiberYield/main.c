//-------------------------------------------
//      Cooperative Blink application showing YIELD
//-------------------------------------------


#include "mansos.h"
#include "stdmansos.h"
#include "leds.h"
#include "coop_scheduler.h" // Required for fibers to work.


DEF_TASK(my_task_1) {
    //
    // Any code here will be re-run every time the task is resumed.
    // You probably don't want to do it in most cases.
    //
    BEGIN_TASK; // Must start coroutine body like this.

    while(1) {
        redLedOn();
        greenLedOff();
        mdelay(500);
        YIELD_TASK(); // Yield the control to the scheduler to allow other tasks to run.
    }

    END_TASK;   // Must end coroutine body like this.
}

DEF_TASK(my_task_2) {
    BEGIN_TASK;
    
    while(1) {
        redLedOff();
        greenLedOn();
        mdelay(500);
        YIELD_TASK();
    }

    END_TASK;
}

//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain() {
    //
    // init code here
    //
    REGISTER_TASK(&my_task_1);  // Must register every task like this for it to be run.
    REGISTER_TASK(&my_task_2);  // The order is 'priority'.
}
