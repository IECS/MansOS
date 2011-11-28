#include "common.h"

#define ALLOW_INCOMPLETE_PACKETS 0 // XXX

ObjectCollection objectsUsed;
vector<Actuator *> actuatorsUsed;
vector<Sensor *> sensorsUsed;
vector<Sink *> sinksUsed;
bool usePackets;

extern const char *outputDir;
extern const char *inputFileName;
extern const char *outputFileName;
FILE *outputFile;

struct IntTypes {
    const char *name;
    const char *name2;
    const char *noValue;
    const char *format;
};

IntTypes uintTypes[] = {
    { "", "", "", "" },
    { "uint8_t", "U8", "0xff", "%u" },
    { "uint16_t", "U16", "0xffff", "%u" },
    { "", "", "", "" },
    { "uint32_t", "U32", "0xffffffff", "%lu" },
};

IntTypes intTypes[] = {
    { "", "", "", "" },
    { "int8_t", "U8", "0xff", "%d" },
    { "int16_t", "U16", "0xffff", "%d" },
    { "", "", "", "" },
    { "int32_t", "U32", "0xffffffff", "%ld" },
};

void outputConfig(Object *o) {
    const char *configName = o->getConfigName();
    if (configName) {
        fprintf(outputFile, "USE_%s=y\n", configName);
    }
}

void generateConfigFile(void)
{
    string configFileName;
    if (outputDir) {
        configFileName = outputDir;
        configFileName += '/';
    }
    configFileName += "config";
    outputFile = fopen(configFileName.c_str(), "w");
    if (!outputFile) sysError("failed to write configuration file %s", configFileName.c_str());
    foreach(ObjectCollection, objectsUsed, outputConfig(*it));
    fclose(outputFile);
}

void generateMakefile(void)
{
    string makefileFileName;
    if (outputDir) {
        makefileFileName += outputDir;
        makefileFileName += '/';
    }
    makefileFileName += "Makefile";
    outputFile = fopen(makefileFileName.c_str(), "w");
    if (!outputFile) sysError("failed to write Makefile file %s", makefileFileName.c_str());
    const char *fileTemplate = 
            "SOURCES = %s\n"
            "APPMOD = App\n"
            "PROJDIR = $(CURDIR)\n"
            "ifndef MOSROOT\n"
            "  MOSROOT = $(PROJDIR)/../..\n"
            "endif\n"
            "include ${MOSROOT}/mos/make/Makefile\n";
    fprintf(outputFile, fileTemplate, outputFileName);
    fclose(outputFile);
}

void Object::generateIncludes() {
    const char *includeFile = getIncludeFile();
    if (includeFile) {
        fprintf(outputFile, "#include \"%s\"\n", includeFile);
    }
}

void generateIncludes(void)
{
    fprintf(outputFile, "#include <stdmansos.h>\n");
    fprintf(outputFile, "#include <lib/codec/crc.h>\n");
    foreach(ObjectCollection, objectsUsed, (*it)->generateIncludes());
    fprintf(outputFile, "\n");
}

void generatePacketType(Sink *sink, vector<PacketField> fields)
{
    vector<PacketField> usedFields;
    if (sink->fields.empty()) {
        // use all fields
        usedFields = fields;
    } else {
        // use only explicitly specified fields
        foreach (vector<PacketField>, fields,
                if (has(sink->fields, toUpperCase(it->name))) usedFields.push_back(*it));
    }
    foreach (vector<PacketField>, sink->constValuedFields,
            usedFields.push_back(*it));

    if (usedFields.empty()) {
        sysError("%sPacket has no fields", sink->getName());
    }

    fprintf(outputFile, "struct %sPacket_s {\n", sink->getName());

    unsigned packetSize = 0;
    foreach (vector<PacketField>, usedFields,
            fprintf(outputFile, "    uint%d_t %s;\n", it->size * 8, 
                    toCamelCase(it->name).c_str());
            packetSize += it->size;
        );

    // 2-byte crc
    if (sink->crc) {
        if (packetSize & 0x1) {
            // add padding field
            fprintf(outputFile, "    uint8_t __reserved;\n");
            packetSize++;
        }
        fprintf(outputFile, "    uint16_t crc;\n");
    }

    // finish the packet
    fprintf(outputFile, "} PACKED;\n\n");
    // and add a typedef
    fprintf(outputFile, "typedef struct %sPacket_s %sPacket_t;\n\n",
            sink->getName(), sink->getName());

    sink->usedFields = usedFields;
}

