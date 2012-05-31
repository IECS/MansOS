#include "stdmansos.h"

static inline uint16_t slowRead1(void) {
    static uint16_t counter;
    return counter++;
}

static inline uint16_t slowRead2(void) {
    return 13;
}

static inline uint16_t slowRead3(void) {
    return 14;
}

static inline uint16_t fastRead(void) {
    return 0xF; // 15
}
