from seal_parser import *
import os

SEPARATOR = "// -----------------------------\n"

def formatCondition(condition, isNew):
    if isNew:
        prefix = "new"
    else:
        prefix = "old"
    return prefix + "ConditionStatus[{0}]".format(abs(condition))

def formatConditions(conditions, isNew):
    result = ""
    # all except last one
    for c in conditions[:-1]:
        if c < 0:
            result += "!" # not
        result += formatCondition(c, isNew)
        result += " && "  # and
    # last one
    if conditions[-1] < 0:
        result += "!" # not
    result += formatCondition(conditions[-1], isNew)
    # got it
    return result

###############################################
class Generator(object):
    def generateIncludes(self):
        for c in self.components:
            c.generateIncludes(self.outputFile)
        for x in components.processFunctionsUsed:
            self.outputFile.write('#include "{}.h"\n'.format(x))
        if components.componentRegister.numCachedSensors:
            self.outputFile.write("#include <lib/processing/cache.h>\n")
        self.outputFile.write("\n")

    def generateConstants(self):
        self.outputFile.write("#define NUM_CONDITIONS {0}\n".format(
                components.conditionCollection.totalConditions()))
        self.outputFile.write("#define DEFAULT_CONDITION 0\n")
        self.outputFile.write("\n")
        for c in self.components:
            c.generateConstants(self.outputFile)
            # self.outputFile.write("\n")

    def cacheTypes(self):
        # use packet types iff there is least one aggregated output
        usePackets = reduce(lambda x, y: x or y.isAggregate(), self.outputs, False)
        if not usePackets: return

        # sensors are used iff there is at least one use case declared
        #anySensorsUsed = reduce(lambda x, y: x or (type(y) is Sensor and len(y.useCases)), self.components, False)
        #if not anySensorsUsed: return
        if len(self.sensors) == 0: return

        packetFields = []
        for c in self.sensors:
            packetFields.append((c.getDataSize(), c.name))
        packetFields = list(set(packetFields)) # uniquify
        packetFields.sort()
        packetFields.reverse()

        for c in self.outputs:
            c.cachePacketType(packetFields)

    def generateTypes(self):
       for c in self.outputs:
           c.generatePacketType(self.outputFile)

    def generateVariables(self):
        self.outputFile.write("bool oldConditionStatus[NUM_CONDITIONS + 1];\n")
        components.componentRegister.generateVariables(self.outputFile)
        for c in self.components:
            c.generateVariables(self.outputFile)

    def generateOutputCode(self):
        for c in self.outputs:
            c.generateOutputCode(self.outputFile, self.sensors)

    def generateCallbacks(self):
        for c in self.components:
            c.generateCallbacks(self.outputFile, self.outputs)

    def generateBranchCode(self):
        components.branchCollection.generateCode(self.outputFile)

    def generateAppMain(self):
        self.outputFile.write("void appMain(void)\n")
        self.outputFile.write("{\n")
        for c in self.components:
            c.generateAppMainCode(self.outputFile)
        # Init Process variables