void generateTypes(void)
{
    // generate packet types, if necessary

    foreach(vector<Sink *>, sinksUsed,
            if ((*it)->aggregate) usePackets = true);

    if (sensorsUsed.empty() || !usePackets) return;

    vector<PacketField> fields;
    foreach(vector<Sensor *>, sensorsUsed,
            fields.push_back(PacketField((*it)->getName(), (*it)->getDataSize())));

    sort(fields.begin(), fields.end(), PacketField::compare);

    vector<Sink *>::iterator it = sinksUsed.begin();
    for (; it != sinksUsed.end(); ++it) {
        if (!(*it)->aggregate) continue;

        // generate packet for this sink...
        generatePacketType(*it, fields);
    }
}

bool Object::generateConstants(const UseCase &uc, int num)
{
    char condStr[20] = "";
    if (uc.condition) {
        sprintf(condStr, "_CONDITION%d", uc.condition->id);
    }

    bool ret = false;

    if (IS_SPECIFIED(uc.period)) {
        ret = true;
        fprintf(outputFile, "#define %s%s_PERIOD    %d\n",
                toUpperCase(getName()).c_str(), condStr, uc.period);
    }
    if (IS_SPECIFIED(uc.onTime)) {
        ret = true;
        fprintf(outputFile, "#define %s%s_ON_TIME    %d\n",
                toUpperCase(getName()).c_str(), condStr, uc.onTime);
    }
    if (IS_SPECIFIED(uc.offTime)) {
        ret = true;
        fprintf(outputFile, "#define %s%s_OFF_TIME    %d\n",
                toUpperCase(getName()).c_str(), condStr, uc.offTime);
    }
    return ret;
}

bool Object::generateConstants()
{
    int i = 1;
    foreach (CaseVector, useCases,
            if (generateConstants(*it, i)) i++);
    return i > 1;
}

bool Sink::generateConstants()
{
    bool ret = Object::generateConstants();

    fprintf(outputFile, "\n");
    if (!usedFields.empty()) {
        ret = true;
        fprintf(outputFile, "#define %s_PACKET_NUM_FIELDS    %d\n",
                toUpperCase(getName()).c_str(), usedFields.size());
    }

    return ret;
}

bool Sensor::generateConstants() {
    fprintf(outputFile, "#define %s_NO_VALUE    %s\n",
            toUpperCase(getName()).c_str(),
            uintTypes[getDataSize()].noValue);

    return Object::generateConstants();
}

bool Condition::generateConstants()
{
    if (!numberSpecified) return false;
    if (op == OP_TEST) return false;
    if (componentParameter->type != COND_ALARM_BASED) {
        return false;
    }
    fprintf(outputFile, "#define CONDITION_%d_TIME    %d\n", id, number);
    return true;
}

