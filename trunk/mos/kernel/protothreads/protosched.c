/*
 * protosched.c
 *
 *  Created on: 2012-07-13
 *      Author: girts
 */

#include <kernel/protothreads/process.h>
#include <kernel/protothreads/etimer.h>
#include <kernel/protothreads/radio-process.h>
#include <kernel/protothreads/autostart.h>
#include <stdmansos.h>
#include <stdlib.h>

#warning "Using proto-threads! appMain() will not be called! Use AUTOSTART_PROCESSES instead!"

#if DEBUG
#define PROTO_SCHED_DEBUG 1
#endif

#if PROTO_SCHED_DEBUG
#include <platform.h>
#define PSPRINTF(...) PRINTF(__VA_ARGS__)
#else
#define PSPRINTF(...) do {} while (0)
#endif


static void
print_processes(struct process * const processes[])
{
  /*  const struct process * const * p = processes;*/
  PSPRINTF("Starting");
  while(*processes != NULL) {
    PSPRINTF(" '%s'", (*processes)->name);
    processes++;
  }
  PSPRINTF("\n");
}


void startProtoSched() {
    process_init();
    process_start(&etimer_process, NULL);
#if USE_RADIO
    process_start(&radio_process, NULL);
#endif

    print_processes(autostart_processes);
    autostart_start(autostart_processes);

    while(1) {
      int r;

      do {
        /* Reset watchdog. */
        // watchdog_periodic();
        r = process_run();
      } while(r > 0);

      /*
       * Idle processing.
       */
      // Ensure, that no interrupt will be caught after we have checked,
      // that process_nevents == 0
      Handle_t h;
      ATOMIC_START(h);
      /* uart1_active is for avoiding LPM3 when still sending or receiving */
      if(process_nevents() != 0 || !PLATFORM_CAN_SLEEP()) {
          ATOMIC_END(h);          /* Re-enable interrupts. */
      } else {
          //PRINTF("zZz\n");
          ENTER_SLEEP_MODE();

//          static unsigned long irq_energest = 0;
//
//          /* Re-enable interrupts and go to sleep atomically. */
//          ENERGEST_OFF(ENERGEST_TYPE_CPU);
//          ENERGEST_ON(ENERGEST_TYPE_LPM);
//          /* We only want to measure the processing done in IRQs when we
//             are asleep, so we discard the processing time done when we
//             were awake. */
//          energest_type_set(ENERGEST_TYPE_IRQ, irq_energest);
//          watchdog_stop();
          /* check if the DCO needs to be on - if so - only LPM 1 */

//          if (msp430_dco_required) {
//              _BIS_SR(GIE | CPUOFF); /* LPM1 sleep for DMA to work!. */
//          } else {
//              _BIS_SR(GIE | SCG0 | SCG1 | CPUOFF); /* LPM3 sleep. This
//                          statement will block
//                          until the CPU is
//                          woken up by an
//                          interrupt that sets
//                          the wake up flag. */
//          }
          /* We get the current processing time for interrupts that was
             done during the LPM and store it for next time around.  */
//          dint();
//          irq_energest = energest_type_time(ENERGEST_TYPE_IRQ);
//          eint();
//          watchdog_start();
//          ENERGEST_OFF(ENERGEST_TYPE_LPM);
//          ENERGEST_ON(ENERGEST_TYPE_CPU);
      }
    }


}
