#include "stdmansos.h"
#include <net/socket.h>
#include <net/routing.h>

//-----------------
// constants
//-----------------
enum { DATA_PORT  = 123 };
#define SLEEP_TIME_MS 2000

#if USE_ROLE_BASE_STATION
// -------------------------------------
// Base station's code
// -------------------------------------

// receiver callback function
static void recvData(Socket_t *socket, uint8_t *data, uint16_t len)
{
    PRINTF("got %d bytes from 0x%04x (0x%02x) \n",
            len, socket->recvMacInfo->originalSrc.shortAddr, *data);
    redLedToggle();
}

void appMain(void)
{
    // open and bind a socket
    Socket_t socket;
    socketOpen(&socket, recvData);
    socketBind(&socket, DATA_PORT);

    for (;;) {
        PRINTF("%lu BS: in app main\n", getSyncTimeSec());
        redLedToggle();
        msleep(SLEEP_TIME_MS);
    }
}

#elif USE_ROLE_FORWARDER || USE_ROLE_COLLECTOR

// -------------------------------------
// Forwarder's code
// -------------------------------------
void appMain(void)
{
    for (;;) {
#ifdef USE_ROLE_FORWARDER
        PRINTF("%lu Forwarder: in app main\n", getSyncTimeSec());
#else
        PRINTF("%lu Collector: in app main\n", getSyncTimeSec());
#endif
        redLedToggle();
        msleep(SLEEP_TIME_MS);
    }
}

#else
// -------------------------------------
// Data originator's code
// -------------------------------------

void appMain(void)
{
    // declare, open, and bind a socket
    Socket_t socket;
    socketOpen(&socket, NULL);
    socketBind(&socket, DATA_PORT);
    socketSetDstAddress(&socket, MOS_ADDR_ROOT);

    uint16_t counter = 0;
    for (;;) {
        // send counter value
        PRINTF("%lu sending counter %i\n", getSyncTimeSec(), counter);
        if (socketSend(&socket, &counter, sizeof(counter))) {
            PRINTF("socketSend failed\n");
        }

        // increase counter value
        ++counter;

        // wait for approximately 30 seconds
        sleep(30);

        // blink a LED
        redLedOn();
        mdelay(100);
        redLedOff();
    }
}

#endif
