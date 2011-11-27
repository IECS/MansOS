#ifndef SADLANG_OBJECTS_H
#define SADLANG_OBJECTS_H

#include <vector>
#include <string>
using namespace std;
#include "condition.h"

struct Parameter_s;
struct Object;
struct Condition;

struct UseCase
{
    Object *parent;
    Condition *condition;

    // period this actuator is toggled / sensor read / packet sent
    unsigned period;
    // time this actuator is turned on/off
    unsigned onTime;
    unsigned offTime;

    enum Action {
        ACTION_NONE,
        ACTION_TURN_ON,
        ACTION_TURN_OFF,
        ACTION_BLINK
    };
    Action action;
    unsigned blinkTimes;

    UseCase(Object *p, Condition *c) :
            parent(p),
            condition(c),
            period(~0u),
            onTime(~0u),
            offTime(~0u),
            action(ACTION_NONE),
            blinkTimes(0) {
    }
};

struct Object
{
    // get name (e.g. for serial printing)
    virtual const char *getName() const = 0;
    // get name for config file, used in USE_xxx=y|n statements
    virtual const char *getConfigName() const = 0;
    // get additional #include file, if any
    virtual const char *getIncludeFile() const { return NULL; }

    // an object can have one of these types
    enum Type {
        ACTUATOR,
        SENSOR,
        SINK
    } type;

    typedef vector<UseCase> CaseVector;
    CaseVector useCases;

    // constructor
    Object(Type xtype) : type(xtype) {}

    void addUseCase(const UseCase &);

    void generateIncludes();
    virtual bool generateConstants();
    virtual bool generateConstants(const UseCase &, int num);

    virtual bool generateVariables();
    virtual bool generateVariables(const UseCase &, int num);

    virtual bool generateAppMainCode();
    virtual bool generateAppMainCode(const UseCase &, int num);

    // generate alarm callback code
    virtual void generateCode(const UseCase &, int num);
    virtual void generateCode();

    virtual bool addParameter(Parameter_t *, Condition *);
};

// -----------------------------------

struct Actuator : public Object
{
    // get function used for turning this object on
    virtual const char *getOnFunction() const = 0;
    // get function used for turning this object off
    virtual const char *getOffFunction() const = 0;
    // get function used for toggling this object's state
    virtual const char *getToggleFunction() const = 0;

    // constructor
    Actuator() : Object(ACTUATOR) {}

    // factory method
    static Actuator *createNew(const char *name);

    virtual bool addParameter(Parameter_t *, Condition *);
};

// default led
class LedAct : public Actuator {
    virtual const char *getName() const { return "Led"; }
    virtual const char *getConfigName() const { return "LEDS"; }
    virtual const char *getOnFunction() const { return "ledOn"; }
    virtual const char *getOffFunction() const { return "ledOff"; }
    virtual const char *getToggleFunction() const { return "ledToggle"; }
};

class RedLedAct : public Actuator {
    virtual const char *getName() const { return "RedLed"; }
    virtual const char *getConfigName() const { return "LEDS"; }
    virtual const char *getOnFunction() const { return "redLedOn"; }
    virtual const char *getOffFunction() const { return "redLedOff"; }
    virtual const char *getToggleFunction() const { return "redLedToggle"; }
};

class GreenLedAct : public Actuator {
    virtual const char *getName() const { return "GreenLed"; }
    virtual const char *getConfigName() const { return "LEDS"; }
    virtual const char *getOnFunction() const { return "greenLedOn"; }
    virtual const char *getOffFunction() const { return "greenLedOff"; }
    virtual const char *getToggleFunction() const { return "greenLedToggle"; }
};

class BlueLedAct : public Actuator {
    virtual const char *getName() const { return "BlueLed"; }
    virtual const char *getConfigName() const { return "LEDS"; }
    virtual const char *getOnFunction() const { return "blueLedOn"; }
    virtual const char *getOffFunction() const { return "blueLedOff"; }
    virtual const char *getToggleFunction() const { return "blueLedToggle"; }
};

