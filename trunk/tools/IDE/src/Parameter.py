'''
Created on 2011. gada 15. dec.

@author: Janis Judvaitis
'''

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