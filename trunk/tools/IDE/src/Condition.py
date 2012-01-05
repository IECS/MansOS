'''
Created on 2011. gada 14. dec.

@author: Janis Judvaitis
'''

class Condition():
    def __init__(self, condition = '', whenStatements = [], 
                 elseStatements = [], conditionComment = ''):
        self.__condition = condition.strip().strip(":")
        self.__whenStatements = whenStatements
        self.__elseStatements = elseStatements
        self.__conditionComment = conditionComment.strip()
        
    # Set statement condition, examples: '1', 'System.time < 5s'
    def setCondition(self, condition):
        self.__condition = condition.strip().strip(":")
    
    # Add when statement, pass statement class object
    # Check for similar statements and merge them if found
    def addWhenStatement(self, whenStatement):
        for x in self.__whenStatements:
            if x.getTypeAndObject() == whenStatement.getTypeAndObject():
                parameters = whenStatement.getStatementParameters()
                for y in parameters:
                    x.addParameter(y)
                x.addComment(whenStatement.getComments())
                return
        self.__whenStatements.append(whenStatement)
    
    # Add else statement, pass statement class object
    # Check for similar statements and merge them if found
    def addElseStatement(self, elseStatement):
        for x in self.__elseStatements:
            if x.getTypeAndObject() == elseStatement.getTypeAndObject():
                parameters = elseStatement.getStatementParameters()
                for y in parameters:
                    x.addParameter(y)
                x.addComment(elseStatement.getComments())
                return
        self.__elseStatements.append(elseStatement)
    
    # Set statement comment, example: 'This condition rocks!'
    def addComment(self, conditionComment):
        self.__conditionComment += '\n' + conditionComment.strip()
        # If new comment was empty we have too many newlines @ end, so...
        self.__conditionComment = self.__conditionComment.strip()
    
    # Return generated SEAL code from this statement
    def getCode(self, prefix = ''):
        result = prefix + 'when ' + self.getCondition() + ':\n'
        for x in self.__whenStatements:
            result += prefix + "\t" + x.getAll() + '\n'
        if self.__elseStatements != []:
            result += prefix + "else:\n"
            for x in self.__elseStatements:
                result += prefix + "\t" + x.getAll() + '\n'
        result += prefix + 'end\n'
        return result
    
    # Return comments associated with this statement
    def getComments(self, prefix = ''):
        if self.__conditionComment == '':
            return ''
        else:
            return prefix + self.__conditionComment + '\n'
    
    # Return generated SEAL code from this statement and
    # comments associated with this statement
    def getAll(self, prefix = ''):
        return self.getComments(prefix) + self.getCode(prefix)
    
    # Return condition
    def getCondition(self):
        return self.__condition
    
    # Return when statements
    def getWhenStatements(self):
        return self.__whenStatements
    
    def getWhenStatementsCode(self):
        result = ''
        for x in self.__whenStatements:
            result += x.getAll() + '\n'
        return result
        
    # Return else statements
    def getElseStatements(self):
        return self.__elseStatements
    
    def getElseStatementsCode(self):
        result = ''
        for x in self.__elseStatements:
            result += x.getAll() + '\n'
        return result
    