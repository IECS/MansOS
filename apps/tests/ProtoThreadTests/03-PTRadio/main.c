//-------------------------------------------
//  Cooperative Radio application, using radio driver process
//-------------------------------------------

#include <stdmansos.h>
#include <net/radio_packet_buffer.h>

PROCESS(send_process, "Radio send");
PROCESS(receive_process, "Radio listen");
AUTOSTART_PROCESSES(&send_process, &receive_process);

/*---------------------------------------------------------------------*/
PROCESS_THREAD(send_process, ev, data)
{
  PROCESS_BEGIN();

  static struct etimer timer;
  static uint16_t counter = 0;

  while (1) {
      PRINTF("Sending %i\n", counter);
      radioSend(&counter, sizeof(counter));
      ++counter;
      redLedToggle();
      waitTimer(timer, CLOCK_SECOND);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------*/
PROCESS_THREAD(receive_process, ev, data)
{
  PROCESS_BEGIN();

  radioOn();

  while (1) {
      waitRadioPacket(ev);
      if (radioPacketBuffer->receivedLength >= 2) {
          uint16_t counter = ((uint16_t *) radioPacketBuffer->buffer)[0];
          PRINTF("Received counter %i\n", counter);
          greenLedToggle();
      }
  }

  PROCESS_END();
}
