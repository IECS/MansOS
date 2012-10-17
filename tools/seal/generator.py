#
# Copyright (c) 2012 Atis Elsts
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#  * Redistributions of source code must retain the above copyright notice,
#    this list of  conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

from seal_parser import *
import os

SEPARATOR = "// -----------------------------\n"

def formatCondition(condition, isNew):
    if isNew:
        prefix = "new"
    else:
        prefix = "old"
    return prefix + "ConditionStatus[{0}]".format(abs(condition) - 1)

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
        if components.componentRegister.numCachedSensors:
            self.outputFile.write("#include <lib/processing/cache.h>\n")
        self.outputFile.write("#include <net/seal_comm.h>\n")
        # XXX: only when network is used
        self.outputFile.write("#include <net/socket.h>\n")
        # XXX: for getFixedTime() and getFixedUptime()
        self.outputFile.write("#include <net/routing.h>\n")
        self.outputFile.write("\n")

    def generateConstants(self):
        # main loop is executed once in second by default
        self.outputFile.write("#ifndef CONDITION_EVALUATION_INTERVAL\n")
        self.outputFile.write("#define CONDITION_EVALUATION_INTERVAL  100\n") # ms
        self.outputFile.write("#endif\n\n")

        self.outputFile.write("#define NUM_CONDITIONS {0}\n".format(
                components.conditionCollection.totalConditions()))
#        self.outputFile.write("#define DEFAULT_CONDITION 0\n\n")

        self.outputFile.write("#define IS_FROM_BRANCH_START ((void *) 1)\n\n")

        components.componentRegister.prepareToGenerateConstants()
        for c in self.components:
            c.generateConstants(self.outputFile)
            # self.outputFile.write("\n")

    def definePacketTypes(self):
        for n in self.networkComponents:
            n.sortFields()
        for o in self.outputs:
            o.definePacketType()

    def generateTypes(self):
        for o in self.outputs:
            o.generatePacketType(self.outputFile)
        for n in self.networkComponents:
            n.generatePacketType(self.outputFile)

    def generateVariables(self):
        self.outputFile.write("bool oldConditionStatus[NUM_CONDITIONS];\n")
        components.componentRegister.generateVariables(self.outputFile)
        for c in self.components:
            c.generateVariables(self.outputFile)
        for n in self.networkComponents:
            n.generateVariables(self.outputFile)

    def generateOutputCode(self):
        sensorsUsed = []
        for s in components.componentRegister.sensors.itervalues():
            if s.isUsed(): sensorsUsed.append(s)
        for c in self.outputs:
            c.generateOutputCode(self.outputFile, sensorsUsed)

    def generateCallbacks(self):
        for o in self.outputs:
            o.prepareToGenerateCallbacks(self.outputFile)

        for s in components.componentRegister.sensors.itervalues():
            s.generateCallbacks(self.outputFile, self.outputs)
        for a in components.componentRegister.actuators.itervalues():
            a.generateCallbacks(self.outputFile, self.outputs)
        for o in components.componentRegister.outputs.itervalues():
            o.generateCallbacks(self.outputFile, self.outputs)

        for n in self.networkComponents:
            n.generateReadFunctions(self.outputFile)

    def generateBranchCode(self):
        components.componentRegister.branchCollection.generateCode(self.outputFile)

    def generateAppMain(self):
        self.outputFile.write("void appMain(void)\n")
        self.outputFile.write("{\n")
        for c in self.components:
            c.generateAppMainCode(self.outputFile)
        components.conditionCollection.generateAppMainCode(self.outputFile)

        self.outputFile.write("\n")
        for i in range(1, components.componentRegister.branchCollection.getNumBranches()):
            conditions = components.componentRegister.branchCollection.getConditions(i)
            self.outputFile.write("    bool branch{0}OldStatus = false;\n".format(i))
        self.outputFile.write("\n")

        self.outputFile.write("    branch0Start();\n\n")

        self.outputFile.write("    for (;;) {\n")
        self.outputFile.write("        uint32_t iterationEndTime = getRealTime() + CONDITION_EVALUATION_INTERVAL;\n")
        self.outputFile.write("\n")

        totalConditions = components.conditionCollection.totalConditions()
        self.outputFile.write("        bool newConditionStatus[NUM_CONDITIONS];\n")
