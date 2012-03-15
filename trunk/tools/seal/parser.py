import time
import ply.lex as lex
import ply.yacc as yacc
import re, string

#import Parameter
#import Statement
#import condContainer
#import Condition
#import comment

import globals

class UseCase(object):
    condition = None
    parameters = []
    def __init__(self, condition, parameters):
        self.condition = condition
        self.parameters = parameters

class Component(object):
    name = ""
    useCases = []

    def __init__(self, name):
        self.name = name

    def addUseCase(self, condition, parameters):
        for p in parameters:
            addParameter(self, p) # XXX
        useCases.append(UseCase(condition, parameters))
        return None # TODO: error string, if any

    def addParameter(self, parameter):
        pass # TODO

    def clear(self):
        self.useCases = []

class Actuator(Component):
    def __init__(self, name):
        super(Actuator,self).__init__(name)

class Sensor(Component):
    def __init__(self, name):
        super(Sensor,self).__init__(name)

class Output(Component):
    def __init__(self, name):
        super(Output,self).__init__(name)

class ComponentRegister(object):
    actuators = {}
    sensors = {}
    outputs = {}
    module = None

    def __init__(self):
        pass

    # load all components for this platform from a file
    def load(self, sourceFile):
        # import the module
        module = __import__(sourceFile)
        # construct the objects
        # (TODO: check for duplicates!)
        for n in module.actuators:
            actuators[n.lower()] = Actuator(n)
        for n in module.sensors:
            sensors[n.lower()]  = Sensor(n)
        for n in module.outputs:
            outputs[n.lower()] = Output(n)

    def findComponent(self, type, name):
        o = None
        if type == "read":
            o = sensors.get(name, None)
        elif type == "use":
            o = actuators.get(name, None)
        elif type == "output":
            o = outputs.get(name, None)
        return o

    def useComponent(self, type, name, parameters, condition):
        o = self.findComponent(type, name)
        if o == None:
            errorStr = "component %s not found" % name
            return errorStr
        return o.addUseCase(condition, parameters)

class Value(object):
    value = None
    suffix = None
    def __init__(self):
        pass
    def getCode():
        s = "{0}".format(self.value)
        if not (suffix is None):
            s += suffix
        return s

###################################################
#class ComponentRegister(object):
#    allComponents = []
#    def __init__(self):
#        pass
#    def useComponent(type, name, parameters, condition):
#        o = componentSpecification.findComponent(type, name)
#        if o == None:
#            errorStr = "component %s not found" % name
#            return errorStr
#        c = None
#        if type == "read":
#            c = Sensor(name)
#        elif type == "use":
#            c = Actuator(name)
#        elif type == "output":
#            c = Output(name)
#        return None

#componentSpecification = ComponentSpecification()
componentRegister = ComponentRegister()

###################################################

#class SealObject(object):
#    pass

#class Component(object):
#    name = None

#class Condition(object):
#    text = None
#    index = None

#def addComponentWithCondition(component, condition)

#class Program(object):
#    declaration_list = []
##    def generateObjects(self):
#        for d in declaration_list:
#            addComponentWithCondition(d, None)
#    def __init__(self, declaration_list):
#        self.declaration_list = declaration_list
#    def getCode(self):
#        code = ""
#        for d in declaration_list:
#            code += d.getCode()
#        return code

#class Declaration(object):

###################################################

class SealParser():
    def __init__(self, printMsg):
        # Lex & yacc
        self.lex = lex.lex(module = self, debug = True, reflags=re.IGNORECASE)
        self.yacc = yacc.yacc(module = self, debug = True)
        # current condtion (for context)
        self.currentCondition = None
        # printing function
        self.printMsg = printMsg
        # initialization done!
        print "Lex & Yacc init done!"
        print "Note: cache is used, so warnings are shown only at first-time compilation!"

    def run(self, s):
        self.result = []
        if s != None:
            print s
            start = time.time()
            # \n added because needed! helps resolving conflicts
            self.yacc.parse('\n' + s + '\n')
            print "Parsing done in %.4f s" % (time.time() - start)
            
        
