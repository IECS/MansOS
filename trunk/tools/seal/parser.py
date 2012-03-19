import time
import ply.lex as lex
import ply.yacc as yacc
import re, string

from structures import *
from components import *

###################################################

class SealParser():
    def __init__(self, printMsg, verboseMode):
        # Lex & yacc
        self.lex = lex.lex(module = self, debug = verboseMode, reflags=re.IGNORECASE)
        self.yacc = yacc.yacc(module = self, debug = verboseMode)
        # current condtion (for context)
        self.currentCondition = None
        # save parameters
        self.printMsg = printMsg
        self.verboseMode = verboseMode
        # initialization done!
        if verboseMode:
            print "Lex & Yacc init done!"
            print "Note: cache is used, so warnings are shown only at first-time compilation!"

    def run(self, s):
        self.result = []
        if s != None:
            if self.verboseMode:
                print s
            start = time.time()
            # \n added because needed! helps resolving conflicts
            self.yacc.parse('\n' + s + '\n')
            print "Parsing done in %.4f s" % (time.time() - start)
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
      "true": "TRUE_TOKEN",
      "false": "FALSE_TOKEN",
      "not": "NOT_TOKEN",
      "and": "AND_TOKEN",
      "or": "OR_TOKEN",
      "==": "EQ_TOKEN",
      "!=": "NEQ_TOKEN",
      ">=": "GEQ_TOKEN",
      "<=": "LEQ_TOKEN"}
 
    tokens = reserved.values() + ["IDENTIFIER_TOKEN",
                                  "QUALIFIED_INTEGER_TOKEN"]
    t_EQ_TOKEN = r'==?'     # alternative spellings: '==' or '='
    t_NEQ_TOKEN = r'!=|<>'  # alternative spellings: '!=' or '<>'
    t_GEQ_TOKEN = r'>='
    t_LEQ_TOKEN = r'<='

    literals = ['.', ',', ':', ';', '{', '}', '(', ')', '[', ']', '+', '-', '/', '*', '>', '<']

    t_ignore = " \t\r"

    def t_IDENTIFIER_TOKEN(self, t):
        r'[a-zA-Z_][0-9a-zA-Z_]*'
        # This checks if no reserved token is met!!
        t.type = self.reserved.get(t.value, "IDENTIFIER_TOKEN")
        return t

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

    def t_newline(self, t):
        r'\n+'
        t.lexer.lineno += t.value.count("\n")

    def t_COMMENT_TOKEN(self, t):
        r'''//.*'''
        t.value = t.value.strip("/ ")

    def t_error(self, t):
        self.printMsg("Line '%d': Illegal character '%s'" % 
                      (t.lexer.lineno, t.value[0]))
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
        '''declaration : USE_TOKEN IDENTIFIER_TOKEN parameter_list ';'
                       | READ_TOKEN IDENTIFIER_TOKEN parameter_list ';'
                       | SENDTO_TOKEN IDENTIFIER_TOKEN parameter_list ';'
                       | when_block
                       | ';'
        '''
        print "parse declaration", p[1]
        if p[1] == []:
            return
        if p[1].lower() == 'use' or p[1].lower() == 'read' or p[1].lower() == 'sendto':
            # add component with condition and parameters
            error = componentRegister.useComponent(
                p[1].lower(), p[2].lower(), p[3], self.currentCondition)
            if error != None:
                self.errorMsg(p, error)
        else:
            pass # TODO - conditions

        p[0] = [] # TODO JJ

    def p_when_block(self, p):
        '''when_block : WHEN_TOKEN condition ':' declaration_list elsewhen_block END_TOKEN
        '''
        print "parse when block, condition", p[2]
        # TODO: add to condition stack!
        self.currentCondition = p[2]
        p[0] = [] # TODO JJ

    def p_elsewhen_block(self, p):
        '''elsewhen_block : ELSEWHEN_TOKEN condition ':' declaration_list elsewhen_block
                          | ELSE_TOKEN ':' declaration_list
                          |
        '''
        p[0] = []

    def p_condition(self, p):
        '''condition : condition_term
                     | NOT_TOKEN condition
                     | condition OR_TOKEN condition_term
           condition_term : condition_term AND_TOKEN logical_statement
                     | logical_statement
        '''
        if len(p) == 2: # logical_statement
            p[0] = p[1]
        elif len(p) == 3: # NOT condition
            p[0] = Expression(None, p[1], p[2])
        else:
            p[0] = Expression(p[1], p[2], p[3])

    def p_logical_statement(self, p):
      ''' logical_statement : arithmetic_expression
               | '(' condition ')'
               | arithmetic_expression EQ_TOKEN arithmetic_expression
               | arithmetic_expression NEQ_TOKEN arithmetic_expression
               | arithmetic_expression '>' arithmetic_expression
               | arithmetic_expression '<' arithmetic_expression
               | arithmetic_expression GEQ_TOKEN arithmetic_expression
               | arithmetic_expression LEQ_TOKEN arithmetic_expression
      '''
      if len(p) == 2 or p[1] == '(':
          p[0] = p[1]
      else:
          p[0] = Expression(p[1], p[2], p[3])

    def p_arithmetic_expression(self, p):
        '''arithmetic_expression : arithmetic_expression '+' arithmetic_term
                | arithmetic_expression '-' arithmetic_term
                | arithmetic_term
           arithmetic_term : arithmetic_term '*' arithmetic_factor
                | arithmetic_term '/' arithmetic_factor
                | arithmetic_factor
        '''
        if len(p) == 2:
            p[0] = p[1]
        else:
            p[0] = Expression(p[1], p[2], p[3])

    def p_arithmetic_factor(self, p):
        '''arithmetic_factor : value
                             | '(' arithmetic_expression ')'
        '''
        if len(p) == 2:
            p[0] = p[1]
        else:
            p[1] = p[2]

    # returns pair (string, Value) or (string, Condition)
    def p_parameter(self, p):
        '''parameter : IDENTIFIER_TOKEN
                     | IDENTIFIER_TOKEN value
        '''
# TODO:
#                     | WHEN_TOKEN condition
# TODO: also support this: "filter >100"
        if len(p) == 2:
            p[0] = (p[1], None)
        else:
            # Works for both cases, in condition case parameter's name is "when" :)
            p[0] = (p[1], p[2])

    def p_parameter_list(self, p):
        '''parameter_list : parameter_list ',' parameter
                          |'''
        if len(p) == 1:
            p[0] = []
        elif len(p) == 4:
            p[1].append(p[3])
            p[0] = p[1]

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
                 | identifier
        '''
        p[0] = p[1]

    def p_error(self, p):
        if p:
            # TODO: print better message!
            self.printMsg("Line '%d': Syntax error at '%s'" % (p.lineno, p.value))
        else:
            self.printMsg("Syntax error at EOF")

    def errorMsg(self, p, msg):
        self.printMsg("Line '%d': Syntax error at..." % (p.lineno(1)))
        self.printMsg(msg)
