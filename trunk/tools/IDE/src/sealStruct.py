'''
Created on 2011. gada 7. dec.

@author: Janis Judvaitis
'''
import Statement
import Condition

class Seal():
    def __init__(self, API, initCode = None):
        self.API = API
        self.__config = {
            'statements': [],
            'constants': [],
            'conditions': []
        }
        
        self.activeState = self.API.STATEMENT
        self.processedCondition = Condition.Condition()
        self.comments = ''
        # Fill in data if any passed
        if initCode != None:
            self.parseCode(initCode)
        
    def parseCode(self, code):
        regex = self.API.getReActuators()
        # Split in lines
        lines = code.split('\n')
        # Split lines in statements, if more than one
        for line in lines:
            statement, comments = self.splitStatement(line)
            if comments != '':
                self.comments += comments + '\n'
            temp = statement.split(';')
            for candidate in temp:
                # Use regex
                res = regex.findall(candidate)
                if res:
                    self.parseStatement(candidate, res)
                elif candidate.strip() != "":
                    print "Unknown line, dropping:", candidate
                else:
                    #print "Empty line"
                    pass
        #print self.__config
        
    def parseStatement(self, statement, actuators):
        roles = {}
        for x in actuators:
            roles[x] = self.API.getRole(x)
        #print roles
        if len(roles) == 0:
            print "Nothing to do here!"
        elif len(roles) == 1:
            role = roles.values()[0]
            if role == self.API.STATEMENT:
                statementObject = self.generateStatementObject(statement, 
                                                               self.comments)
                self.comments = ''
                #print "Generated statement object"
                if self.activeState == self.API.STATEMENT:
                    self.addStatement(statementObject)
                    #print "Added statement"
                elif self.activeState == self.API.CONDITION_START:
                    self.processedCondition.addWhenStatement(statementObject)
                    #print "Added statement to when"
                elif self.activeState == self.API.CONDITION_CONTINUE:
                    self.processedCondition.addElseStatement(statementObject)
                    
                    #print "Added statement to else"
                else:
                    print "Error", self.processedCondition, statementObject, self.activeState
            elif role == self.API.CONDITION_START:
                self.activeState = role
                self.processedCondition = Condition.Condition(
                                statement[len(roles.keys()[0]):].strip(" :;"),
                                [], [], self.comments)
                self.comments = ''
                
                #print "Added condition when"
            elif role == self.API.CONDITION_CONTINUE:
                self.activeState = role
                #print "Added condition else"
            elif role == self.API.CONDITION_END:
                self.activeState = self.API.STATEMENT
                self.addCondition(self.processedCondition)
                self.processedObject = None
                #print "Finished condition "
            else:
                print "This shouldn't happen"
        elif len(roles) == 2:
            # Find where to split string
            lastIndex = max([statement.find(roles.keys()[0]),statement.find(roles.keys()[1])])
            # Split string in two parts
            firstPart = statement[:lastIndex]
            secondPart = statement[lastIndex:]
            
            # Find condition and statement parts
            condition = ''
            actuator = ''
            if firstPart.find(roles.keys()[1]) == -1:
                condition = secondPart[len(roles.keys()[1]):]
                actuator = firstPart
            else:
                condition = firstPart[len(roles.keys()[1]):]
                actuator = secondPart
            
            # Create Statement and Condition class objects
            statementObject = self.generateStatementObject(actuator, '')
            conditionObject = Condition.Condition(condition, [statementObject],
                                                  [], self.comments)
            self.comments = ''
            
            self.addCondition(conditionObject)
        else:
            print "This should never happen :D", roles
    
    # Generate statement object from statement
    def generateStatementObject(self, statement, comment):
        #print statement
        parts = statement.split(",")
        statementType, statementObject = parts[0].strip().split(" ", 1)
        #print statementType, statementObject
        # Create new Statment class object and fill in all values
        # weird, but can't miss [] as parameter and give comment by name, 
        # causes all parameters to be added to all statements
        result = Statement.Statement(statementType, statementObject, 
                                     [], comment)
        #print parts
        for x in parts[1:]:
            if x.strip() == '':
                continue
            temp = x.strip().split(None, 1)
            if len(temp) == 0:
                print "Fail!",temp, x
            elif temp[0] == '':
                print "Got empty statement:", statement, comment
            elif len(temp) == 1:
                result.addParameterByNameAndValue(temp[0], None)
            elif len(temp) == 2:
                result.addParameterByNameAndValue(temp[0], temp[1])
            else:
                print "This should never happen! :D", temp
        return result

    # Add given object to __config conditions
    def addCondition(self, conditionObject):
        for x in self.__config['conditions']:
            if x.getCondition() == conditionObject.getCondition():
                # Merge when statements
                for y in conditionObject.getWhenStatements():
                    x.addWhenStatement(y)
                # Merge else statements
                for y in conditionObject.getElseStatements():
                    x.addElseStatement(y)
                # Merge comments
                x.addComment(conditionObject.getComments())
                return
        self.__config['conditions'].append(conditionObject)

    # Add given object to __config statements(actuators, sensors, output, config).
    # Merge if similar object exists.
    def addStatement(self, statementObject):
        for x in self.__config['statements']:
            if x.getTypeAndObject() == statementObject.getTypeAndObject():
                parameters = statementObject.getStatementParameters()
                for y in parameters:
                    x.addParameter(y)
                x.addComment(statementObject.getComments())
                return
        self.__config['statements'].append(statementObject)
    
    # Split statement into code and comments, take ' and " in count
    def splitStatement(self, statement):
        i = 0
        bad1 = bad2 = 0
        while i < len(statement):
            if statement[i] == "'":
                bad1 += 1
            elif statement[i] == '"':
                bad2 += 1
            elif statement[i:i+2] == "//":
                if bad1%2 == 0 and bad2%2 == 0:
                    return statement[:i], statement[i:]
            i += 1
        return statement, ''
    
    # Generate all code from all objects
    def generateAllCode(self):
        return self.generateAllStatements() + self.generateAllConditions()
    
    # Generate all code from all statement objects
    def generateAllStatements(self):
        result = ''
        for x in self.__config['statements']:
            result += x.getAll() + '\n\n'
        return result
    
    # Generate all code from all condition objects
    def generateAllConditions(self):
        result = ''
        for x in self.__config['conditions']:
            result += x.getAll() + '\n'
        return result
    
    def getStruct(self):
        return self.__config
