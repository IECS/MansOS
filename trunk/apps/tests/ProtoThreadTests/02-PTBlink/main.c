//-------------------------------------------
//  Cooperative Blink application using timers
//-------------------------------------------

// Warning: no appMain() is called! AUTOSTART_PROCESSES structure is used instead.

#include "stdmansos.h"

PROCESS(blink_red_process, "blink_red");
PROCESS(blink_green_process, "blink_green");
PROCESS(blink_blue_process, "blink_blue");
AUTOSTART_PROCESSES(&blink_red_process, &blink_green_process, &blink_blue_process);

/*---------------------------------------------------------------------*/
PROCESS_THREAD(blink_red_process, ev, data)
{
  PROCESS_BEGIN();

  static struct etimer timer;

  while (1) {
      etimer_set(&timer, CLOCK_SECOND / 2);
      PROCESS_WAIT_UNTIL(etimer_expired(&timer));
      redLedToggle();
      yellowLedToggle(); // for Atmega
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------*/
PROCESS_THREAD(blink_green_process, ev, data)
{
  PROCESS_BEGIN();

  static struct etimer timer;

  while (1) {
      etimer_set(&timer, CLOCK_SECOND);
      PROCESS_WAIT_UNTIL(etimer_expired(&timer));
      greenLedToggle();
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------*/
PROCESS_THREAD(blink_blue_process, ev, data)
{
  PROCESS_BEGIN();

  static struct etimer timer;

  while (1) {
      etimer_set(&timer, CLOCK_SECOND * 2);
      PROCESS_WAIT_UNTIL(etimer_expired(&timer));
      blueLedToggle();
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------*/

