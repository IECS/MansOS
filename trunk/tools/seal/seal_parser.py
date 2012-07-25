import time
import ply.lex as lex
import ply.yacc as yacc
import re, string

from structures import *
import components

###################################################

class SealParser():
    def __init__(self, architecture, printMsg, verboseMode = True, debugMode = True):
        self.isError = False
        # Lex & yacc
        self.lex = lex.lex(module = self, debug = verboseMode, reflags = re.IGNORECASE)
        self.yacc = yacc.yacc(module = self, debug = verboseMode)
        # current condtion (for context)
        self.currentCondition = None
        self.newCode = True
        self.lineTracking = {"Condition": [], "Statement": []}
        # save parameters
        self.printMsg = printMsg
        self.verboseMode = verboseMode
        self.debugMode = debugMode
        # initialization done!
        if verboseMode:
            print "Lex & Yacc init done!"
            print "Note: cache is used, so warnings are shown only at first-time compilation!"
        # load available components
        components.componentRegister.load(architecture)

    def run(self, s):
        if self.verboseMode:
            print s
        if s == None: return
        self.newCode = True
        self.result = None
        self.lineTracking = {"Condition": [], "Statement": []}
        start = time.time()
        # \n added because guarantees, that lineNr is correct!
        self.yacc.parse('\n' + s)
        if self.verboseMode:
            print "Parsing done in %.4f s" % (time.time() - start)
        if self.result:
            self.result.addComponents(components.componentRegister,
                                      components.conditionCollection)
        return self.result