#        for x in self.result:
#            print x
#            if x != None:
#                print x.getCode("") + '\n'
        return self.result

### Lex

    # Tokens (case insensitive!)
    reserved = {
      "use": "USE_TOKEN",
      "read": "READ_TOKEN",
      "sendto": "SENDTO_TOKEN",
      "when": "WHEN_TOKEN",
      "else": "ELSE_TOKEN",
      "elsewhen": "ELSEWHEN_TOKEN",
      "end": "END_TOKEN",
      "code": "CODE_TOKEN",
      "true": "TRUE_TOKEN",
      "false": "FALSE_TOKEN",
      "not": "NOT_TOKEN",
      "==": "EQ_TOKEN",
      "!=": "NEQ_TOKEN",
      ">": "GR_TOKEN",
      "<": "LE_TOKEN",
      ">=": "GEQ_TOKEN",
      "<=": "LEQ_TOKEN",
      ">": "GR_TOKEN",
      }
 
    tokens = ['IDENTIFIER_TOKEN'
              + 'QUALIFIED_INTEGER_TOKEN']
              + list(reserved.values())

#    tokens = [
#        'CODE_BLOCK', 'IDENTIFIER_TOKEN', 'SECONDS_TOKEN',
#        'MILISECONDS_TOKEN', 'INTEGER_TOKEN', 'COMMENT_TOKEN' #, 'newline'
#        ] + list(reserved.values())

    literals = ['.', ',', ':', ';', '{', '}']

    def t_IDENTIFIER_TOKEN(self, t):
        r'[a-zA-Z_][0-9a-zA-Z_]*'
        # This checks if no reserved token is met!!
        t.type = self.reserved.get(t.value, "IDENTIFIER_TOKEN")
        return t
    
#    def t_SECONDS_TOKEN(self, t):
#        r'[0-9]+s'
#        return t
    
#    def t_MILISECONDS_TOKEN(self, t):
#        r'[0-9]+ms'
#        #t.value = int(t.value[:-2])
#        return t

    def t_QUALIFIED_INTEGER_TOKEN(self, t):
        r'[0-9]+[a-zA-Z_]*'
        prefix, suffix = '', ''
        parsingPrefix = True
        for c in t.value:
            if parsingPrefix:
                if c >= '0' and c <= '9':
                    prefix += c
                    continue
                parsingPrefix = False
            suffix += c
        t.value = (int(prefix), suffix)
        return t

#    def t_COMMENT_TOKEN(self, t):
#        r'''//.*'''
#        t.value = t.value.strip("/ ")
#        print 'comment:', t.value
#        return t

    # Needed for OP :)
#    def t_HELP_TOKEN(self, t):
#        r'[=<>!][=]?'
#        # This checks if no reserved token is met!!
#        t.type = self.reserved.get(t.value, "HELP_TOKEN")
#        return t

    t_ignore = " \t\r\n"

#    def t_newline(self, t):
#        r'\n'
#        print "newline", t.lexer.lineno
#        t.lexer.lineno += t.value.count("\n")
#        self.currLine += 1
#        return t

    def t_error(self, t):
        self.printMsg("Line '%d': Illegal character '%s'" % 
                      (self.lineNumber, t.value[0]))
        t.lexer.skip(1)

