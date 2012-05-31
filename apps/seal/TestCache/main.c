#include "stdmansos.h"
#include "schedtest.h"

enum {
    SENSOR_NOT_CACHEABLE = 0,
    SENSOR_SLOW_READ1,
    SENSOR_SLOW_READ2,
    SENSOR_SLOW_READ3,
    SENSOR_FAST_READ,
    TOTAL_SENSORS,
};

typedef struct SensorCache_s {
    union {
        uint8_t u8;
        uint16_t u16;
        uint32_t u32;
    } value;
    uint32_t expireTime; // in jiffies (TODO: overflow possible!)
//    uint16_t code;
} SensorCache_t;

SensorCache_t sensorCache[TOTAL_SENSORS];

typedef uint8_t (*ReadFunctionU8)(void);
typedef uint16_t (*ReadFunctionU16)(void);
typedef uint32_t (*ReadFunctionU32)(void);

uint8_t readSensorU8(uint16_t code, ReadFunctionU8 func, uint16_t expireTime)
{
    uint32_t now = getJiffies();
    SensorCache_t *cacheValue = &sensorCache[code];
    if (cacheValue->expireTime && !timeAfter32(now, cacheValue->expireTime)) {
        // take from cache
        return cacheValue->value.u8;
    }
    uint8_t result = func();
    if (expireTime) {
        // add to cache
        cacheValue->value.u8 = result;
        cacheValue->expireTime = now + expireTime;
    }
    return result;
}

uint16_t readSensorU16(uint16_t code, ReadFunctionU16 func, uint16_t expireTime)
{
    uint32_t now = getJiffies();
    SensorCache_t *cacheValue = &sensorCache[code];
    if (cacheValue->expireTime && !timeAfter32(now, cacheValue->expireTime)) {
        // take from cache
        return cacheValue->value.u16;
    }
    uint16_t result = func();
    if (expireTime) {
        // add to cache
        cacheValue->value.u16 = result;
        cacheValue->expireTime = now + expireTime;
    }
    return result;
}

uint32_t readSensorU32(uint16_t code, ReadFunctionU32 func, uint16_t expireTime)
{
    uint32_t now = getJiffies();
    SensorCache_t *cacheValue = &sensorCache[code];
    if (cacheValue->expireTime && !timeAfter32(now, cacheValue->expireTime)) {
        // take from cache
        return cacheValue->value.u32;
    }
    uint32_t result = func();
    if (expireTime) {
        // add to cache
        cacheValue->value.u32 = result;
        cacheValue->expireTime = now + expireTime;
    }
    return result;
}

void appMain(void)
{
    // while (1) {
    //     // change the default LED status
    //     ledToggle();
    //     // wait for 1000 milliseconds
    //     mdelay(1000);
    // }
}
