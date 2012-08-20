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

static void recvData(Socket_t *socket, uint8_t *data, uint16_t len)
{
    PRINTF("got %d bytes from 0x%04x (0x%02x) \n",
            len, socket->recvMacInfo->originalSrc.shortAddr, *data);
    redLedToggle();
}

void appMain(void)
{
    Socket_t socket;
    socketOpen(&socket, recvData);
    socketBind(&socket, DATA_PORT);

    for (;;) {
        PRINT("BS: in app main\n");
        redLedToggle();
        mdelay(SLEEP_TIME_MS);
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
        PRINT("Forwarder: in app main\n");
#else
        PRINT("Collector: in app main\n");
#endif
        redLedToggle();
        mdelay(1000);
    }
}

#else
// -------------------------------------
// Data originator's code
// -------------------------------------

void appMain(void)
{
    Socket_t socket;
    socketOpen(&socket, NULL);
    socketBind(&socket, DATA_PORT);
    socketSetDstAddress(&socket, MOS_ADDR_ROOT);

    uint16_t counter = 0;
    for (;;) {
        // PRINTF("sending counter %i\n", counter);
        if (socketSend(&socket, &counter, sizeof(counter))) {
			PRINT("socketSend failed\n");
        }
        mdelay(3000);
		redLedOn();
        mdelay(100);
		redLedOff();
		++counter;
    }
}

#endif
