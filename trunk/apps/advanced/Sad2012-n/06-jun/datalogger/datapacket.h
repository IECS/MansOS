
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

#define DATA_INTERVAL (1000ul * 60 * 10)  // -- once in every ten minutes
#define DATA_INTERVAL_SMALL (5000ul * 1)  // -- once in every five seconds

#define BLINK_INTERVAL      (1000ul * 5)

#define EXT_FLASH_RESERVED  (256 * 1024ul) // 256kb
