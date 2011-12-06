


struct RadioInfoPacket_s {
    uint16_t address;
    uint16_t lastTestNo;
    uint16_t numTests;
    uint16_t avgPdr;
    uint16_t numTestsNe;
    uint16_t avgPdrNe;
    uint16_t avgRssiNe;
    uint16_t avgLqiNe;
};

typedef struct RadioInfoPacket_s RadioInfoPacket_t;


#define TEST_CHANNEL 15
#define BS_CHANNEL   26
