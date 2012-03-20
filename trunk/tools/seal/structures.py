
########################################################

INDENT_STRING = "  "  # indent new code level with two spaces

def getIndent(indent):
    result = ""
    i = indent
    while i > 0:
        result += INDENT_STRING
        i -= 1
    return result

########################################################
class Value(object):
    def __init__(self, value=None, suffix=None):
        self.value = value
        self.suffix = value
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
        result = self.type + " " + self.name + " "
        for p in self.parameters:
            result += p[0]
            if p[1] != None:
                result += " "
                result += p[1].getCode()
            result += ", "
        result = result[:-2]
        result += ';'
        return result

########################################################
class Declaration(object):
    def __init__(self, componentUseCase, whenBlock):
        self.componentUseCase = componentUseCase
        self.whenBlock = whenBlock

    def getCode(self, indent):
        if self.componentUseCase != None:
            return getIndent(indent) + self.componentUseCase.getCode(indent)
        else:
            return getIndent(indent) + self.whenBlock.getCode(indent)

########################################################
CODE_BlOCK_TYPE_WHEN = 1
CODE_BlOCK_TYPE_ELSEWHEN = 2
CODE_BlOCK_TYPE_ELSE = 3

class CodeBlock(object):
    def __init__(self, blockType, condition, declarations, next):
        self.blockType = blockType
        self.condition = condition
        self.declarations = declarations
        self.next = next

    def getCode(self, indent):
        result = ""
        if self.blockType == CODE_BlOCK_TYPE_WHEN:
            if self.condition != None:  # unless top level block...
                result = "when"
                result += " " + self.condition.getCode() + " "
                result = ":\n"
                indent += 1
        elif self.blockType == CODE_BlOCK_TYPE_ELSEWHEN:
            result = "elsewhen"
            result += " " + self.condition.getCode() + " "
            result = ":\n"
            indent += 1
        elif self.blockType == CODE_BlOCK_TYPE_ELSE:
            result = "else:\n"
            indent += 1
 
        for d in self.declarations:
            result += getIndent(indent)
            result += d.getCode(indent)
            result += "\n"

        if self.next != None:
            assert indent > 0
            indent -= 1
            result += getIndent(indent)
            result += self.next.getCode(indent)
            result += "\n"

        return result
