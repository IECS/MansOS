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

import Parameter
import globals as g
import comment

class Statement():
    def __init__(self, mode = '', obj = ''):
        self.__mode = mode.strip()
        self.__obj = obj.strip()
        self.__param = []
        self.__comment = comment.Comment()
        self.__condition = ''
        self.__identifier = g.STATEMENT
        
    def setMode(self, mode):
        self.__mode = mode.strip()
    
    def getMode(self):
        return self.__mode
    
    def setObject(self, obj):
        self.__obj = obj.strip()
        
    def getObject(self):
        return self.__obj
    
    def getModeAndObject(self):
        return self.getMode() + " " + self.getObject()
    
    # Overwrites all parameters!!
    def setParameters(self, parameters):
        self.__param = parameters
    
    # This automatically overwrites old parameter with same name, so be careful
    def addParameter(self, parameter):
        name = parameter.getName()
        # Forbid parameters with no name
        if name == '':
            return
        for x in self.__param:
            if x.getName() == name:
                x.setValue(parameter.getRealValue())
                return
        self.__param.append(parameter)
        
    # This automatically overwrites old parameter with same name, so be careful
    def addParameterByNameAndValue(self, name, value):
        self.addParameter(Parameter.Parameter(name, value))
        
    def getParam(self):
        return self.__param
    
    def getParamValueByName(self, name):
        for x in self.__param:
            if x.getName() == name:
                return x.getRealValue()
        return None
    
    def getComment(self):
        return self.__comment
    
    def setComment(self, comment):
        self.__comment = comment
    
    def setCondition(self, condition):
        self.__condition = condition.strip()
        
    def getCondition(self):
        return self.__condition
    
    def getIdentifier(self):
        return self.__identifier
    
    def getCode(self, prefix):
        result = self.getComment().getPreComments(prefix) + '\n'
        result += prefix + self.getModeAndObject()
        for param in self.getParam():
            result += ', ' + param.getCode()
        if self.getCondition() != '':
            result += ', when ' + self.getCondition()
        result += '; ' + self.getComment().getPostComment(True) + '\n'
        return result.strip("\n").replace("\n\n","\n")
    
