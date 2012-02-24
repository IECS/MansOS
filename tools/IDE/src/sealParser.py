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
#

import re
import Statement
import globals as g
import condContainer
import Condition

class SealParser:
    def __init__(self, API):
        # Define regex'es and handling functions for all what needs to be parsed
        self.API = API
        self.__commentQueue = []
        self.__matches = [
            [re.compile(r"^(?://).*", re.IGNORECASE), 
             self.__queueComment],
            [re.compile(r"^(?:use|read|output) .*(?:,| )when .*", re.IGNORECASE), 
             self.__parseStatementWithWhen],
            [re.compile(r"^(?:use|read|output) .*", re.IGNORECASE), 
             self.__parseStatement],
            [re.compile(r"^when (.*):.*\n((?:.\n)*)end", re.IGNORECASE), 
             self.__parseSimpleWhen]
                         ]
        # regex for splitting statements in parts
        self.__statRegex = re.compile(
                r"^(use|read|output) (\w*),?(.*);", re.IGNORECASE)
        self.__statWhenRegex = re.compile(
                r"^((?:use|read|output) .*)(?:,| )when (.*);", re.IGNORECASE)
        self.__whenRegex = re.compile(
                r"^when (.*:.*)\n((?:.*\n)*?)((?: *//.*\n)*) *end *(//.*)", re.IGNORECASE)
            #TODO: Add complicated when!!!
        
    def parseCode(self, source):
        if source == None:
            return None
        source = source.strip("\n ")
        result = []
        # Parse first statement in source until all is parsed
        while source != '':
            found = False
            # Cycle all regexes to find one who suits
            for regex, parser in self.__matches:
                res = regex.match(source)
                # If regex matches, call responsible parser for this chunk
                if res != None:
                    obj = parser(source[:res.end()])
                    if obj != None:
                        result.append(obj)
                    # TODO add returned Statement|Condition to seal struct!!
                    source = source[res.end():].strip("\n ")
                    found = True
                    break
            # If no regex matched drop first line add log it
            if not found:
                #TODO: Try to fix it, check for ending ; etc...
                errorCode = source.split("\n",1)
                self.API.logMsg(g.WARNING,"Failed to parse: \"" + 
                                errorCode[0].strip("\n ") + "\"")
                if len(errorCode) > 1:
                    source = errorCode[1].strip("\n ")
                else:
                    break
        return result
            
    # Returns given code split in format [code, comments]
    def __findComments(self, code):
        i = 0
        single = double = 0
        while i < len(code):
            if code[i] == "'":
                single += 1
            elif code[i] == '"':
                double += 1
            elif code[i:i + 2] == "//":
                if single % 2 == 0 and double % 2 == 0:
                    return code[:i], code[i:]
            i += 1
        return code, ''
    
    def __queueComment(self, comment):
        self.__commentQueue.append(comment)
        # Needed because this is called as parser
        return None
    
    def __getQueuedComments(self, obj):
        while len(self.__commentQueue):
            obj.addComment(self.__commentQueue.pop(0))
        return obj
    
    def __parseStatementWithWhen(self, code):
        code, comment = self.__findComments(code)
        res = self.__statWhenRegex.match(code)
        if res == None:
            self.API.logMsg(g.WARNING, "Syntax error: \"" + code + "\"")
            return None
        # Create simple statement, parsing it without when part
        newStatement = self.__parseStatement(res.group(1) + ';', True)
        # Modify created statement with condition and inline comment
        newStatement.setCondition(res.group(2))
        newStatement.setInlineComment(comment)
        self.API.logMsg(g.INFO,"Parsed: \n" +  newStatement.getCode(''))
        return newStatement
        
    # localUse means, that errors here shouldn't be reported, because it's 
    # called by other parser, who will report error if needed.
    def __parseStatement(self, code, localUse = False):
        code, comment = self.__findComments(code)
        res = self.__statRegex.match(code)
        if res == None:
            if not localUse:
                self.API.logMsg(g.WARNING, "Syntax error: \"" + code + "\"")
            return None
        # If there is no parameters, object is in second group, 
        # else it is in first, and parameters are in second
        newStatement = Statement.Statement(res.group(1), res.group(2))
        self.__getQueuedComments(newStatement)
        newStatement.setInlineComment(comment)
        for pair in res.group(3).split(","):
            data = pair.strip(" ,").split(" ")
            # We do not check for empty parameter, because it is checked
            # by Statement class
            if len(data) == 1:
                newStatement.addParameterByNameAndValue(data[0], True)
            else:
                newStatement.addParameterByNameAndValue(data[0], data[1])
        if not localUse:
            self.API.logMsg(g.INFO,"Parsed: \n" +  newStatement.getCode(''))
        return newStatement
        
    def __parseSimpleWhen(self, code):
        print "Simple when\t\t", code
    
        