void generateConstants(void)
{
    bool any = false;
    foreach(ObjectCollection, objectsUsed,
            if ((*it)->generateConstants()) any = true);
    if (any) fprintf(outputFile, "\n");

    foreach(ConditionCollection, allConditions,
            if ((*it)->generateConstants()) any = true);
    if (any) fprintf(outputFile, "\n");

    // bool any;

    // // xxx_NO_VALUE constants
    // any = false;
    // foreach(vector<Sensor *>, sensorsUsed,
    //         any = true;
    //         fprintf(outputFile, "#define %s_NO_VALUE    %s\n",
    //                 toUpperCase((*it)->getName()).c_str(), uintTypes[(*it)->getDataSize()].noValue));
    // if (any) fprintf(outputFile, "\n");

    // xxx_PERIOD constants
    // any = false;
    // foreach(ObjectCollection, objectsUsed,
    //         if (IS_SPECIFIED((*it)->period)) {
    //             any = true;
    //             fprintf(outputFile, "#define %s_PERIOD    %d\n",
    //                     toUpperCase((*it)->getName()).c_str(), (*it)->period);
    //         });
    // if (any) fprintf(outputFile, "\n");

    // xxx_ON/OFF constants
    // any = false;
    // foreach(vector<Actuator *>, actuatorsUsed,
    //         if (IS_SPECIFIED((*it)->onTime)) {
    //             any = true;
    //             fprintf(outputFile, "#define %s_ON_TIME   %d\n",
    //                     toUpperCase((*it)->getName()).c_str(), (*it)->onTime);
    //         }
    //         if (IS_SPECIFIED((*it)->offTime)) {
    //             any = true;
    //             fprintf(outputFile, "#define %s_OFF_TIME  %d\n",
    //                     toUpperCase((*it)->getName()).c_str(), (*it)->offTime);
    //         });
    // if (any) fprintf(outputFile, "\n");

    // xxx_NUM_FIELDS constants
    // any = false;
    // foreach(vector<Sink *>, sinksUsed,
    //         if (!(*it)->usedFields.empty()) {
    //             any = true;
    //             fprintf(outputFile, "#define %s_PACKET_NUM_FIELDS    %d\n",
    //                     toUpperCase((*it)->getName()).c_str(), (*it)->usedFields.size());
    //         });
    // if (any) fprintf(outputFile, "\n");
}

bool Object::generateVariables(const UseCase &uc, int num)
{
    char condStr[20] = "";
    if (uc.condition) {
        sprintf(condStr, "Condition%d", uc.condition->id);
    }

    bool ret = false;

    if (IS_SPECIFIED(uc.period)) {
        ret = true;
        fprintf(outputFile, "Alarm_t %s%sAlarm;\n",
                toCamelCase(getName()).c_str(), condStr);
    }
    if (IS_SPECIFIED(uc.onTime)) {
        ret = true;
        fprintf(outputFile, "Alarm_t %s%sOnAlarm;\n",
                toCamelCase(getName()).c_str(), condStr);
    }
    if (IS_SPECIFIED(uc.offTime)) {
        ret = true;
        fprintf(outputFile, "Alarm_t %s%sOffAlarm;\n",
                toCamelCase(getName()).c_str(), condStr);
    }
    return ret;
}

bool Object::generateVariables()
{
    int i = 1;
    foreach (CaseVector, useCases,
            if (generateVariables(*it, i)) i++);
    return i > 1;
}

bool Condition::generateVariables()
{
    if (numberSpecified
            && op == OP_TEST
            && componentParameter->type == COND_ALARM_BASED) {
        fprintf(outputFile, "Alarm_t condition%dAlarm;\n", id);
        generatedAlarm = true;
    }
    fprintf(outputFile, "bool condition%dVariable;\n", id);
    return true;
}

void generateVariables(void)
{
    bool any;

    // packets
    any = false;
    foreach(vector<Sink *>, sinksUsed,
            if (!(*it)->usedFields.empty()) {
                any = true;
                fprintf(outputFile, "%sPacket_t %sPacket;\n",
                        (*it)->getName(),
                        toCamelCase((*it)->getName()).c_str());
                fprintf(outputFile, "uint16_t %sPacketNumFieldsFull;\n",
                        toCamelCase((*it)->getName()).c_str());
            });
    if (any) fprintf(outputFile, "\n");

    // alarms
    any = false;
    foreach(ObjectCollection, objectsUsed,
            if ((*it)->generateVariables()) any = true);
    if (any) fprintf(outputFile, "\n");

    foreach(ConditionCollection, allConditions,
            if ((*it)->generateVariables()) any = true);
    if (any) fprintf(outputFile, "\n");
}

void generateSerialFunction(const IntTypes &type) {
    fprintf(outputFile, "static inline void serialPrint%s(const char *name, %s value)\n",
            type.name2, type.name);
    fprintf(outputFile, "{\n");
    fprintf(outputFile, "    PRINTF(\"%%s=%%u\\n\", name, value);\n");
    fprintf(outputFile, "}\n\n");
}

