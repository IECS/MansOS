'''
Created on 2011. gada 14. dec.

@author: Janis Judvaitis
'''
import Parameter
import globals as g

class Statement():
    def __init__(self, mode = '', obj = ''):
        self.__mode = mode.strip()
        self.__obj = obj.strip()
        self.__param = []
        self.__comment = ''
        self.__inlineComment = ''
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
    
    def addComment(self, comment):
        self.__comment = (self.__comment + '\n' + comment).strip()
        
    def getComment(self):
        return self.__comment
    
    def setInlineComment(self, inlineComment):
        self.__inlineComment = inlineComment.strip()
    
    def getInlineComment(self):
        return self.__inlineComment
    
    def setCondition(self, condition):
        self.__condition = condition.strip()
        
    def getCondition(self):
        return self.__condition
    
    def getIdentifier(self):
        return self.__identifier
    
    def getCode(self, prefix):
        result = (self.getComment() + '\n').replace('\n', '\n' + prefix)
        result += self.getModeAndObject()
        for param in self.getParam():
            result += ', ' + param.getCode()
        if self.getCondition() != '':
            result += ', when ' + self.getCondition()
        result += '; ' + self.getInlineComment() + '\n'
        return result.strip()
    
