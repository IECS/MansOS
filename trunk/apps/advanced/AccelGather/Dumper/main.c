#include <stdmansos.h>

typedef struct Packet {
    uint32_t timestamp;
    uint16_t acc_x;
    uint16_t acc_y;
    uint16_t acc_z;
} Packet_t;

static inline void printPacket(Packet_t *p) {
    PRINTF("%lu %d %d %d\n", p->timestamp,
            p->acc_x, p->acc_y, p->acc_z);
}

//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    PRINTF("\n\nAccelerometer data dumping app\n");

    extFlashWake();

    uint32_t extFlashAddr = 0;
    extFlashRead(extFlashAddr, (uint8_t *) &extFlashAddr, sizeof(extFlashAddr));
    if (extFlashAddr != 0) {
        PRINTF("Wrong magic!\n");
        return;
    }

    extFlashAddr = 4;

    Packet_t p;
    for (; extFlashAddr < 2ul *1024 * 1024; extFlashAddr += sizeof(p)) {
        extFlashRead(extFlashAddr, (uint8_t *) &p, sizeof(p));
        if (p.timestamp == 0xffffffff
                && p.acc_x == 0xffff
                && p.acc_y == 0xffff
                && p.acc_z == 0xffff) {
            break;
        }
        printPacket(&p);
    }
}
