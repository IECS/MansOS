'''
Created on 2011. gada 15. dec.

@author: Janis Judvaitis
'''

class Parameter():
    def __init__(self, parameterName = '', parameterValue = None):
        self.__parameterName = parameterName.strip()
        self.setValue(parameterValue)
    
    # Set parameter name
    def setName(self, parameterName):
        self.__parameterName = parameterName.strip()
    
    # Return parameter name
    def getName(self):
        return self.__parameterName
    
    # Set parameter value
    def setValue(self, parameterValue):
        
        # For check boxes
        if type(parameterValue) is bool:
            self.__parameterValue = parameterValue
        # For select boxes
        else:
            self.__parameterValue = None if parameterValue == None else parameterValue.strip()
        #print "Added", self.__parameterName,"->",self.__parameterValue
    
    # Return parameter value
    def getValue(self):
        if type(self.__parameterValue) is bool:
            return ''
        return '' if self.__parameterValue == None else self.__parameterValue
        
    # Return parameter value(real, without preparing for output)
    def getRealValue(self):
        return self.__parameterValue
    
    # Return parameter name and value
    def getAll(self):
        if type(self.__parameterValue) is bool:
            if self.__parameterValue == True:
                return self.getName()
            else:
                return ''
        return (self.getName() + ' ' +self.getValue()).strip()
    
# Class for parameter definitions, differs from parameter class, 
# because this supports list as value and is not getAll() friendly
class ParameterDefinition():
    def __init__(self, parameterName = '', parameterValue = ''):
        self.__parameterName = parameterName.strip()
        self.__parameterValue = parameterValue
    
    # Set parameter name
    def setName(self, parameterName):
        self.__parameterName = parameterName.strip()
    
    # Return parameter name
    def getName(self):
        return self.__parameterName
    
    # Set parameter value
    def setValue(self, parameterValue):
        self.__parameterValue = parameterValue
    
    # Return parameter value
    def getValue(self):
        return self.__parameterValue
    
    # Return parameter name and value
    def getAll(self):
        return (self.getName() + ' ' +self.getValue()).strip()