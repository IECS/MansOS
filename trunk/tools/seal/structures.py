class Value(object):
    def __init__(self, value=None, suffix=None):
        self.value = value
        self.suffix = value
    def getCode(self):
        s = "{0}".format(self.value)
        if not (self.suffix is None):
            s += self.suffix
        return s

class Expression(object):
#    left, op, right = None, None, None

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