#        self.outputFile.write("        newConditionStatus[DEFAULT_CONDITION] = true;\n")
        for i in range(totalConditions):
            self.outputFile.write("        newConditionStatus[{0}] = condition{1}Check(oldConditionStatus[{0}]);\n".format(i, i + 1))
        self.outputFile.write("\n")

#        self.outputFile.write("        bool branch0NewStatus = newConditionStatus[DEFAULT_CONDITION];\n")
        for i in range(1, components.componentRegister.branchCollection.getNumBranches()):
            conditions = components.componentRegister.branchCollection.getConditions(i)
            self.outputFile.write("        bool branch{0}NewStatus = {1};\n".format(i, formatConditions(conditions, True)))

        for i in range(1, components.componentRegister.branchCollection.getNumBranches()):
            s = '''
        if (branch{0}OldStatus != branch{0}NewStatus) {1}
            if (branch{0}NewStatus) branch{0}Start();
            else branch{0}Stop();
        {2}\n'''
            self.outputFile.write(s.format(i, '{', '}'))

        self.outputFile.write("\n")
        self.outputFile.write("        memcpy(oldConditionStatus, newConditionStatus, sizeof(oldConditionStatus));\n")

        for i in range(1, components.componentRegister.branchCollection.getNumBranches()):
            self.outputFile.write("        branch{0}OldStatus = branch{0}NewStatus;\n".format(i))
        self.outputFile.write("\n")

        self.outputFile.write("        uint32_t now = getRealTime();\n")
        self.outputFile.write("        if (timeAfter32(iterationEndTime, now)) {\n")
        self.outputFile.write("            msleep(iterationEndTime - now);\n")
        self.outputFile.write("        }\n")
        self.outputFile.write("    }\n")
        self.outputFile.write("}\n")

    def generate(self, outputFile):
#        self.isError = False
        self.outputFile = outputFile

        # generate condition code now, for later use
        components.conditionCollection.generateCode(components.componentRegister)
        # find out the sensors that should be cached
        components.componentRegister.markCachedSensors()
        # find out the sensors that should synched
        components.componentRegister.markSyncSensors()

        self.components = components.componentRegister.getAllComponents()
        self.outputs = []
        for c in self.components:
            if type(c) is components.Output and len(c.useCases):
                self.outputs.append(c)
        self.networkComponents = components.componentRegister.networkComponents.values()
        # generate packet types now, for later use
        self.definePacketTypes()

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
        outputFile.write("// Branches\n\n")
        self.generateBranchCode()
        outputFile.write(SEPARATOR)
        outputFile.write("// Conditions\n\n")
        components.conditionCollection.writeOutCode(self.outputFile)
        outputFile.write(SEPARATOR)
        outputFile.write("// Main function\n\n")
        self.generateAppMain()