### YACC

    def p_program(self, p):
        '''program : declaration_list
        '''
        self.result = p[1]

    def p_declaration_list(self, p):
        '''declaration_list : declaration_list declaration
                            |
        '''
        if len(p) == 1:
            p[0] = []
        elif len(p) == 3:
            p[1].append(p[2])
            p[0] = p[1]

    def p_declaration(self, p):
        '''declaration : USE_TOKEN_C IDENTIFIER_TOKEN parameter_list ';'
                       | READ_TOKEN_C IDENTIFIER_TOKEN parameter_list ';'
                       | SENDTO_TOKEN_C IDENTIFIER_TOKEN parameter_list ';'
                       | when_block
        '''
        if p[1].lower() == 'use' or p[1].lower() == 'read' or p[1].lower() == 'sendto':
            # add component with condition and parameters
            error = componentRegister.useComponent(p[1].lower(), p[2].lower(), p[3], self.currentCondition)
            if error != None:
                self.errorMsg(p, error)
        else:
            pass # TODO - conditions

        p[0] = [] # TODO
    
     
    def p_parameter_list(self, p):
        '''parameter_list : parameter_list ',' parameter
                          |'''
        if len(p) == 1:
            p[0] = []
        elif len(p) == 4:
            p[1].append(p[3])
            p[0] = p[1]
        
    def p_parameter(self, p):
        '''parameter : IDENTIFIER_TOKEN
                     | IDENTIFIER_TOKEN value
                     | WHEN_TOKEN condition
        '''
        if p[1] == "when":
            pass # TODO
        elif len(p) == 2:
            p[0] = Parameter.Parameter(p[1], None)
        else:
            p[0] = Parameter.Parameter(p[1], p[2])
#        # Works for both cases, in condition case parameter's name is when :)
#        # Idea is that if when exist will always be last parameter, but
#        # maybe we can allow it to be any parameter.
#        elif len(p) == 3:
#            p[0] = Parameter.Parameter(p[1], p[2])

    def p_boolean_value(self, p):
        '''boolean_value : TRUE_TOKEN
                         | FALSE_TOKEN
        '''
        v = Value()
        if string.lower(p[1]) == 't':
            v.value = True
        else:
            v.value = False
        p[0] = v

    def p_integer_value(self, p):
        '''integer_value : QUALIFIED_INTEGER_TOKEN
        '''
        v = Value()
        v.value = p[1][0]
        v.suffix = p[1][1]
        p[0] = v
        
    def p_identifier(self, p):
        '''identifier : IDENTIFIER_TOKEN'''
        v = Value()
        v.value = p[1]
        p[0] = v

    def p_value(self, p):
        '''value : boolean_value
                 | integer_value
                 | IDENTIFIER_TOKEN
        '''
        p[0] = p[1]

#    def p_qualified_number(self, p):
#        '''qualified_number : INTEGER_TOKEN
#                            | SECONDS_TOKEN
#                            | MILISECONDS_TOKEN
#        '''
#        p[0] = p[1]

    def p_condition(self, p):
        '''condition : value
                     | NOT_TOKEN condition
                     | condition op condition
        '''
        p[0] = p[1]
        for x in range(2, len(p)):
            p[0] += ' ' + p[x]

#    def p_class_parameter(self, p):
#        '''class_parameter : IDENTIFIER_TOKEN
#        '''
#        p[0] = p[1]

#    def p_class_parameter(self, p):
#        '''class_parameter : IDENTIFIER_TOKEN "." IDENTIFIER_TOKEN
#        '''
#        p[0] = p[1] + "." + p[3]

    def p_op(self, p):
        ''' op : EQ_TOKEN
               | NEQ_TOKEN
               | GR_TOKEN
               | LE_TOKEN
               | GEQ_TOKEN
               | LEQ_TOKEN
        '''
        p[0] = p[1]

    def p_error(self, p):
        if p:
            self.printMsg("Line '%d': Syntax error at '%s'" % (p.lineno(1), p.value))
        else:
            self.printMsg("Syntax error at EOF")

    def errorMsg(self, p, msg):
        self.printMsg("Line '%d': Syntax error at '%s'" % (p.lineno(1), p.value))
        self.printMsg(msg)

### Helpers

#    def queueComment(self, comment, pre = False):
#        if pre:
#            if isinstance(comment, list):
#                for x in comment:
#                    self.commentQueue.addPreComment(x)
#            else:
#                self.commentQueue.setPreComments(comment)
#        else:
#            self.commentQueue.setPostComment(comment)
#            
#    def getQueuedComment(self):
#        queuedComment = self.commentQueue
#        self.commentQueue = comment.Comment([], '')
#        return queuedComment
    
