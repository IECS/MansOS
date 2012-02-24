'''
Created on 2011. gada 7. dec.

@author: Janis Judvaitis
'''
import sealParser

class Seal():
    def __init__(self, API, initCode = None):
        self.API = API
        self.sealParser = sealParser.SealParser(API)
        self.__parsedCode = self.sealParser.parseCode(initCode)
        
    
    def getFirstObject(self):
        return self.__parsedCode[0]
    
    def getPredictedType(self):
        return self.getFirstObject().getIdentifier()
        
    def getCode(self):
        result = ''
        for obj in self.getData():
            result += obj.getCode('') + '\n'
        return result