#        global isError
#        print "global isError: ", isError
#        if isError: self.isError = True

    def generateConfigFile(self, outputFile):
        config = set()
        # put all config in a set
        for c in self.components:
            cfg = c.getConfig()
            if cfg: config.add(cfg + '\n')
        for x in components.componentRegister.additionalConfig:
            if x.find('=') != -1:
                config.add("{}\n".format(x.upper()))
            else:
                config.add("USE_{}=y\n".format(x.upper()))
        for x in components.componentRegister.systemParams:
            config.add(x.getConfigLine() + "\n")
        if components.componentRegister.numCachedSensors > 0:
            config.add("USE_CACHE=y\n")
        # print the set to the file
        for line in config:
            outputFile.write(line)
        # check if cache is used
        if components.componentRegister.numCachedSensors > 0:
            outputFile.write("CONST_TOTAL_CACHEABLE_SENSORS={}\n".format(
                    components.componentRegister.numCachedSensors))

    def generateMakefile(self, outputFile, outputFileName, pathToOS):
        sources = os.path.basename(outputFileName)
        for s in components.componentRegister.extraSourceFiles:
            sources += " " + s
        outputFile.write('''
SOURCES = {0}
APPMOD = App
PROJDIR = $(CURDIR)
ifndef MOSROOT
  MOSROOT = {1}
endif
'''.format(sources, pathToOS))
        outputFile.write("include ${MOSROOT}/mos/make/Makefile")

    def isComponentUsed(self, componentName):
        return components.componentRegister.isComponentUsed(componentName)

    def generateBaseStationCode(self, path, pathToOS):
        # print "generateBaseStationCode @", path
        try:
            os.makedirs(path)
        except Exception:
            pass
        with open(os.path.join(path, 'Makefile'), 'w') as outputFile:
            self.generateMakefile(outputFile, "main.c", pathToOS)

        with open(os.path.join(path, 'config'), 'w') as outputFile:
            for x in components.componentRegister.systemParams:
                outputFile.write(x.getConfigLine() + "\n")
            outputFile.write("USE_SEAL_COMM=y\n")
            outputFile.write("USE_ROLE_BASE_STATION=y\n")
            c = components.componentRegister.findComponentByName("network")
            if c: outputFile.write(c.getConfig())

        with open(os.path.join(path, 'main.c'), 'w') as outputFile:
            # TODO: named values!
            outputFile.write("#include <stdmansos.h>\n")
            outputFile.write("#include <net/seal_comm.h>\n")
            outputFile.write("#include <net/socket.h>\n")
            outputFile.write("\n")
            outputFile.write("const char *sensorNames[32] = {\n")
            for (name, id) in components.componentRegister.allSensorNames.iteritems():
                outputFile.write('    [{}] = "{}",\n'.format(id, name))
            outputFile.write("};\n")
            outputFile.write("\n")
            outputFile.write("void valueRxCallback(uint16_t code, int32_t value)\n")
            outputFile.write("{\n")
            outputFile.write('    PRINTF("  %s=%ld\\n", sensorNames[code], value);\n')
            outputFile.write("}\n")
            outputFile.write("\n")
            outputFile.write("void appMain(void)\n")
            outputFile.write("{\n")
            outputFile.write("    uint16_t i;\n")
            outputFile.write("    for (i = 0; i < 31; ++i) {\n")
            outputFile.write("        sealCommRegisterInterest(i, valueRxCallback);\n")
            outputFile.write("    }\n")
            outputFile.write("}\n")

    def generateForwarderCode(self, path, pathToOS):
        # print "generateForwarderCode @", path
        try:
            os.makedirs(path)
        except Exception:
            pass
        with open(os.path.join(path, 'Makefile'), 'w') as outputFile:
            self.generateMakefile(outputFile, "main.c", pathToOS)

        with open(os.path.join(path, 'config'), 'w') as outputFile:
            for x in components.componentRegister.systemParams:
                outputFile.write(x.getConfigLine() + "\n")
            outputFile.write("USE_ROLE_FORWARDER=y\n")
            c = components.componentRegister.findComponentByName("network")
            if c: outputFile.write(c.getConfig())

        with open(os.path.join(path, 'main.c'), 'w') as outputFile:
            outputFile.write("#include <stdmansos.h>\n")
            outputFile.write("\n")
            outputFile.write("void appMain(void) {\n")
            outputFile.write("}\n")

    def generateCollectorCode(self, path, pathToOS):
        # print "generateCollectorCode @", path
        try:
            os.makedirs(path)
        except Exception:
            pass
        with open(os.path.join(path, 'Makefile'), 'w') as outputFile:
            self.generateMakefile(outputFile, "main.c", pathToOS)

        with open(os.path.join(path, 'config'), 'w') as outputFile:
            for x in components.componentRegister.systemParams:
                outputFile.write(x.getConfigLine() + "\n")
            outputFile.write("USE_ROLE_COLLECTOR=y\n")
            c = components.componentRegister.findComponentByName("network")
            if c: outputFile.write(c.getConfig())

        with open(os.path.join(path, 'main.c'), 'w') as outputFile:
            outputFile.write("#include <stdmansos.h>\n")
            outputFile.write("\n")
            outputFile.write("void appMain(void) {\n")
            outputFile.write("}\n")

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
