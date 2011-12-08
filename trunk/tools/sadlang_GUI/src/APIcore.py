class ApiCore:
    def __init__(self):
        pass

    useObjects = {
                        "Led":None,
                        "RedLed": None,
                        "GreenLed": None,
                        "BlueLed": None
                }
    usePeriod = {
                        '100ms': 100,
                        '200ms': 200,
                        '500ms': 500,
                        '1s': 1000,
                        '2s': 2000,
                        '5s': 5000
                }
    useOnAt = {
                        '100ms': 100,
                        '200ms': 200,
                        '500ms': 500,
                        '1s': 1000,
                        '2s': 2000,
                        '5s': 5000
                }
    useOffAt = {
                        '100ms': 100,
                        '200ms': 200,
                        '500ms': 500,
                        '1s': 1000,
                        '2s': 2000,
                        '5s': 5000
                }
#    useTurnOn = {
#                        '100ms': 100,
#                        '200ms': 200,
#                        '500ms': 500,
#                        '1s': 1000,
#                        '2s': 2000,
#                        '5s': 5000
#                }
#    useTurnOff = {
#                        '100ms': 100,
#                        '200ms': 200,
#                        '500ms': 500,
#                        '1s': 1000,
#                        '2s': 2000,
#                        '5s': 5000
#                }
    useBlinkTimes = {
                        '1': 1,
                        '2': 2,
                        '3': 3,
                        '5': 5,
                        '10': 10,
                        '50': 50,
                }
    useParams = {
                        'period': usePeriod,
                        'on_time': useOnAt,
                        'off_time': useOffAt,
                        #"turn_on": None,
                        #"turn_off": None,
                        "blink": None,
                        "blinkTwice": None,
                        "blinkTimes": useBlinkTimes
                }
    readPeriod = {
                        '100ms': 100,
                        '200ms': 200,
                        '500ms': 500,
                        '1s': 1000,
                        '2s': 2000,
                        '5s': 5000
                }
    readParams = {
                        'period': readPeriod
                }
    readObjects = {
                        "Light": None,
                        "Humidity": None
                }
    sinkBaudrate = {
                        '38400': 38500,
                        '9600': 9600
                }
    sinkPrint = {
                        'In main loop...': 38500,
                        'param param...': 9600
                }
    sinkParams = {
                        "baudrate": sinkBaudrate,
                        #"print": sinkPrint,
                        "aggregate": None,
                        "crc":None
                }
    sinkObjects = {
                        "Serial": None,
                        "Radio": None
                }
    keyWords = {
                        "use": {
                                'obj': useObjects,
                                'param': useParams
                        },
                        "read": {
                                'obj': readObjects,
                                'param': readParams
                        },
                        "output": {
                                'obj': sinkObjects,
                                'param': sinkParams
                        },
                }
    defaultConditions = [
                        "System.time < 5s",
                        "System.isDaytime"
                ]
    activeConditions = {}
    
    conditionCount = -1
    
    platforms = ["telosb", "sadmote", "atmega", "waspmote"]
    
    path = ""
    
    
    def getPlatforms(self):
        return self.platforms
    
    def getObjects(self, keyWord):
        print self.keyWords[keyWord]['obj']
        retVal = []
        for x in self.keyWords[keyWord]['obj'].keys():
            retVal.append(x[0:])
        return retVal
    
    def getParams(self, keyWord):
        return self.keyWords[keyWord]['param']
    
    def addInst(self, keyWord, objName, obj):
        self.keyWords[keyWord]['obj'][objName] = obj
    
    def getDefaultConditions(self):
        return self.defaultConditions
    
    def getConditions(self):
        return self.activeConditions
    
    def addCondition(self):
        self.conditionCount += 1
        self.activeConditions[self.conditionCount] = {}
        return self.activeConditions[self.conditionCount]
    
    def getActionKeyWords(self):
        words = []
        for x in self.keyWords:
            for y in self.keyWords[x]['obj']:
                words.append(x + " " + y)
        return words
    
    def generateUseCmd(self, keyWord):
        cmd = "use " + keyWord
        print self.useObjects[keyWord]
        for x in self.useObjects[keyWord]:
            if self.useObjects[keyWord][x] == True:
                cmd += ", " + x
            elif self.useObjects[keyWord][x] == False:
                pass
            elif len(self.useObjects[keyWord][x]) != 0:
                cmd += ", " + x + " " + str(self.useObjects[keyWord][x])
        if ',' in cmd:
            return cmd + ";"
        else:
            return ""
        
    def generateReadCmd(self, keyWord):
        cmd = "read " + keyWord
        print self.readObjects[keyWord]
        for x in self.readObjects[keyWord]:
            if self.readObjects[keyWord][x] == True:
                cmd += ", " + x
            elif self.readObjects[keyWord][x] == False:
                pass
            elif len(self.readObjects[keyWord][x]) != 0:
                cmd += ", " + x + " " + str(self.readObjects[keyWord][x])
        if ',' in cmd:
            return cmd + ";"
        else:
            return ""
        
    def generateSinkCmd(self, keyWord):
        cmd = "output " + keyWord
        print self.sinkObjects[keyWord]
        for x in self.sinkObjects[keyWord]:
            if self.sinkObjects[keyWord][x] == True:
                cmd += ", " + x
            elif self.sinkObjects[keyWord][x] == False:
                pass
            elif len(self.sinkObjects[keyWord][x]) != 0:
                if x == "print":
                    cmd += ", " + x + ' "' + str(self.sinkObjects[keyWord][x])+'"'
                else:
                    cmd += ", " + x + " " + str(self.sinkObjects[keyWord][x])
        if ',' in cmd:
            return cmd + ";"
        else:
            return ""
    
    def generateCmd(self, data, keyWord):
        cmd = keyWord
        print data
        for x in data:
            if data[x] == True:
                cmd += ", " + x
            elif data[x] == False:
                pass
            elif len(data[x]) != 0:
                if x == "print":
                    cmd += ", " + x + ' "' + str(data[x])+'"'
                else:
                    cmd += ", " + x + " " + str(data[x])
        if ',' in cmd:
            return cmd + ";"
        else:
            return ""
        
    def printConditions(self):
        print self.activeConditions
    
    def generateAll(self):
        use = ""
        for x in self.useObjects:
            if self.useObjects[x] != None:
                use += self.generateUseCmd(x) + "\n"
        if ";" in use:
            use += "\n"
        read = ""
        for x in self.readObjects:
            if self.readObjects[x] != None:
                read += self.generateReadCmd(x) + "\n"
        if ";" in read:
            read += "\n"
        sink = ""
        for x in self.sinkObjects:
            if self.sinkObjects[x] != None:
                sink += self.generateSinkCmd(x) + "\n"
        if ";" in sink:
            sink += "\n"
        code = use + read + sink;
        for x in self.activeConditions:
            if "condition" not in self.activeConditions[x]:
                continue
            whenCode = "When " + self.activeConditions[x]['condition']+ ":\n"
            for z in self.activeConditions[x]['when']:
                if z != []:
                    whenCode += "    " + z[1]
                    if z[0] != None:
                        for q in z[0]:
                            if z[0][q] == True:
                                whenCode += ", "+ q
                            elif z[0][q] == False:
                                pass
                            elif len(z[0][q]) > 1:
                                whenCode += ", "+ q + " " + z[0][q]
                    whenCode += ";\n"
            elseCode = "else:\n"
            for z in self.activeConditions[x]['else']:
                if z != []:
                    elseCode += "    " + z[1]
                    if z[0] != None:
                        for q in z[0]:
                            if z[0][q] == True:
                                elseCode += ", "+ q
                            elif z[0][q] == False:
                                pass
                            elif len(z[0][q]) > 1:
                                elseCode += ", "+ q + " " + z[0][q]
                    elseCode += ";\n"
            code += whenCode
            if ";" in elseCode:
                code += elseCode
            code += "end\n\n"
        print code
        return code
