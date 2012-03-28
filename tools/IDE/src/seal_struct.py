# -*- coding: utf-8 -*-
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
from structures import ComponentUseCase, CodeBlock
from globals import * #@UnusedWildImport

class SealStruct():
    def __init__(self, API, initCode = None, parser = None):
        self.API = API
        self.next = None
        if parser == None:
            self.sealParser = self.API.sealParser
        self.__parsedCode = self.sealParser.run(initCode)

    def getPredictedType(self):
        if type(self.__parsedCode.declarations[0]) == ComponentUseCase:
            return STATEMENT
        elif type(self.__parsedCode.declarations[0]) == CodeBlock:
            return CONDITION
        else:
            return UNKNOWN

    def getMode(self):
        if self.getPredictedType() == STATEMENT:
            return self.__parsedCode.declarations[0].type
        return ''

    def getObject(self):
        if self.getPredictedType() == STATEMENT:
            return self.__parsedCode.declarations[0].name
        return ''

    def getParamValueByName(self, name):
        if self.getPredictedType() == STATEMENT:
            for x in self.__parsedCode.declarations[0].parameters:
                if x[0].lower() == name.lower():
                    if x[1] == None:
                        return True
                    else:
                        return x[1].getCode()
        return None

    def getCondition(self):
        print self.__parsedCode.declarations
        if self.getPredictedType() == CONDITION:
            return self.__parsedCode.declarations[0].condition.getCode()
        return ''

    def getNextCondition(self):
        if self.next == None:
            self.next = self.__parsedCode.declarations[0].next
        else:
            self.next = self.next.next

        if self.next.condition is not None:
            return self.next.condition.getCode()
        else:
            return None

    def getCode(self):
        return self.__parsedCode.getCode(0)