// -----------------------------------

struct Sensor : public Object
{
    // get function used for reading this sensor
    virtual const char *getReadFunction() const = 0;
    // get data size in bytes
    virtual unsigned getDataSize() const = 0;
    // constructor
    Sensor() : Object(SENSOR) {}
    // factory method
    static Sensor *createNew(const char *name);

    virtual bool addParameter(Parameter_t *, Condition *);
};

class LightSens : public Sensor {
public:
    virtual const char *getName() const { return "Light"; }
    virtual const char *getConfigName() const { return "ADC"; } // XXX
    virtual const char *getReadFunction() const { return "readLight"; }
    virtual unsigned getDataSize() const { return 2; }
};

class HumiditySens : public Sensor {
public:
    virtual const char *getName() const { return "Humidity"; }
    virtual const char *getConfigName() const { return "HUMIDITY"; }
    virtual const char *getReadFunction() const { return "readHumidity"; }
    virtual unsigned getDataSize() const { return 2; }
};

// -----------------------------------

#define DEFAULT_SERIAL_BAUDRATE     38400
#define DEFAULT_RADIO_CHANNEL       11

struct PacketField {
    string name;
    unsigned size;
    PacketField(const char *n, unsigned s) : name(n), size(s) {}
    static bool compare(const PacketField &x1, const PacketField &x2) {
        return x1.size > x2.size;
    }
};

struct Sink : public Object
{
    // 'aggregate' is true when a packet is generated
    bool aggregate;
    bool crc;
    // serial port attributes
    unsigned baudrate;
    // radio attributes
    unsigned radioChannel;
    // if nonempty: include only these fields in the packet / output
    vector<string> fields;
    // fields actually used
    vector<PacketField> usedFields;

    // constructor
    Sink() : Object(SINK),
             aggregate(false),
             crc(false),
             baudrate(DEFAULT_SERIAL_BAUDRATE),
             radioChannel(DEFAULT_RADIO_CHANNEL) {}
    // factory method
    static Sink *createNew(const char *name);

    virtual bool generateConstants();

    virtual void generateCode();
    virtual void generateCodeForSensor(Sensor *);

    virtual const char *getSendFuntion() const = 0;

    virtual bool addParameter(Parameter_t *, Condition *);
};

class SerialSink : public Sink {
public:
    virtual const char *getName() const { return "Serial"; }
    virtual const char *getConfigName() const { return "SERIAL"; }
    virtual void generateCode();
    virtual const char *getSendFuntion() const { return "serialPacketPrint()"; }
};

class RadioSink : public Sink {
public:
    virtual const char *getName() const { return "Radio"; }
    virtual const char *getConfigName() const { return "RADIO"; }
    RadioSink() {
        aggregate = true;
        crc = true;
    }
    virtual const char *getSendFuntion() const {
        return "radioSend(&radioPacket, sizeof(radioPacket))";
    }
};

class FlashSink : public Sink {
public:
    virtual const char *getName() const { return "Flash"; }
    virtual const char *getConfigName() const { return "EXT_FLASH"; }
    virtual const char *getSendFuntion() const {
        return "flashStreamWrite(&flashPacket, sizeof(flashPacket))";
    }
};

// -----------------------------------

class ObjectCollection {
    vector<Object *> all;
public:
    typedef vector<Object *>::iterator iterator;
    typedef vector<Object *>::const_iterator const_iterator;
    iterator begin() { return all.begin(); }
    const_iterator begin() const { return all.begin(); }
    iterator end() { return all.end(); }
    const_iterator end() const { return all.end(); }

    // bool alreadyUsed(const char *name) const;
    Object *createNew(const char *name);
    Object *findOrCreate(const char *name);
};

extern ObjectCollection objectsUsed;

#endif
