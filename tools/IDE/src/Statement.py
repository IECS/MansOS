'''
Created on 2011. gada 14. dec.

@author: Janis Judvaitis
'''
import Parameter

class Statement():
    def __init__(self, statementType = '', statementObject = '', 
                 statementParameters = [], statementComment = ''):
        self.__statementType = statementType.strip()
        self.__statementObject = statementObject.strip()
        self.__statementParameters = statementParameters
        self.__statementComment = statementComment.strip()
        
    # Set statement type, examples: 'use', 'read', 'output'
    def setType(self, statementType):
        self.__statementType = statementType.strip()
    
    # Set statement object, examples: 'RedLed', 'Light', 'Radio'
    def setObject(self, statementObject):
        self.__statementObject = statementObject.strip()
    
    # Add statement parameters, example 'Period' -> '100ms', 'turn_on' -> None
    # This automatically overwrites old parameter with same name, so be careful
    def addParameterByNameAndValue(self, parameterName, parameterValue):
        self.addParameter(Parameter.Parameter(parameterName, parameterValue))
    
    # Add statement parameters
    # This automatically overwrites old parameter with same name, so be careful
    def addParameter(self, parameter):
        name = parameter.getName()
        for x in self.__statementParameters:
            if x.getName() == name:
                x.setValue(parameter.getRealValue())
                return
        self.__statementParameters.append(parameter)
        
    # Set statement comment, example: 'This statement rocks!'
    def addComment(self, statementComment):
        self.__statementComment += '\n' + statementComment
        # If new comment was empty we have too many newlines @ end, so...
        self.__statementComment = self.__statementComment.strip()
    
    # Return generated SEAL code from this statement
    def getCode(self):
        result = self.getTypeAndObject()
        for x in self.__statementParameters:
            if x.getRealValue() != False:
                result += ', ' + x.getAll()
        return result + ';'
    
    def getType(self):
        return self.__statementType
    
    
    def getObject(self):
        return self.__statementObject
    
    # Return comments associated with this statement
    def getComments(self):
        return self.__statementComment
    
    # Return generated SEAL code from this statement and
    # comments associated with this statement
    def getAll(self):
        return ('' if self.getComments() == ''  else self.getComments() + '\n') + self.getCode()
    
    # Get concatenation of type and object, example "Use RedLed", "read Light"
    def getTypeAndObject(self):
        return self.__statementType + " " + self.__statementObject
    
    # Return all statement parameters
    def getStatementParameters(self):
        return self.__statementParameters
    
    def getParamValueByName(self, parameterName):
        for x in self.__statementParameters:
            if x.getName() == parameterName:
                return x.getValue()
        return -1