void generateSerialFunctions(bool useI8, bool useU8, bool useI16, bool useU16, bool useI32, bool useU32)
{
    if (useI8) generateSerialFunction(intTypes[1]);
    if (useU8) generateSerialFunction(uintTypes[1]);
    if (useI16) generateSerialFunction(intTypes[2]);
    if (useU16) generateSerialFunction(uintTypes[2]);
    if (useI32) generateSerialFunction(intTypes[4]);
    if (useU32) generateSerialFunction(uintTypes[4]);
}

void SerialSink::generateCode()
{
    bool useU8, useU16, useU32;
    useU8 = useU16 = useU32 = false;

    if (aggregate) {
        foreach (vector<PacketField>, usedFields,
                if (it->size == 1) {
                    useU8 = true;
                } else if (it->size == 2) {
                    useU16 = true;
                } else if (it->size == 4) {
                    useU32 = true;
                });
    } else {
        foreach(vector<Sensor *>, sensorsUsed,
                if ((*it)->getDataSize() == 1) {
                    useU8 = true;
                } else if ((*it)->getDataSize() == 2) {
                    useU16 = true;
                } else if ((*it)->getDataSize() == 4) {
                    useU32 = true;
                });
    }
    generateSerialFunctions(false, useU8, false, useU16, false, useU32);

    if (!aggregate) return;

    fprintf(outputFile, "static inline void serialPacketPrint(void)\n");
    fprintf(outputFile, "{\n");
    fprintf(outputFile, "    PRINT(\"======================\\n\");\n");
    foreach (vector<PacketField>, usedFields,
            fprintf(outputFile, "    serialPrint%s(\"%s\", serialPacket.%s);\n",
                    uintTypes[it->size].name2,
                    it->name.c_str(), toCamelCase(it->name).c_str()));
    fprintf(outputFile, "}\n\n");

    Sink::generateCode();
}

void Sink::generateCode()
{
    string ucName = toUpperCase(getName());
    string ccName = toCamelCase(getName());

    fprintf(outputFile, "static inline void %sPacketInit(void)\n", ccName.c_str());
    fprintf(outputFile, "{\n");
    // count const fields
    unsigned numConstFields = 0;
    foreach (vector<PacketField>, usedFields,
            if (it->isConstant) numConstFields++);
    fprintf(outputFile, "    %sPacketNumFieldsFull = %d;\n",
            ccName.c_str(), numConstFields);
    foreach (vector<PacketField>, usedFields,
            if (it->isConstant) {
                fprintf(outputFile, "    %sPacket.%s = %d;\n",
                        ccName.c_str(), toCamelCase(it->name).c_str(),
                        it->value);
            } else {
                fprintf(outputFile, "    %sPacket.%s = %s_NO_VALUE;\n",
                        ccName.c_str(), toCamelCase(it->name).c_str(),
                        toUpperCase(it->name).c_str());
            });
    fprintf(outputFile, "}\n\n");

    fprintf(outputFile, "static inline void %sPacketSend(void)\n", ccName.c_str());
    fprintf(outputFile, "{\n");
    if (crc) {
            fprintf(outputFile, "    %sPacket.crc = crc16((const uint8_t *) &%sPacket, sizeof(%sPacket) - 2);\n",
                    ccName.c_str(), ccName.c_str(), ccName.c_str());
    }
    fprintf(outputFile, "    %s;\n", getSendFuntion());
    fprintf(outputFile, "    %sPacketInit();\n", ccName.c_str());
    fprintf(outputFile, "}\n\n");

    fprintf(outputFile, "static inline bool %sPacketIsFull(void)\n", ccName.c_str());
    fprintf(outputFile, "{\n");
    fprintf(outputFile, "    return %sPacketNumFieldsFull >= %s_PACKET_NUM_FIELDS;\n",
            ccName.c_str(), ucName.c_str());
    fprintf(outputFile, "}\n\n");
}

