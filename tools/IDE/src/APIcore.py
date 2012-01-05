import re
import string
import sealStruct 
import Parameter

class ApiCore:
    def __init__(self):
        # Actuator roles
        self.STATEMENT = 0 # such as use, read, output
        self.CONDITION_START = 1 # such as when
        self.CONDITION_CONTINUE = 2 # such as else
        self.CONDITION_END = 3 # such as end
        self.IGNORE = 4 # such as comments, empty lines etc.
        
        # All defined platforms
        self.__platforms = ["telosb", "sadmote", "atmega", "waspmote"]
        
        # Active editor number
        self.__activeEditorNr = 0
        
        # All editor pages currently opened
        self.__editors = []
        
        # All actuators and other keywords goes here
        # TODO: change parameters to Parameter objects, so their order is correct
        self.__actuators = {
            'use': {
                'objects': ['Led', 'RedLed','GreenLed','BlueLed'],
                'parameters': [
                    Parameter.ParameterDefinition('period', ['','100ms','200ms','500ms','1s','2s','5s']),
                    Parameter.ParameterDefinition('on_at', ['','100ms','200ms','500ms','1s','2s','5s']),
                    Parameter.ParameterDefinition('off_at', ['','100ms','200ms','500ms','1s','2s','5s']),
                    Parameter.ParameterDefinition('blinkTimes', ['','1', '2', '3', '5', '10', '25']),
                    Parameter.ParameterDefinition('blink', None),
                    Parameter.ParameterDefinition('blinkTwice', None),
                    Parameter.ParameterDefinition('turn_on', None),
                    Parameter.ParameterDefinition('turn_off', None),
                ],
                'role': self.STATEMENT
            },
            'read': {
                'objects': ['temperature', 'humidity'],
                'parameters': [
                    Parameter.ParameterDefinition('period', ['','100ms','200ms','500ms','1s','2s','5s'])
                ],
                'role': self.STATEMENT
            },
            'output': {
                'objects': ['serial', 'radio'],
                'parameters': [
                    Parameter.ParameterDefinition('aggregate', None),
                    Parameter.ParameterDefinition('crc', None),
                    Parameter.ParameterDefinition('baudrate', ['','2400', '4800', '9600', '19200', '38400', '57600', '115200']),
                ],
                'role': self.STATEMENT
            },
            'when': {
                'objects': ["System.time < 5s", "System.isDaytime"],
                'parameters': [],
                'role': self.CONDITION_START
            },
            'else': {
                'objects': [],
                'parameters': [],
                'role': self.CONDITION_CONTINUE
            },
            'end': {
                'objects': [],
                'parameters': [],
                'role': self.CONDITION_END
            }
        }
        # Compile regex for finding actuators
        self.__reActuators = re.compile(string.join(self.__actuators.keys(), '|'), re.I)
        
        self.seal = sealStruct.Seal(self)
        
    # Return regex for finding actuators
    def getReActuators(self):
        return self.__reActuators
    
    def getRole(self, actuator):
        return self.__actuators[actuator]['role']
    
    def getStatementType(self, line):
        
        actuator = line.split(None, 1)
        if actuator != []:
            if actuator[0] in self.__actuators:
                return self.__actuators[actuator[0]]['role']
        return self.IGNORE
    
    def getActuatorInfo(self, actuator):
        if actuator in self.__actuators:
            return self.__actuators[actuator]
        return None
    
    # Get all actuators, who have role == self.STATEMENT
    def getAllStatementActuators(self):
        result = []
        for x in self.__actuators.keys():
            if self.__actuators[x]['role'] == self.STATEMENT:
                result.append(x)
        return result
    
    def getDefaultConditions(self):
        return self.__actuators['when']['objects']
    
    def getPlatforms(self):
        return self.__platforms
    
    def addEditor(self, editor):
        self.__editors.append(editor)
        
    def getEditor(self, nr):
        return self.__editors[nr]
    
    def getActiveEditor(self):
        return self.getEditor(self.getActiveEditorNr())
    
    def setActiveEditorNr(self, nr):
        self.__activeEditorNr = nr
    
    def getActiveEditorNr(self):
        return self.__activeEditorNr