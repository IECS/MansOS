#include "common.h"

ComponentMap allComponents;

#define CHECK_VALUE_INT(p)         // TODO
#define CHECK_VALUE_BOOL(p)        // TODO
#define CHECK_VALUE_STRING(p)      // TODO
#define CHECK_VALUE_SAFE_INT(p)    // TODO
#define CHECK_VALUE_SAFE_BOOL(p)   // TODO
#define CHECK_VALUE_SAFE_STRING(p) // TODO

void Object::addUseCase(const UseCase &uc)
{
    // TODO: check for duplicated conditions!

    useCases.push_back(uc);
}

bool Object::addParameter(Parameter_t *p, Condition *condition)
{
    // printf("object::addParameter \"%s\"\n", p->name);
    // fflush(stdout);

    if (!strcasecmp(p->name, "period")) {
        CHECK_VALUE_INT(p);
        UseCase uc(this, condition);
        uc.period = p->value->u.integer;
        addUseCase(uc);
        return true;
    }
    return false;
}

bool Actuator::addParameter(Parameter_t *p, Condition *condition) {
    // printf("actuator::addParameter \"%s\"\n", p->name);
    // fflush(stdout);

    if (Object::addParameter(p, condition)) {
        return true;
    }
    if (!strcasecmp(p->name, "on_at")) {
        CHECK_VALUE_INT(p);
        UseCase uc(this, condition);
        uc.onTime = p->value->u.integer;
        addUseCase(uc);
        return true;
    }
    if (!strcasecmp(p->name, "off_at")) {
        CHECK_VALUE_INT(p);
        UseCase uc(this, condition);
        uc.offTime = p->value->u.integer;
        addUseCase(uc);
        return true;
    }
    return false;
}

bool Sensor::addParameter(Parameter_t *p, Condition *condition) {
    // printf("sensor::addParameter \"%s\"\n", p->name);
    // fflush(stdout);

    if (Object::addParameter(p, condition)) {
        return true;
    }
    return false;
}

bool Sink::addParameter(Parameter_t *p, Condition *condition) {
    // printf("sink::addParameter \"%s\"\n", p->name);
    // fflush(stdout);

    if (Object::addParameter(p, condition)) {
        return true;
    }
    if (!strcasecmp(p->name, "baudrate")) {
        CHECK_VALUE_INT(p);
        baudrate = p->value->u.integer;
        return true;
    }
    if (!strcasecmp(p->name, "aggregate")) {
        CHECK_VALUE_SAFE_BOOL(p);
        if (p->value) {
            aggregate = p->value->u.boolean;
        } else {
            aggregate = true;
        }
        return true;
    }
    return false;
}

bool Sink::addPacketField(Field_t *field)
{
    assert(field->name);
    if (field->value == NULL) {
        fields.push_back(toUpperCase(field->name));
    } else {
        CHECK_VALUE_INT(field->value);
        // TODO: also support boolean. And string (as a variable name)?
        constValuedFields.push_back(
                PacketField(field->name, sizeof(uint16_t),
                        field->value->u.integer));
    }
    return true;
}

// ----------------------------------------------------

Actuator *Actuator::createNew(const char *name)
{
    if (!strcasecmp(name, "Led")) {
        return new LedAct();
    }
    if (!strcasecmp(name, "RedLed")) {
        return new RedLedAct();
    }
    if (!strcasecmp(name, "GreenLed")) {
        return new GreenLedAct();
    }
    if (!strcasecmp(name, "BlueLed")) {
        return new BlueLedAct();
    }
    return NULL;
}

Sensor *Sensor::createNew(const char *name)
{
    if (!strcasecmp(name, "Light")) {
        return new LightSens();
    }
    if (!strcasecmp(name, "Humidity")) {
        return new HumiditySens();
    }
    return NULL;
}

Sink *Sink::createNew(const char *name)
{
    if (!strcasecmp(name, "Serial")) {
        return new SerialSink();
    }
    if (!strcasecmp(name, "Radio")) {
        return new RadioSink();
    }
    if (!strcasecmp(name, "Flash")) {
        return new FlashSink();
    }
    return NULL;
}

// -----------------------------------

// bool ObjectCollection::alreadyUsed(const char *name) const
// {
//     foreach_const(vector<Object *>, all,
//             if (!strcasecmp((*it)->getName(), name)) return true);
//     return false;
// }

// check for duplicates
// if (alreadyUsed(name)) {
//     userError("object %s is already used", name);
// }

Object *ObjectCollection::createNew(const char *name)
{
    Object *o = Actuator::createNew(name);
    if (!o) o = Sensor::createNew(name);
    if (!o) o = Sink::createNew(name);
    if (!o) {
        userError("unknown object %s", name);
    }
    // insert it in the collection
    all.push_back(o);
    // return it
    return o;
}

Object *ObjectCollection::findOrCreate(const char *name)
{
    // check if alreday used
    foreach_const(vector<Object *>, all,
            if (!strcasecmp((*it)->getName(), name)) return *it);

    // create new object
    return createNew(name);
}
