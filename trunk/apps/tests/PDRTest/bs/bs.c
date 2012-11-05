#include <stdmansos.h>
#include "../common.h"

#define MAX_NEIGHBORS 11

uint8_t nbrCount;
RadioInfoPacket_t nbrs[MAX_NEIGHBORS];

bool checkNeighbor(uint16_t addr, RadioInfoPacket_t *packet)
{
    uint8_t i;
    for (i = 0; i < nbrCount; i++) {
        if (nbrs[i].address == packet->address) {
            nbrs[i] = *packet;
            return true;
        }
    }

    // If we got so far it means that there's no such neighbor, let's add one
    if (nbrCount < MAX_NEIGHBORS) {
        nbrs[nbrCount++] = *packet;
        return true;
    } 
    return false;
}

void recvCallback(void)
{
    RadioInfoPacket_t packet;
    int16_t len;

    greenLedToggle();
    len = radioRecv(&packet, sizeof(packet));
    if (len < 0) {
        PRINTF("radio receive failed\n");
        return;
    }
    if (len < sizeof(packet)) {
        PRINTF("too small!\n");
        return;
    }
    // PRINT("radio rx!\n");
    checkNeighbor(packet.address, &packet);
}

void dumpNeighbors(void)
{
    uint8_t i;
    for (i = 0; i < nbrCount; i++) {
        RadioInfoPacket_t *p = &nbrs[i];
        PRINTF("%04x: last: %d num: %d PDR: %d%% ne: %d PDR-NE: %d%% rssi: %d lqi: %d\n",
                p->address, p->lastTestNo, p->numTests, p->avgPdr,
                p->numTestsNe, p->avgPdrNe, p->avgRssiNe, p->avgLqiNe);
    }
}

void appMain(void)
{
    radioSetChannel(BS_CHANNEL);
    radioSetReceiveHandle(recvCallback);
    radioOn();

    for (;;) {
        PRINTF(".\n");
        DISABLE_INTS();
        dumpNeighbors();
        ENABLE_INTS();
        mdelay(5000);
    }
}