### Lex

    # Tokens (case insensitive!)
    reserved = {
      "use": "USE_TOKEN",
      "read": "READ_TOKEN",
      "networkread": "NETWORK_READ_TOKEN",
      "output": "OUTPUT_TOKEN",
      "when": "WHEN_TOKEN",
      "else": "ELSE_TOKEN",
      "elsewhen": "ELSEWHEN_TOKEN",
      "end": "END_TOKEN",
      "parameters": "PARAMETERS_TOKEN",
      "define": "DEFINE_TOKEN",
      "config": "CONFIG_TOKEN",
      "const": "CONST_TOKEN",
      "set": "SET_TOKEN",
      "pattern": "PATTERN_TOKEN",
      "load": "LOAD_TOKEN",
      "where": "WHERE_TOKEN",
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
                                  "HEX_INTEGER_LITERAL",
                                  "DECIMAL_INTEGER_LITERAL",
                                  "STRING_LITERAL"]

    # rules for all tokens that are not keywords (i.e. alphanumeric)
    t_EQ_TOKEN = r'==?'     # two alternative spellings: '==' or '='
    t_NEQ_TOKEN = r'!=|<>'  # two alternative spellings: '!=' or '<>'
    t_GEQ_TOKEN = r'>='
    t_LEQ_TOKEN = r'<='

    # Rules:
    #   - avoid different kind of braces and brackets (as in C),
    #     instead use "(" and ")" for everything
    #   - avoid arithmetic operators, use functions instead
    #   - but allow unary + and -
    literals = ['.', ',', ':', ';', '(', ')', '<', '>', '+', '-']

    t_ignore = " \t\r"

    def t_IDENTIFIER_TOKEN(self, t):
        r'[a-zA-Z_][0-9a-zA-Z_]*'
        # This checks if reserved token is met.
        t.type = self.reserved.get(t.value.lower(), "IDENTIFIER_TOKEN")
        return t

    def t_HEX_INTEGER_LITERAL(self, t):
        r'0x[0-9a-fA-F]+'
        t.value = (int(t.value, 16), '')
        return t

    def t_DECIMAL_INTEGER_LITERAL(self, t):
        r'[+-]?[0-9]+[a-zA-Z_]*'
        prefix, suffix = '', ''
        parsingSign = True
        parsingPrefix = False
        sign = 1
        for c in t.value:
            if parsingSign:
                parsingSign = False
                parsingPrefix = True
                if c == '+':
                    continue
                if c == '-':
                    sign = -1
                    continue
            if parsingPrefix:
                if c >= '0' and c <= '9':
                    prefix += c
                    continue
                parsingPrefix = False
            suffix += c.lower()
        t.value = (int(prefix) * sign, suffix)
        return t

    t_STRING_LITERAL = r'"[^"\n]*"'

    def t_newline(self, t):
        r'\n+'
        if self.newCode:
            self.newCode = False
            t.lexer.lineno = 0
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
        self.result = CodeBlock(CODE_BLOCK_TYPE_PROGRAM, None, p[1], None)

    def p_declaration_list(self, p):
        '''declaration_list : declaration_list declaration
                            | empty
        '''
        if len(p) == 2:
            p[0] = []
        elif len(p) == 3:
            if p[2] != ';':
                p[1].append(p[2])
            p[0] = p[1]

    def p_declaration(self, p):
        '''declaration : component_use_case
                       | network_read_statement
                       | when_block
                       | system_config
                       | pattern_declaration
                       | const_statement 
                       | set_statement
                       | define_statement
                       | parameters_statement
                       | load_statement
                       | ';'
                       | error END_TOKEN
                       | error ';'
        '''
        # use case, when block, parameter, "set" statement, or empty statement
        if len(p) == 2:
            p[0] = p[1]
        # error token
        elif len(p) == 3:
            self.printMsg("Trying to continue...\n")

    def p_component_use_case(self, p):
        '''component_use_case : USE_TOKEN IDENTIFIER_TOKEN parameter_list ';'
                              | READ_TOKEN functional_expression parameter_list ';'
                              | OUTPUT_TOKEN IDENTIFIER_TOKEN output_fields parameter_list ';'
        '''
        # allow all, including unknown, components here (because of virtual components)
        if len(p) == 5:
            p[0] = ComponentUseCase(p[1], p[2], p[3], None)
        else:
            p[0] = ComponentUseCase(p[1], p[2], p[4], p[3])
        self.lineTracking["Statement"].append((p.lineno(1), p.lineno(4), p[0]))

    def p_network_read_statement(self, p):
        '''network_read_statement : NETWORK_READ_TOKEN IDENTIFIER_TOKEN output_fields ';'
        '''
        p[0] = NetworkReadStatement(p[2], p[3])

    def p_system_config(self, p):
        '''system_config : CONFIG_TOKEN value ';'
        '''
        p[0] = SystemParameter(p[2])

    def p_const_statement(self, p):
        '''const_statement : CONST_TOKEN IDENTIFIER_TOKEN value ';'
        '''
        name = p[2].lower()
        value = p[3]
        p[0] = ConstStatement(name, value)
        if name in components.componentRegister.systemConstants:
            self.errorMsg(p, "Constant '{}' already defined, ignoring".format(name), False)
        else:
            components.componentRegister.systemConstants[name] = value

    def p_set_statement(self, p):
        '''set_statement : SET_TOKEN IDENTIFIER_TOKEN value ';'
        '''
        p[0] = SetStatement(p[2], p[3])

    def p_pattern_declaration(self, p):
        '''pattern_declaration : PATTERN_TOKEN IDENTIFIER_TOKEN '(' value_list ')' ';'
        '''
        p[0] = PatternDeclaration(p[2], p[4])

    def p_define_statement(self, p):
        '''define_statement : DEFINE_TOKEN IDENTIFIER_TOKEN functional_expression parameter_list ';'
        '''
        p[0] = ComponentDefineStatement(p[2], p[3], p[4])

    def p_load_statement(self, p):
        '''load_statement : LOAD_TOKEN STRING_LITERAL ';'
        '''
        p[0] = LoadStatement(p[2].strip('"'))

    def p_functional_expression(self, p):
        '''functional_expression : IDENTIFIER_TOKEN '(' argument_list ')'
                                 | value
        '''
        if len(p) == 2:
            # hacks...
            if type(p[1].value) is str: v = p[1].value
            # XXX: for now just ignore structure.field syntax inside functions
            # elif type(p[1].value) is SealValue: v = p[1].value.firstPart
            elif type(p[1].value) is SealValue: v = p[1].value
            else: v = p[1]
            p[0] = FunctionTree(v, [])
        else:
            p[3].reverse()
            p[0] = FunctionTree(p[1], p[3])

    def p_argument_list(self, p):
        '''argument_list : functional_expression ',' argument_list
                         | functional_expression
        '''
        if len(p) == 4: p[0] = p[3]
        else: p[0] = []
        p[0].append(p[1])

    def p_output_fields(self, p):
        '''output_fields : '(' output_field_list ')'
                         | empty
        '''
        if len(p) == 2: p[0] = []
        else: p[0] = p[2]

    def p_output_field_list(self, p):
        '''output_field_list : output_field_spec ',' output_field_list
                             | output_field_spec
        '''
        if len(p) == 4: p[0] = p[3]
        else: p[0] = []
        p[0].append(p[1])

    def p_output_field_spec(self, p):
        '''output_field_spec : IDENTIFIER_TOKEN
                             | IDENTIFIER_TOKEN integer_literal
        '''
        if len(p) == 2: p[0] = (p[1], 1)
        else: p[0] = (p[1], p[2].value)

    def p_parameters_statement(self, p):
        '''parameters_statement : PARAMETERS_TOKEN IDENTIFIER_TOKEN parameter_list ';'
        '''
        p[0] = ParametersDefineStatement(p[2], p[3])

    def p_when_block(self, p):
        '''when_block : WHEN_TOKEN condition ':' declaration_list elsewhen_block END_TOKEN
        '''
        p[0] = CodeBlock(CODE_BLOCK_TYPE_WHEN, p[2], p[4], p[5])
        self.lineTracking["Condition"].append((p.lineno(1), p.lineno(6), p[0]))

    def p_elsewhen_block(self, p):
        '''elsewhen_block : ELSEWHEN_TOKEN condition ':' declaration_list elsewhen_block
                          | ELSE_TOKEN ':' declaration_list
                          | empty
        '''
        if len(p) == 2:   # empty
            p[0] = None
        elif len(p) == 4: # else block
            p[0] = CodeBlock(CODE_BLOCK_TYPE_ELSE, None, p[3], None)
        else:             # elsewhen block
            p[0] = CodeBlock(CODE_BLOCK_TYPE_ELSEWHEN, p[2], p[4], p[5])

    def p_condition(self, p):
        '''condition : condition_term
                     | condition OR_TOKEN condition_term
           condition_term : condition_factor
                     | condition_term AND_TOKEN condition_factor
           condition_factor : logical_statement
                     | NOT_TOKEN condition_factor
        '''
        if len(p) == 2: # logical_statement
