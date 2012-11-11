//-------------------------------------------
//  Cooperative Blink application showing YIELD
//-------------------------------------------

// Warning: no appMain() is called! Use PROCESS_AUTOSTART structure instead!

#include "stdmansos.h"
#include "leds.h"

PROCESS(blink_red_process, "blink_red");
PROCESS(blink_green_process, "blink_green");
PROCESS(blink_blue_process, "blink_blue");
AUTOSTART_PROCESSES(&blink_red_process, &blink_green_process, &blink_blue_process);


/*---------------------------------------------------------------------*/
PROCESS_THREAD(blink_red_process, ev, data)
{
  PROCESS_BEGIN();

  while (1) {
      PROCESS_PAUSE();
      redLedToggle();
      mdelay(500);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------*/
PROCESS_THREAD(blink_green_process, ev, data)
{
  PROCESS_BEGIN();

  while (1) {
      PROCESS_PAUSE();
      greenLedToggle();
      mdelay(500);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------*/
PROCESS_THREAD(blink_blue_process, ev, data)
{
  PROCESS_BEGIN();

  while (1) {
      PROCESS_PAUSE();
      blueLedToggle();
      mdelay(500);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------*/
