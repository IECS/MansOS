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

class Parameter():
    def __init__(self, name = '', value = None):
        self.__name = name.strip()
        self.setValue(value)
    
    def setName(self, name):
        self.__name = name.strip()
    
    def getName(self):
        return self.__name
    
    def setValue(self, value):
        # For check boxes
        if type(value) is bool:
            self.__value = value
        # For select boxes
        else:
            self.__value = None if value == None else value.strip()
    
    def getValue(self):
        if type(self.__value) is bool:
            return ''
        return '' if self.__value == None else self.__value
        
    # Return parameter value(real, without preparing for output)
    def getRealValue(self):
        return self.__value
    
    # Return parameter name and value
    def getCode(self):
        if type(self.__value) is bool:
            if self.__value == True:
                return self.getName()
            else:
                return ''
        return (self.getName() + ' ' +self.getValue()).strip()
    
# Class for parameter definitions, differs from parameter class, 
# because this supports list as value and is not getCode() friendly
class ParameterDefinition():
    def __init__(self, name = '', value = ''):
        self.__name = name.strip()
        self.__value = value
    
    # Set parameter name
    def setName(self, name):
        self.__name = name.strip()
    
    # Return parameter name
    def getName(self):
        return self.__name
    
    # Set parameter value
    def setValue(self, value):
        self.__value = value
    
    # Return parameter value
    def getValue(self):
        return self.__value
    
    # Return parameter name and value
    def getAll(self):
        return [self.getName(), self.getValue()]