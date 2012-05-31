#include "stdmansos.h"

static inline void slowPreread1(void) {
}

static inline uint16_t slowRead1(void) {
    static uint16_t counter;
    return counter++;
}

static inline void slowPreread2(void) {
}


static inline uint16_t slowRead2(void) {
    return 13;
}

static inline void slowPreread3(void) {
}

static inline uint16_t slowRead3(void) {
    return 14;
}

static inline uint16_t fastRead(void) {
    return 0xF; // 15
}
