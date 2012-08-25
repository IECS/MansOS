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
        PRINTF("%lu BS: in app main\n", getFixedTime());
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
        PRINTF("%lu Forwarder: in app main\n", getFixedTime());
#else
        PRINTF("%lu Collector: in app main\n", getFixedTime());
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
//    radioSetTxPower(50); // 195 is default, according to doc
//    radioSetTxPower(195);

    Socket_t socket;
    socketOpen(&socket, NULL);
    socketBind(&socket, DATA_PORT);
    socketSetDstAddress(&socket, MOS_ADDR_ROOT);

    uint16_t counter = 0;
    for (;;) {
        PRINTF("%lu sending counter %i\n", getFixedTime(), counter);
        blueLedToggle();

        if (socketSend(&socket, &counter, sizeof(counter))) {
            PRINT("socketSend failed\n");
        }
//        mdelay(3000);
        mdelay(30000);
        redLedOn();
        mdelay(100);
        redLedOff();
        ++counter;
    }
}

#endif