void Sink::generateCodeForSensor(Sensor *s)
{
    string ccName = toCamelCase(getName());
    string ucSensorName = toUpperCase(s->getName());
    string ccSensorName = toCamelCase(s->getName());

    if (!aggregate) {
        // this must be serial, because all other sinks require packets!
        fprintf(outputFile, "    %sPrint%s(\"%s\", %s);\n",
                ccName.c_str(),
                uintTypes[s->getDataSize()].name2,
                ccSensorName.c_str(),
                ccSensorName.c_str());
        return;
    }

    // a packet; more complex case
#if ALLOW_INCOMPLETE_PACKETS
    fprintf(outputFile, "    if (%sPacket.%s != %s_NO_VALUE) {\n",
            ccName.c_str(), ccSensorName.c_str(), ucSensorName.c_str());
    fprintf(outputFile, "        %sPacketSend();\n", ccName.c_str());
#else
    fprintf(outputFile, "    if (%sPacket.%s == %s_NO_VALUE) {\n",
            ccName.c_str(), ccSensorName.c_str(), ucSensorName.c_str());
    fprintf(outputFile, "        %sPacketNumFieldsFull++;\n", ccName.c_str());
#endif
    fprintf(outputFile, "    }\n");

    fprintf(outputFile, "    %sPacket.%s = %s;\n",
            ccName.c_str(), ccSensorName.c_str(), ccSensorName.c_str());
#if ALLOW_INCOMPLETE_PACKETS
    fprintf(outputFile, "    %sPacketNumFieldsFull++;\n", ccName.c_str());
#endif

    fprintf(outputFile, "    if (%sPacketIsFull()) {\n", ccName.c_str());
    fprintf(outputFile, "        %sPacketSend();\n", ccName.c_str());
    fprintf(outputFile, "    }\n\n");
}

void Object::generateCode(const UseCase &uc, int num)
{
    if (uc.condition) {
//        fprintf(outputFile, "    %s if (condition%dVariable) { \n",
//                myType.name, ccName.c_str(), asSensor->getReadFunction());
    } else {
        // TODO: default condition (but must be last!)

    }
}

// used for actuators and sensors
void Object::generateCode()
{
//    if (IS_UNSPECIFIED(period)) return;

    string ucName = toUpperCase(getName());
    string ccName = toCamelCase(getName());

    fprintf(outputFile, "void %sAlarmCallback(void *param)\n", ccName.c_str());
    fprintf(outputFile, "{\n");
    if (type == SENSOR) {
        Sensor *asSensor = (Sensor *) this;
        IntTypes &myType = uintTypes[asSensor->getDataSize()];
        fprintf(outputFile, "    %s %s = %s();\n",
                myType.name, ccName.c_str(), asSensor->getReadFunction());
        foreach(vector<Sink *>, sinksUsed,
                (*it)->generateCodeForSensor(asSensor));
    } else {
        Actuator *asActuator = (Actuator *) this;
        fprintf(outputFile, "    %s();\n",
                asActuator->getToggleFunction());
    }

    int i = 0;
    foreach (CaseVector, useCases,
            generateCode(*it, i++));
    // TODO: generate "else" code in case there is no default condition!

    fprintf(outputFile, "    alarmSchedule(&%sAlarm, %s_PERIOD);\n",
            ccName.c_str(), ucName.c_str());
    fprintf(outputFile, "}\n\n");
}

bool Condition::generateAppMainCode()
{
    return false;
}

