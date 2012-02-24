'''
Created on 2012. gada 17. febr.

@author: Janis Judvaitis
'''
import globals as g

class condContainer:
    def __init__(self):
        # When and else hold condition instance who represents condition 
        self.__when = None
        self.__else = None
        # Holds array of condition instances representing elsewhen
        self.__elseWhen = []
        # Describe end statement
        self.__endComment = ''
        self.__endInlineComment = ''
        self.__oneLiner = False
        self.__identifier = g.CONDITION
    
    def setWhen(self, when):
        self.__when = when
        
    def getWhen(self):
        return self.__when
    
    def setElse(self, else_):
        self.__when = else_
        
    def getElse(self):
        return self.__else
    
    def addElseWhen(self, elseWhen):
        self.__elseWhen.append(elseWhen)
        
    def getElseWhen(self):
        return self.__elseWhen
    
    def addEndComment(self, comment):
        self.__endComment = (self.__endComment + '\n' + comment).strip()
    
    def getEndComment(self):
        return self.__endComment
    
    def setEndInlineComment(self, inlineComment):
        self.__endInlineComment = inlineComment.strip()
    
    def getEndInlineComment(self):
        return self.__endInlineComment
    
    def setOneLiner(self, oneLiner):
        self.__oneLiner = oneLiner
        
    def getOneLiner(self):
        return self.__oneLiner
    
    def getIdentifier(self):
        return self.__identifier
    
    def getCode(self, prefix):
        if self.getOneLiner() == True:
            return self.getWhen().getCode(prefix)
        result = self.getWhen().getCode(prefix) + '\n'
        for cond in self.getElseWhen():
            result += cond.getCode(prefix) + '\n'
        result += self.getElse().getCode(prefix) + '\n'
        result += (self.getEndComment() + '\n').replace('\n', '\n' + prefix)
        result += prefix +'end ' + self.getEndInlineComment()
        return result.strip()
