from components import *

class ConditionCollection(object):
    def reset(self):
        self.conditionStack = []
        self.conditionList = []
        self.branchNumber = 0
        self.branchChanged = False

    def add(self, condition):
        self.conditionList.append(condition)
        currentConditionNumber = len(self.conditionList)
        self.conditionStack.append(currentConditionNumber)
        self.branchChanged = True

    def invertLast(self):
        last = self.conditionStack.pop()
        self.conditionStack.append(-last)
        self.branchChanged = True

    def size(self):
        return len(self.conditionStack)

    def totalConditions(self):
        return len(self.conditionList)

    def pop(self, n):
        while n > 0:
            self.conditionStack.pop()
            n -= 1

    def generateCode(self, outputFile):
        for i in range(len(self.conditionList)):
            self.generateCodeForCondition(i, outputFile)

    def generateCodeForCondition(self, i, outputFile):
        outputFile.write("bool condition{0}Check(void) {1}\n".format(i + 1, '{'))
        outputFile.write("    return {0};\n".format(
                string.replace(string.replace(string.replace(self.conditionList[i].getCode(), " and ", " && "),
                               " or ", " || "),
                               " not ", " ! ")))
                
        outputFile.write("}\n")

conditionCollection =  ConditionCollection()

########################################################

INDENT_STRING = "  "  # indent new code level with two spaces

def getIndent(indent):
    # take indent string n times
    return INDENT_STRING * indent

########################################################
class Value(object):
    def __init__(self, value=None, suffix=None):
        self.value = value
        self.suffix = suffix
    def getCode(self):
        s = "{0}".format(self.value)
        if not (self.suffix is None):
            s += self.suffix
        return s

########################################################
class Expression(object):
    def __init__(self, left=None, op=None, right=None):
        self.left = left
        self.op = op
        self.right = right

    def getCode(self):
        if self.left != None and self.right != None:
            return self.left.getCode() + " " + self.op + " " + self.right.getCode()
        if self.op != None:
            return self.op + " " + self.right.getCode()
        if type(self.right) is Expression:
            return "(" + self.right.getCode() + ")"
        return self.right

########################################################
class ComponentUseCase(object):
    def __init__(self, type, name, parameters):
        self.type = type.lower()
        self.name = name.lower()
        self.parameters = parameters

    def getCode(self, indent):
        result = self.type + " " + self.name
        for p in self.parameters:
            result += " "
            result += p[0]
            if p[1] != None:
                result += " "
                result += p[1].getCode()
            result += ","
        if self.parameters != []:
            result = result[:-1] # remove last comma
        result += ';'
        return result

    def addComponents(self):
        if conditionCollection.branchChanged:
            conditionCollection.branchNumber += 1
            conditionCollection.branchChanged = False
        componentRegister.useComponent(self.type, self.name,
                                       self.parameters,
                                       conditionCollection.conditionStack,
                                       conditionCollection.branchNumber)

########################################################
CODE_BlOCK_TYPE_PROGRAM = 0
CODE_BlOCK_TYPE_WHEN = 1
CODE_BlOCK_TYPE_ELSEWHEN = 2
CODE_BlOCK_TYPE_ELSE = 3

class CodeBlock(object):
    def __init__(self, blockType, condition, declarations, next_):
        self.blockType = blockType
        self.condition = condition
        self.declarations = declarations
        self.next = next_

    def getCode(self, indent):
        result = getIndent(indent)
        if self.blockType == CODE_BlOCK_TYPE_WHEN:
            result += "when"
            result += " " + self.condition.getCode()
            result += ":\n"
            indent += 1
        elif self.blockType == CODE_BlOCK_TYPE_ELSEWHEN:
            result += "elsewhen"
            result += " " + self.condition.getCode()
            result += ":\n"
            indent += 1
        elif self.blockType == CODE_BlOCK_TYPE_ELSE:
            result += "else:\n"
            indent += 1
 
        for d in self.declarations:
            result += getIndent(indent)
            result += d.getCode(indent)
            result += "\n"

        # recursive call
        if self.next != None:
            assert indent > 0
            indent -= 1
            result += getIndent(indent)
            result += self.next.getCode(indent)
            result += "\n"
        elif self.blockType != CODE_BlOCK_TYPE_PROGRAM:
            result += "end\n"

        return result

    def addComponents(self):
        if self.blockType == CODE_BlOCK_TYPE_PROGRAM:
            # reset all
            conditionCollection.reset()
        elif self.blockType == CODE_BlOCK_TYPE_WHEN:
            stackStartSize = conditionCollection.size()
            # append new
            conditionCollection.add(self.condition)
        elif self.blockType == CODE_BlOCK_TYPE_ELSEWHEN:
            # invert previous...
            conditionCollection.invertLast()
            # ..and append new
            conditionCollection.add(self.condition)
        elif self.blockType == CODE_BlOCK_TYPE_ELSE:
            # just invert previous
            conditionCollection.invertLast()

        # add components - use cases always first, "when ..." blocks afterwards!
        for d in self.declarations:
            if type(d) is ComponentUseCase:
                d.addComponents()
        for d in self.declarations:
            if type(d) is CodeBlock:
                d.addComponents()

        # recursive call
        if self.next != None:
            self.next.addComponents()

        if self.blockType == CODE_BlOCK_TYPE_WHEN:
            # pop all conditions from this code block
            conditionCollection.pop(conditionCollection.size() - stackStartSize)
