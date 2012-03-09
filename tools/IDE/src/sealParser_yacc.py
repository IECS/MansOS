# -*- coding: UTF-8 -*-
#
# Copyright (c) 2008-2012 the MansOS team. All rights reserved.
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
#'

import time
from ply import *

import Parameter
import Statement
import condContainer
import Condition
import comment

class SealParser():
    def __init__(self):
        
        self.commentQueue = comment.Comment()
        
        self.lex = lex.lex(module = self, debug = True)
        
        yacc.yacc(module = self, debug = True)
        
        print "Lex & Yacc init done!"

    def run(self, s):
        self.result = []
        if s != None:
            print s
            start = time.time()
            # \n added cus needed! helps resolving conflicts
            yacc.parse(s + '\n')
            print "Parsing done in %.4f s" % (time.time()-start)
        
        for x in self.result:
            print x
            if x != None:
                print x.getCode("") + '\n'
        return self.result

### Lex

    # Tokens
    reserved = {
      "parameter": "PARAMETER_TOKEN",
      "use": "USE_TOKEN",
      "read": "READ_TOKEN",
      "sendto": "SENDTO_TOKEN",
      "when": "WHEN_TOKEN",
      "else": "ELSE_TOKEN",
      "elsewhen": "ELSEWHEN_TOKEN",
      "begin": "BEGIN_TOKEN",
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
    
    t_CODE_BLOCK = r'\"\"\".*\"\"\"'
    
    tokens = [
        'CODE_BLOCK', 'IDENTIFIER_TOKEN', 'SECONDS_TOKEN',
        'MILISECONDS_TOKEN', 'INTEGER_TOKEN', 'COMMENT_TOKEN', 'newline'
        ] + list(reserved.values())

    literals = ['.',',',':',';','{','}']
    
    def t_IDENTIFIER_TOKEN(self, t):
        r'[a-zA-Z_][0-9a-zA-Z_]*'
        # This checks if no reserved token is met!!
        t.type = self.reserved.get(t.value, "IDENTIFIER_TOKEN")
        return t
    
    def t_SECONDS_TOKEN(self, t):
        r'[0-9]+s'
        return t
    
    def t_MILISECONDS_TOKEN(self, t):
        r'[0-9]+ms'
        #t.value = int(t.value[:-2])
        return t
        
    def t_INTEGER_TOKEN(self, t):
        r'[0-9]+'
        t.value = int(t.value)
        return t
        
    def t_COMMENT_TOKEN(self, t):
        r'''//.*'''
        t.value = t.value.strip("/ ")
        return t
    
    # Needed for OP :)
    def t_HELP_TOKEN(self, t):
        r'[=<>!][=]?'
        # This checks if no reserved token is met!!
        t.type = self.reserved.get(t.value, "HELP_TOKEN")
        return t
    
    t_ignore = " \t"
    
    def t_newline(self, t):
        r'\n+'
        print "newline", t.lexer.lineno
        t.lexer.lineno += t.value.count("\n")
        return t
    
    def t_error(self, t):
        print("Illegal character '%s'" % t.value[0])
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
        elif len(p) == 2:
            p[0] = []
    
    def p_declaration(self, p):
        '''declaration : comment_list USE_TOKEN IDENTIFIER_TOKEN parameter_list ';' comment
                       | comment_list READ_TOKEN IDENTIFIER_TOKEN parameter_list ';' comment
                       | comment_list SENDTO_TOKEN IDENTIFIER_TOKEN packet_field_specifier parameter_list ';' comment
                       | when_block
                       | code_block
        '''
        # code_block, when_block
        if len(p) == 2:
            p[0] = p[1]
        else:
            p[0] = Statement.Statement(p[2], p[3])
            self.queueComment(p[1], True)
            # use & read
            if len(p) == 6:
                self.queueComment(p[6])
                p[0].setParameters(p[3])
            # send_to
            elif len(p) == 7:
                self.queueComment(p[6])
                p[0].setParameters(p[4])
                # TODO: parse packet_field
            p[0].setComment(self.getQueuedComment())
        
    def p_when_block(self, p):
        '''when_block : comment_list WHEN_TOKEN condition ":" comment declaration_list elsewhen_block END_TOKEN comment
        '''
        if len(p) == 10:
            p[0] = p[7]
            newCond = Condition.Condition("when")
            newCond.setCondition(p[3])
            newCond.setStatements(p[6])
            self.queueComment(p[1], True)
            self.queueComment(p[5])
            newCond.setComment(self.getQueuedComment())
            #self.queueComment(p[8], True) # this comment raises conflicts!!! -> comment_list
            # maybe we need to add non empty declaration list!!
            self.queueComment(p[9])
            p[0].setEndComment(self.getQueuedComment())
            p[0].setWhen(newCond)
            p[0].fixElseWhenOrder()
    
    def p_elsewhen_block(self, p):
        '''elsewhen_block : comment_list ELSEWHEN_TOKEN condition ":" comment declaration_list elsewhen_block
                          | comment_list ELSE_TOKEN ":" comment declaration_list
                          | 
        '''
        if len(p) == 1:
            p[1] = []
        elif len(p) == 2:
            p[0] = condContainer.condContainer()
        # else
        elif len(p) == 6:
            newCond = Condition.Condition("else")
            newCond.setStatements(p[5])
            self.queueComment(p[1], True)
            self.queueComment(p[4])
            newCond.setComment(self.getQueuedComment())
            p[0] = condContainer.condContainer()
            p[0].setElse(newCond)
        # elsewhen
        elif len(p) == 8:
            p[0] = p[7]
            newCond = Condition.Condition("elsewhen")
            newCond.setCondition(p[3])
            newCond.setStatements(p[6])
            self.queueComment(p[1], True)
            self.queueComment(p[5])
            newCond.setComment(self.getQueuedComment())
            p[0].addElseWhen(newCond)
        
    def p_code_block(self, p):
        '''code_block : CODE_TOKEN IDENTIFIER_TOKEN CODE_BLOCK
        '''
        # ???

    def p_packet_field_specifier(self, p):
        '''packet_field_specifier : "{" packet_field_list "}"
        '''
    
    def p_packet_field_list(self, p):
        '''packet_field_list : packet_field_list "," packet_field
                             | packet_field
        '''

    def p_packet_field(self, p):
        '''packet_field : IDENTIFIER_TOKEN
                        | IDENTIFIER_TOKEN value
        '''
    
    def p_comment_list(self, p):
        '''comment_list : comment_list comment
                        | comment
        '''
        if len(p) == 2:
            p[0] = [p[1]]
        elif len(p) == 3:
            p[1].append(p[2])
            p[0] = p[1]
    
    def p_comment(self, p):
        '''comment : COMMENT_TOKEN newline
                   | newline
        '''
        if len(p) == 2:
            p[1] = ''
        p[0] = p[1]
    
    def p_parameter_list(self, p):
        '''parameter_list : parameter_list "," parameter
                          |'''
        if len(p) == 1:
            p[0] = ''
        elif len(p) == 4:
            p[1].append(p[3])
            p[0] = p[1]
        
    def p_parameter(self, p):
        '''parameter : IDENTIFIER_TOKEN
                     | IDENTIFIER_TOKEN value
                     | WHEN_TOKEN condition
        '''
        if len(p) == 2:
            p[0] = Parameter.Parameter(p[1], True)
        # Works for both cases, in condition case parameter's name is when :)
        elif len(p) == 3:
            p[0] = Parameter.Parameter(p[1], p[2])

    def p_value(self, p):
        '''value : boolean_value
                 | qualified_number
                 | IDENTIFIER_TOKEN
        '''
        p[0] = p[1]
    
    def p_boolean_value(self, p):
        '''boolean_value : TRUE_TOKEN
                         | FALSE_TOKEN
        '''
        p[0] = p[1]
        
    def p_qualified_number(self, p):
        '''qualified_number : INTEGER_TOKEN
                            | SECONDS_TOKEN
                            | MILISECONDS_TOKEN
        '''
        p[0] = p[1]

    def p_condition(self, p):
        '''condition : class_parameter
                     | NOT_TOKEN class_parameter
                     | class_parameter op qualified_number
                     | qualified_number op class_parameter
                     | NOT_TOKEN class_parameter op qualified_number
                     | NOT_TOKEN qualified_number op class_parameter
                     | boolean_value
        '''
        p[0] = p[1]
        for x in range(2, len(p)):
            p[0] += ' ' + p[x]
         
    def p_class_parameter(self, p):
        '''class_parameter : IDENTIFIER_TOKEN "." IDENTIFIER_TOKEN
        '''
        p[0] = p[1] + "." + p[3]
        
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
            print("Syntax error at '%s'" % p.value)
        else:
            print("Syntax error at EOF")

### Helpers

    def queueComment(self, comment, pre = False):
        if pre:
            self.commentQueue.setPreComments(comment)
        else:
            self.commentQueue.setPostComment(comment)
            
    def getQueuedComment(self):
        queuedComment = self.commentQueue
        self.commentQueue = comment.Comment()
        return queuedComment
    