#        for p in components.processStructInits:
#            self.outputFile.write(p)

        self.outputFile.write("\n\n")
        self.outputFile.write("    for (;;) {\n")
        self.outputFile.write("        uint32_t iterationEndTime = getRealTime() + 1000;\n")
        self.outputFile.write("\n")

        totalConditions = components.conditionCollection.totalConditions()
        self.outputFile.write("        bool newConditionStatus[NUM_CONDITIONS + 1];\n")
        self.outputFile.write("        newConditionStatus[DEFAULT_CONDITION] = true;\n")
        for i in range(totalConditions):
            self.outputFile.write("        newConditionStatus[{0}] = condition{0}Check();\n".format(i + 1))
        self.outputFile.write("\n")

        self.outputFile.write("        bool branch0OldStatus = oldConditionStatus[DEFAULT_CONDITION];\n")
        for i in range(1, components.branchCollection.getNumBranches()):
            conditions = components.branchCollection.getConditions(i)
            self.outputFile.write("        bool branch{0}OldStatus = {1};\n".format(i, formatConditions(conditions, False)))
        self.outputFile.write("\n")

        self.outputFile.write("        bool branch0NewStatus = newConditionStatus[DEFAULT_CONDITION];\n")
        for i in range(1, components.branchCollection.getNumBranches()):
            conditions = components.branchCollection.getConditions(i)
            self.outputFile.write("        bool branch{0}NewStatus = {1};\n".format(i, formatConditions(conditions, True)))

        for i in range(components.branchCollection.getNumBranches()):
            s = '''
        if (branch{0}OldStatus != branch{0}NewStatus) {1}
            if (branch{0}NewStatus) branch{0}Start();
            else branch{0}Stop();
        {2}\n'''
            self.outputFile.write(s.format(i, '{', '}'))

        self.outputFile.write('''
        memcpy(oldConditionStatus, newConditionStatus, sizeof(oldConditionStatus));

        uint32_t now = getRealTime();
        if (timeAfter32(iterationEndTime, now)) {
            msleep(iterationEndTime - now);
        }\n''')
        self.outputFile.write("    }\n")
        self.outputFile.write("}\n")

    def generate(self, outputFile):
        self.outputFile = outputFile
        self.components = components.componentRegister.getAllComponents()
        self.outputs = []
        self.sensors = []
        self.actuators = []
        for c in self.components:
            if type(c) is components.Output and len(c.useCases):
                self.outputs.append(c)
            elif type(c) is components.Sensor and len(c.useCases):
                self.sensors.append(c)
            elif type(c) is components.Actuator and len(c.useCases):
                self.actuators.append(c)
        self.cacheTypes()

        # find out the sensors that should be cached
        components.componentRegister.markCachedSensors()
        # generate condition code now, for later use
        components.conditionCollection.generateCode(components.componentRegister)

        self.generateIncludes()
        outputFile.write(SEPARATOR)
        outputFile.write("// Constants\n\n")
        self.generateConstants()
        outputFile.write(SEPARATOR)
        outputFile.write("// Types, variables\n\n")
        self.generateTypes()
        self.generateVariables()
        outputFile.write(SEPARATOR)
        outputFile.write("// Outputs\n\n")
        self.generateOutputCode()
        outputFile.write(SEPARATOR)
        outputFile.write("// Callbacks\n\n")
        self.generateCallbacks()
        outputFile.write(SEPARATOR)
        outputFile.write("// Conditions\n\n")
        components.conditionCollection.writeOutCode(self.outputFile)
        outputFile.write(SEPARATOR)
        outputFile.write("// Branches\n\n")
        self.generateBranchCode()
        outputFile.write(SEPARATOR)
        outputFile.write("// Main function\n\n")
        self.generateAppMain()

    def generateConfigFile(self, outputFile):
        for c in self.components:
            c.generateConfig(outputFile)
        for x in components.processFunctionsUsed:
            outputFile.write("USE_{}=y\n".format(x.upper()))
        for x in components.componentRegister.systemParams:
            outputFile.write(x.getConfigLine())
            outputFile.write("\n")
        if components.componentRegister.numCachedSensors > 0:
            outputFile.write("USE_CACHE=y\n")
            outputFile.write("CONST_TOTAL_CACHEABLE_SENSORS={}\n".format(
                    components.componentRegister.numCachedSensors))

    def generateMakefile(self, outputFile, outputFileName, pathToOS):
        outputFile.write('''
SOURCES = {0}
APPMOD = App
PROJDIR = $(CURDIR)
ifndef MOSROOT
  MOSROOT = $(PROJDIR)/../{1}
endif
'''.format(os.path.basename(outputFileName), pathToOS))
        outputFile.write("include ${MOSROOT}/mos/make/Makefile")

###############################################
class MansOSGenerator(Generator):
    def __init__(self):
        super(Generator, self).__init__()
    def generateIncludes(self):
        self.outputFile.write("#include <stdmansos.h>\n")
        self.outputFile.write("#include <string.h>\n")
        self.outputFile.write("#include <lib/codec/crc.h>\n")
        super(MansOSGenerator, self).generateIncludes()

###############################################
class ContikiGenerator(Generator):
    def __init__(self):
        super(Generator, self).__init__()
    def generateIncludes(self):
        self.outputFile.write("#include \"contiki.h\"\n")
        self.outputFile.write("#include \"lib/crc16.h\"\n")
        self.outputFile.write("#include <stdbool.h>\n")
        super(ContikiGenerator, self).generateIncludes()
        self.outputFile.write("#define crc16(d, l) crc16_data(d, l, 0)\n")
        self.outputFile.write("\n")

###############################################
def createGenerator(targetOS):
    if targetOS == "mansos":
        return MansOSGenerator()
    if targetOS == "contiki":
        return ContikiGenerator()
    return None
