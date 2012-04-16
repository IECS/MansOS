#ifndef SAD_DATA_PACKET_H
#define SAD_DATA_PACKET_H

#ifndef MANSOS
#include <stdint.h>

#define PACKED __attribute__((packed))

typedef uint16_t MosShortAddr;
typedef uint16_t Seqnum_t;
#endif

struct DataPacket_s {
    uint32_t timestamp;
    uint16_t sourceAddress;
    uint16_t dataSeqnum;
    uint16_t islLight;
    uint16_t apdsLight0;
    uint16_t apdsLight1;
    uint16_t sq100Light;
    uint16_t internalVoltage;
    uint16_t internalTemperature;
    uint16_t sht75Humidity;
    uint16_t sht75Temperature;
    uint16_t crc;
} PACKED;

typedef struct DataPacket_s DataPacket_t;

// routing information, sent out from base station, forwarded by nodes
struct SadRoutingInfoPacket_s {
    uint8_t packetType;        // ROUTING_INFORMATION
    uint8_t __reserved;
    MosShortAddr rootAddress;  // address of the base station / gateway
    uint16_t hopCount;         // distance from root
    Seqnum_t seqnum;           // sequence number
    uint32_t rootClock;        // used for time sync to calculate the delta
} PACKED;

typedef struct SadRoutingInfoPacket_s SadRoutingInfoPacket_t;

#ifndef MANSOS
enum {
    ROUTING_INFORMATION,
    ROUTING_REQUEST,
};
#endif

typedef struct Neighbors_s {
    uint16_t addr;
    uint16_t seqNum;
} Neighbors_t;


#define MAX_NEIGHBORS    20

#define POSSIBLE_SEQNUM_OFFSET 5

#define SAD_ROUTING_ID   0xAA
#define SAD_DATA_ID      0xBB
#define SAD_I_AM_HERE_ID 0xD8

// Is 'a' is after 'b'?
#define seqnumAfter(a, b) ((int16_t) ((b) - (a)) < 0)

#endif
