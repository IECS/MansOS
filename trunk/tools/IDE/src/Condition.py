'''
Created on 2011. gada 14. dec.

@author: Janis Judvaitis
'''

class Condition():
    def __init__(self, mode):
        # when, elsewhen or else
        self.__mode = mode
        # else holds empty condition
        self.__condition = ''
        # Statement instances
        self.__statements = []
        # Comments
        self.__comment = ''
        self.__inlineComment = ''
        
    def setMode(self, mode):
        self.__mode = mode
        
    def getMode(self):
        return self.__mode
    
    def setCondition(self, condition):
        self.__condition = condition
        
    def getCondition(self):
        return self.__condition
    
    def addStatement(self, statement):
        self.__statements.append(statement)
        
    def getStatements(self):
        return self.__statements
    
    def addComment(self, comment):
        self.__comment = (self.__comment + '\n' + comment).strip()
    
    def getComment(self):
        return self.__comment
    
    def setInlineComment(self, comment):
        self.__inlineComment = comment.strip()
    
    def getInlineComment(self):
        return self.__inlineComment
    
    def getCode(self, prefix):
        result = (self.getComment() + '\n').replace('\n', '\n' + prefix)
        result += prefix + self.getMode() + ' ' + self.getCondition() + ': '
        result += self.getInlineComment() + '\n'
        for statement in self.getStatements():
            result +='\t' + statement.getCode(prefix) + '\n'
        return result.strip()
        