bool Object::generateAppMainCode(const UseCase &uc, int num)
{
    char condStr1[20] = "";
    char condStr2[20] = "";
    if (uc.condition) {
        sprintf(condStr1, "Condition%d", uc.condition->id);
        sprintf(condStr2, "_CONDITION%d", uc.condition->id);
    }

    bool ret = false;

    if (IS_SPECIFIED(uc.period)) {
        ret = true;
        fprintf(outputFile, "    alarmInit(&%s%sAlarm, %s%sAlarmCallback, NULL);\n",
                toCamelCase(getName()).c_str(), condStr1,
                toCamelCase(getName()).c_str(), condStr1);
        fprintf(outputFile, "    alarmSchedule(&%s%sAlarm, %s%s_PERIOD);\n",
                toCamelCase(getName()).c_str(), condStr1,
                toUpperCase(getName()).c_str(), condStr2);
    }
    if (IS_SPECIFIED(uc.onTime)) {
        ret = true;
        fprintf(outputFile, "    alarmInit(&%s%sOnAlarm, %s%sOnAlarmCallback, NULL);\n",
                toCamelCase(getName()).c_str(), condStr1,
                toCamelCase(getName()).c_str(), condStr1);
        fprintf(outputFile, "    alarmSchedule(&%s%sOnAlarm, %s%s_ON_TIME);\n",
                toCamelCase(getName()).c_str(), condStr1,
                toUpperCase(getName()).c_str(), condStr2);
    }
    if (IS_SPECIFIED(uc.offTime)) {
        ret = true;
        fprintf(outputFile, "    alarmInit(&%s%sOffAlarm, %s%sOffAlarmCallback, NULL);\n",
                toCamelCase(getName()).c_str(), condStr1,
                toCamelCase(getName()).c_str(), condStr1);
        fprintf(outputFile, "    alarmSchedule(&%s%sOffAlarm, %s%s_OFF_TIME);\n",
                toCamelCase(getName()).c_str(), condStr1,
                toUpperCase(getName()).c_str(), condStr2);
    }
    return ret;
}

bool Object::generateAppMainCode()
{
    bool any = false;
    const char *ifunc = getInitFunction();
    if (ifunc) {
        any = true;
        fprintf(outputFile, "    %s();\n", ifunc);
    }

    foreach (CaseVector, useCases,
            if (generateAppMainCode(*it, 1)) any = true);
    return any;
}

// ----------------------------------------

void generateSinkCode(void)
{
    foreach(vector<Sink *>, sinksUsed,
            (*it)->generateCode());
}

void generateCallbacks(void)
{
    foreach(vector<Sensor *>, sensorsUsed,
            (*it)->generateCode());
    foreach(vector<Actuator *>, actuatorsUsed,
            (*it)->generateCode());
}

void generateAppMain(void)
{
    bool any;

    fprintf(outputFile, "void appMain(void)\n");
    fprintf(outputFile, "{\n");

    // packet initialization code
    any = false;
    foreach(vector<Sink *>, sinksUsed,
            if ((*it)->aggregate) {
                any = true;
                fprintf(outputFile, "    %sPacketInit();\n",
                        toCamelCase((*it)->getName()).c_str());
            });
    if (any) fprintf(outputFile, "\n");

    any = false;
    foreach(ObjectCollection, objectsUsed,
            if ((*it)->generateAppMainCode()) any = true);
    if (any) fprintf(outputFile, "\n");

    any = false;
    foreach(ConditionCollection, allConditions,
            if ((*it)->generateAppMainCode()) any = true);
    if (any) fprintf(outputFile, "\n");

    fprintf(outputFile, "}\n");
}

void generateCode(void)
{
    string outputFileFullName;
    if (outputDir) {
        outputFileFullName += outputDir;
        outputFileFullName += '/';
    }
    outputFileFullName += outputFileName;
    outputFile = fopen(outputFileFullName.c_str(), "w");
    if (!outputFile) sysError("failed to write output file %s", outputFileFullName.c_str());

    foreach(ObjectCollection, objectsUsed,
            switch ((*it)->type) {
            case Object::ACTUATOR:
                actuatorsUsed.push_back((Actuator *) *it);
                break;
            case Object::SENSOR:
                sensorsUsed.push_back((Sensor *) *it);
                break;
            case Object::SINK:
                sinksUsed.push_back((Sink *) *it);
                break;
            });

    generateIncludes();
    fprintf(outputFile, SEPARATOR);
    generateTypes();
    generateVariables();
    generateConstants();
    fprintf(outputFile, SEPARATOR);
    generateSinkCode();
    fprintf(outputFile, SEPARATOR);
    generateCallbacks();
    fprintf(outputFile, SEPARATOR);
    generateAppMain();

    fclose(outputFile);
}
