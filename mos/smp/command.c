#include "stdmansos.h"
#include "smp.h"
#include <kernel/reprogramming.h>
#include <hil/snum.h>
#include <net/addr.h>

bool getMoteType(bool set, uint8_t oidLen, SmpOid_t oid,
                 SmpVariant_t *arg, SmpVariant_t *response) {
    if (set) {
        PRINTF("getMoteType: read only!\n");
        return false;
    }
    response->type = ST_OCTET;
#if defined PLATFORM_TELOSB
    response->u.uint8 = SMP_MOTE_TELOSB;
#elif defined PLATFORM_PC
    response->u.uint8 = SMP_MOTE_PC;
#elif defined PLATFORM_ATMEGA
    response->u.uint8 = SMP_MOTE_ATMEGA;
#elif defined PLATFORM_EPIC
    response->u.uint8 = SMP_MOTE_EPIC;
#elif MCU_MSP430
    response->u.uint8 = SMP_MOTE_MSP430; // generic msp430
#else
#warning Implement correct mote type for this platform!
    response->u.uint8 = SMP_MOTE_OTHER;
#endif
    return true;
}

bool processPanAddress(bool set, uint8_t oidLen, SmpOid_t oid,
                       SmpVariant_t *arg, SmpVariant_t *response) {
    if (set && !arg) {
        PRINTF("processPanAddress: argument not supplied\n");
        return false;
    }
    if (arg && arg->type != ST_UINTEGER16) {
        PRINTF("processPanAddress: wrong arg type 0x%x\n", arg->type);
        return false;
    }

    if (set) {
        localAddress = response->u.uint16;
        return true;
    }

    response->type = ST_UINTEGER16;
    response->u.uint16 = localAddress;
    return true;
}

bool getIeeeAddress(bool set, uint8_t oidLen, SmpOid_t oid,
                    SmpVariant_t *arg, SmpVariant_t *response) {
    if (set) {
        PRINTF("getIeeeAddress: read only!\n");
        return false;
    }

    response->type = ST_UINTEGER64;
    return halGetSerialNumber((uint8_t *) &response->u.uint64);
}

bool processLedCommand(bool set, uint8_t oidLen, SmpOid_t oid,
                       SmpVariant_t *arg, SmpVariant_t *response) {
    if (set && !arg) {
        PRINTF("processLedCommand: argument not supplied\n");
        return false;
    }
    if (arg && arg->type != ST_OCTET) {
        PRINTF("processLedCommand: wrong arg type 0x%x\n", arg->type);
        return false;
    }

    uint8_t led = oid[0];
    if (set) {
        if (arg->u.uint8) {
            ledsOn(1 << led);
        } else {
            ledsOff(1 << led);
        }
    }
    response->type = ST_OCTET;
    arg->u.uint8 = ledsGet() & (1 << led);
    return true;
}

void ensureHumidityIsOn(void) {
    static bool on;
    if (!on) {
        on = true;
        humidityOn();
    }
}

bool processSensorCommand(bool set, uint8_t oidLen, SmpOid_t oid,
                          SmpVariant_t *arg, SmpVariant_t *response) {
    if (set) {
        PRINTF("processSensorCommand: read only!\n");
        return false;
    }

    uint8_t sensor = oid[0];
    response->type = ST_UINTEGER16;
    switch (sensor) {
    case SMP_SENSOR_TSR:
        response->u.uint16 = adcRead(ADC_LIGHT_TOTAL);
        break;
    case SMP_SENSOR_PAR:
        response->u.uint16 = adcRead(ADC_LIGHT_PHOTOSYNTHETIC);
        break;
    case SMP_SENSOR_VOLTAGE:
        response->u.uint16 = adcRead(ADC_INTERNAL_VOLTAGE);
        break;
    case SMP_SENSOR_HUMIDITY:
        ensureHumidityIsOn();
        response->u.uint16 = readHumidity();
        break;
    case SMP_SENSOR_TEMPERATURE:
        ensureHumidityIsOn();
        response->u.uint16 = readHTemperature();
        break;
    default:
        PRINTF("processSensorCommand: unknown sensor %d\n", sensor);
        return false;
    }
    return true;
}

bool processSmpBinaryPacket(uint8_t oidLen, SmpOid_t oid,
                            SmpVariant_t *arg, SmpVariant_t *response) {
    if (!arg || arg->type != ST_BINARY) {
        PRINTF("processReprogrammingCmd: wrong arg type 0x%x\n", arg->type);
        return false;
    }

    uint8_t len = *arg->u.data++;
    if (len < 2) {
        PRINTF("processReprogrammingCmd: too short %d\n", len);
        return false;
    }
    --len;
    uint8_t type = *arg->u.data;
    uint8_t size = 0;
    switch (type) {
    case RPROG_PACKET_START:
        size = sizeof(ReprogrammingStartPacket_t);
        break;
    case RPROG_PACKET_CONTINUE:
        size = sizeof(ReprogrammingContinuePacket_t);
        break;
    case RPROG_PACKET_REBOOT:
        size = sizeof(RebootCommandPacket_t);
        break;
    default:
        PRINTF("processReprogrammingCmd: wrong packet type 0x%x\n", type);
        return false;
    }
    if (len < size) {
        PRINTF("processReprogrammingCmd: too short %d\n", len);
        return false;
    }
    bool ok = true;
#if USE_REPROGRAMMING
    switch (type) {
    case RPROG_PACKET_START:
        processRSPacket((ReprogrammingStartPacket_t *) arg->u.data);
        break;
    case RPROG_PACKET_CONTINUE:
        ok = processRCPacket((ReprogrammingContinuePacket_t *) arg->u.data);
        break;
    case RPROG_PACKET_REBOOT:
        processRebootCommand((RebootCommandPacket_t *) arg->u.data);
        break;
    }
#else
    ok = false;
#endif
    response->type = ST_OCTET;
    response->u.uint8 = ok ? 0 : 0xff; // a return code is required
    return true;
}

bool processCommand(bool set, uint8_t command, uint8_t oidLen, SmpOid_t oid,
                    SmpVariant_t *arg, SmpVariant_t *response) {
    PRINTF("process %s command %d", (set ? "set" : "get"), command);
    if (arg && arg->type == 0xFF) arg = NULL;
    switch (command) {
    case SMP_RES_TYPE:
        return getMoteType(set, oidLen, oid, arg, response);
    case SMP_RES_ADDRESS:
        return processPanAddress(set, oidLen, oid, arg, response);
    case SMP_RES_IEEE_ADDRESS:
        return getIeeeAddress(set, oidLen, oid, arg, response);
    case SMP_RES_LED:
        return processLedCommand(set, oidLen, oid, arg, response);
    case SMP_RES_SENSOR:
        return processSensorCommand(set, oidLen, oid, arg, response);
    case SMP_RES_BINARY_PACKET:
        return processSmpBinaryPacket(oidLen, oid, arg, response);
    default:
        PRINTF("command %d not implemented\n", command);
        break;
    }

    return false;
}
