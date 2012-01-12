import re
import string
import sealStruct 
import Parameter
import translater
import cPickle
import os

class ApiCore:
    def __init__(self):
        # Setting file name
        self.__settingFile = ".SEAL"
        if os.path.exists(self.__settingFile) & os.path.isfile(self.__settingFile):
            f = open(self.__settingFile, 'r')
            self.__settings = cPickle.load(f)
            f.close()
        else:
            # All variables placed here is saved to configuration file and 
            # reloaded next run time. See setSetting and getSetting.
            self.__settings = {
                   "activeLanguage" : "LV"
               }
        # Actuator roles
        self.STATEMENT = 0 # such as use, read, output
        self.CONDITION_START = 1 # such as when
        self.CONDITION_CONTINUE = 2 # such as else
        self.CONDITION_END = 3 # such as end
        self.IGNORE = 4 # such as comments, empty lines etc.

        # All defined platforms
        self.__platforms = ["telosb", "sadmote", "atmega", "waspmote"]
        
        # All actuators and other keywords goes here
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
        
        self.translater = translater.Translater(self)
        
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
        # Return empty object
        return {
                'objects': [],
                'parameters': [],
                'role': self.IGNORE
                }
    
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
    
    def getSetting(self, setting):
        if setting in self.__settings:
            return self.__settings[setting]
        return ''
    
    def setSetting(self, name, value):
        self.__settings[name] = value
        
    def saveSettings(self):
        f = open(self.__settingFile, 'w')
        cPickle.dump(self.__settings, f)
        f.close()