#            p[0] = p[1]
            p[0] = Expression(right=p[1])
        elif len(p) == 3: # NOT condition
            p[0] = Expression(None, p[1], p[2])
        else:
            p[0] = Expression(p[1], p[2], p[3])

    def p_logical_statement(self, p):
        ''' logical_statement : functional_expression
               | '(' condition ')'
               | functional_expression EQ_TOKEN functional_expression
               | functional_expression NEQ_TOKEN functional_expression
               | functional_expression '>' functional_expression
               | functional_expression '<' functional_expression
               | functional_expression GEQ_TOKEN functional_expression
               | functional_expression LEQ_TOKEN functional_expression
        '''
        if len(p) == 2:
            p[0] = p[1]
        elif p[1] == '(':
            p[0] = p[2] # TODO: will not be able to generate code correctly!
        else:
            p[0] = Expression(p[1], p[2], p[3])

    def p_parameter_list(self, p):
        '''parameter_list : parameter_list ',' parameter
                          | empty
        '''
        if len(p) == 2:
            p[0] = []
        elif len(p) == 4:
            p[1].append(p[3])
            p[0] = p[1]

    # returns pair (string, Value) or (string, Condition)
    def p_parameter(self, p):
        '''parameter : IDENTIFIER_TOKEN
                     | IDENTIFIER_TOKEN value
                     | PATTERN_TOKEN value
                     | WHERE_TOKEN condition
        '''
        if len(p) == 2:
            p[0] = (p[1], None)
        else:
            p[0] = (p[1], p[2])

    def p_value_list(self, p):
        '''value_list : value_list ',' value
                      | value
        '''
        if len(p) == 2:
            p[0] = [p[1]]
        else:
            p[0] = p[1]
            p[0].append(p[3])

    def p_value(self, p):
        '''value : boolean_literal
                 | integer_literal
                 | string_literal
                 | identifier
        '''
        p[0] = p[1]

    def p_boolean_literal(self, p):
        '''boolean_literal : TRUE_TOKEN
                           | FALSE_TOKEN'''
        p[0] = Value(string.lower(p[1][0]) == 't')

    def p_integer_literal(self, p):
        '''integer_literal : DECIMAL_INTEGER_LITERAL
                           | HEX_INTEGER_LITERAL'''
        p[0] = Value(p[1][0], p[1][1])

    def p_string_literal(self, p):
        '''string_literal : STRING_LITERAL'''
        p[0] = Value(p[1])

    def p_identifier(self, p):
        '''identifier : IDENTIFIER_TOKEN
                      | IDENTIFIER_TOKEN '.' IDENTIFIER_TOKEN
        '''
        p[1] = p[1].lower() # leave p[3] alone
        if len(p) == 2:
            # try to resolve constant
            const = components.componentRegister.systemConstants.get(p[1], None)
            if const: p[0] = Value(const.value)
            else: p[0] = Value(SealValue(p[1]))
        else:
            p[0] = Value(SealValue(p[1], p[3]))

    def p_empty(self, p):
        '''empty :'''
        pass

    def p_error(self, p):
        self.isError = True
        if p:
            # TODO: print better message!
            self.printMsg("Syntax error at line {0}: {1}\n".format(p.lineno, p.value))
        else:
            self.printMsg("Syntax error at EOF\n")

    def errorMsg(self, p, msg, doSetError = True):
        if doSetError: self.isError = True
        self.printMsg("Syntax error at line {0}: {1}\n".format(p.lineno(1), msg))
