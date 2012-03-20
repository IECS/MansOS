from structures import *
from components import *

SEPARATOR = "// -----------------------------\n"

def formatCondition(condition, isNew):
    if isNew:
        return "newConditionStatus[{0}]".format(abs(condition))
    return "conditionStatus[{0}]".format(abs(condition))

def formatConditions(conditions, isNew):
    result = ""
    # all except last one
    for c in conditions[:-1]:
        if c < 0:
            result += "not "
        result += formatCondition(c, isNew)
        result += " "
    # last one
    if conditions[:-1] < 0:
        result += "not "
    result + formatCondition(conditions[-1], isNew)
    # got it
    return result

###############################################
class Generator(object):
    def generateIncludes(self):
        for c in self.components:
            pass # TODO
        self.outputFile.write("\n")

    def generateConstants(self):
        self.outputFile.write("#define NUM_CONDITIONS {0}\n".format(
                conditionCollection.totalConditions()))
        self.outputFile.write("#define DEFAULT_CONDITION 0\n")
        self.outputFile.write("\n")
        for c in self.components:
            c.generateConstants(self.outputFile)
            self.outputFile.write("\n")

    def generateTypes(self):
        pass # TODO

    def generateVariables(self):
        self.outputFile.write("bool conditionStatus[NUM_CONDITIONS + 1];\n")
        for c in self.components:
            c.generateVariables(self.outputFile)

    def generateOutputCode(self):
        pass # TODO

    def generateCallbacks(self):
        for c in self.components:
            c.generateCallbacks(self.outputFile)

    def generateConditionCode(self):
        conditionCollection.generateCode(self.outputFile)

    def generateBranchCode(self):
        branchCollection.generateCode(self.outputFile)

    def generateAppMain(self):
        self.outputFile.write("void appMain(void)\n")
        self.outputFile.write("{\n")
        for c in self.components:
            c.generateAppMainCode(self.outputFile)
        self.outputFile.write("\n\n")
        self.outputFile.write("    for (;;) {\n")
        self.outputFile.write("        uint32_t iterationEndTime = getRealTime() + 1000;\n")
        self.outputFile.write("\n")

        totalConditions = conditionCollection.totalConditions()
        self.outputFile.write("        bool newConditionStatus[NUM_CONDITIONS + 1];\n")
        self.outputFile.write("        newConditionStatus[DEFAULT_CONDITION] = true;\n")
        for i in range(totalConditions):
            self.outputFile.write("        newConditionStatus[{0}] = condition{0}Check();\n".format(i + 1))
        self.outputFile.write("\n")

        self.outputFile.write("        bool branch0OldStatus = conditionStatus[DEFAULT_CONDITION];\n")
        for i in range(branchCollection.getNumBranches()):
            if i == 0: continue
            conditions = branchCollection.getConditions(branchCollection.branches[i])
            self.outputFile.write("        bool branch{0}OldStatus = {1};\n".format(i, formatConditions(conditions, False)))

        self.outputFile.write("        bool branch0NewStatus = newConditionStatus[DEFAULT_CONDITION];\n")
        for i in range(branchCollection.getNumBranches()):
            if i == 0: continue
            conditions = branchCollection.getConditions(branchCollection.branches[i])
            self.outputFile.write("        bool branch{0}NewStatus = {1};\n".format(i, formatConditions(conditions, True)))

        for i in range(branchCollection.getNumBranches()):
            s = '''
        if (branch{0}OldStatus != branch{0}NewStatus) {1}
            if (branch{0}NewStatus) branch{0}Start();
            else branch{0}Stop();
        {2}\n'''
            self.outputFile.write(s.format(i, '{', '}'))

        self.outputFile.write('''
        memcpy(conditionStatus, newConditionStatus, sizeof(conditionStatus));

        uint32_t now = getRealTime();
        if (timeAfter32(iterationEndTime, now)) {
            msleep(iterationEndTime - now);
        }''')
        self.outputFile.write("    }\n")
        self.outputFile.write("}\n")

    def generate(self, outputFile):
        self.components = componentRegister.getAllComponents()
        self.outputFile = outputFile

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
        self.generateConditionCode()
        outputFile.write(SEPARATOR)
        outputFile.write("// Branches\n\n")
        self.generateBranchCode()
        outputFile.write(SEPARATOR)
        outputFile.write("// Main function\n\n")
        self.generateAppMain()

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
