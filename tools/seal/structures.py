import string, sys

########################################################

INDENT_STRING = "  "  # indent new code level with two spaces
def getIndent(indent):
    # take indent string n times
    return INDENT_STRING * indent

def userError(msg):
    # TODO JJ - redirect to IDE
    sys.stderr.write(msg)
    # TODO
    # parser.isError = True

def toMilliseconds(x):
    if type(x) is int: return x
    value = x.value
    if x.suffix is None or x.suffix == '' or x.suffix == "ms":
        pass
    elif x.suffix == "s" or x.suffix == "sec":
        value *= 1000
    else:
        userError("Unknown suffix {0} for time value\n".format(x.suffix))
    return value

def toCamelCase(s):
    if s == '': return ''
    return string.lower(s[0]) + s[1:]

########################################################
class ConditionCollection(object):
    def __init__(self):
        self.reset()

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

########################################################
class Value(object):
    def __init__(self, value=None, suffix=None):
        self.value = value
        self.suffix = suffix
    def getCode(self):
        if self.value is None: return None
        s = "{0}".format(self.value)
        if not (self.suffix is None):
            s += self.suffix
        return s
    def getType(self):
        if type(self.value) is bool:
            return "bool"
        return "int_t"
    def asString(self):
        if type(self.value) is bool:
            return str(self.value).lower()
        # TODO: convert time values to milliseconds?
        return str(self.value)

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
class SystemParameter(object):
    def __init__(self, name, value):
        self.name = name.lower()
        self.value = value

    def addComponents(self, componentRegister, conditionCollection):
        if self.name in componentRegister.systemParams:
            userError("Parameter {0} already specified, ignoring\n".format(self.name))
            return
        componentRegister.systemParams[self.name] = self

    def getCode(self, indent):
        return "parameter " + self.name + " " + self.value.getCode() + ';'

########################################################
class PatternDeclaration(object):
    def __init__(self, name, values):
        self.name = name.lower()
        self.values = values

    def addComponents(self, componentRegister, conditionCollection):
        if self.name in componentRegister.patterns:
            userError("Pattern {0} already specified, ignoring\n".format(self.name))
            return
        componentRegister.patterns[self.name] = self

    def getCode(self, indent):
        result = "pattern " + self.name + " ["
        assert len(self.values)
        for v in self.values[:-1]:
            result += v.getCode() + ', '
        result += self.values[-1].getCode()
        result += '];'
        return result

    def generateVariables(self, outputFile):
        outputFile.write("int_t __pattern_{0}[] = {1}\n".format(self.name, '{'));
        for v in self.values[:-1]:
            outputFile.write("    {0},\n".format(v.asString()))
        outputFile.write("    {0}\n".format(self.values[-1].asString()))
        outputFile.write("};\n");
        outputFile.write("uint_t __pattern_{0}Cursor = 0;\n".format(self.name));

########################################################
class SetStatement(object):
    def __init__(self, name, value):
        self.name = name.lower()
        self.value = value

    def addComponents(self, componentRegister, conditionCollection):
        componentRegister.setState(self.name, self.value,
                                   conditionCollection.conditionStack,
                                   conditionCollection.branchNumber)

    def getCode(self, indent):
        return "set " + self.name + " " + self.value.getCode() + ';'

class DefineStatement(object):
    def __init__(self, name, parameters):
        self.name = name.lower()
        self.parameters = parameters

    def getCode(self, indent):
        result = self.name
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

    def addComponents(self, componentRegister, conditionCollection):
        if self.name in componentRegister.defines:
            userError("Define {0} already specified, ignoring\n".format(self.name))
            return
        componentRegister.defines[self.name] = self

########################################################
class ComponentUseCase(object):
    def __init__(self, type, name, parameters):
        self.type = type.lower()
        self.name = name.lower()
        self.parameters = parameters

    def getCode(self, indent):
        result = self.type + " " + self.name + ","
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

    def addComponents(self, componentRegister, conditionCollection):
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

    def addComponents(self, componentRegister, conditionCollection):
        if self.blockType == CODE_BlOCK_TYPE_PROGRAM:
            # reset all
            conditionCollection.reset()
            # TODO: system parameters!
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

        # if there are some declarations present for this code branch,
        # make sure it gets unique number
        if conditionCollection.branchChanged and len(self.declarations):
            conditionCollection.branchNumber += 1
            conditionCollection.branchChanged = False

        # add components - use cases always first, "when ..." blocks afterwards!
        for d in self.declarations:
            if type(d) is SystemParameter:
                if self.blockType != CODE_BlOCK_TYPE_PROGRAM:
                    userError("Parameter declarations supported only in top level, ignoring parameter {0}\n".format(
                            d.name))
                else:
                    d.addComponents(componentRegister, conditionCollection)
            if type(d) is PatternDeclaration:
                if self.blockType != CODE_BlOCK_TYPE_PROGRAM:
                    userError("Pattern declarations supported only in top level, ignoring pattern {0}\n".format(
                            d.name))
                else:
                    d.addComponents(componentRegister, conditionCollection)
            if type(d) is SetStatement or type(d) is DefineStatement:
                d.addComponents(componentRegister, conditionCollection)
        for d in self.declarations:
            if type(d) is ComponentUseCase:
                d.addComponents(componentRegister, conditionCollection)
        for d in self.declarations:
            if type(d) is CodeBlock:
                d.addComponents(componentRegister, conditionCollection)

        # recursive call
        if self.next != None:
            self.next.addComponents(componentRegister, conditionCollection)

        if self.blockType == CODE_BlOCK_TYPE_WHEN:
            # pop all conditions from this code block
            conditionCollection.pop(conditionCollection.size() - stackStartSize)
