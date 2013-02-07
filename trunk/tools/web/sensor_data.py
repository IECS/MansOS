#
# MansOS web server - data parsing, storing and visualization
#

import time, os
from settings import *

# Polynomial ^8 + ^5 + ^4 + 1
def crc8Add(acc, byte):
    acc ^= byte
    for i in range(8):
        if acc & 1:
            acc = (acc >> 1) ^ 0x8c
        else:
            acc >>= 1
    return acc

def crc8(s):
    acc = 0
    for c in s:
        acc = crc8Add(acc, ord(c))
    return acc

###############################################

class SensorData(object):
    def __init__(self, motename):
        self.columns = []
        self.data = []
        self.tempData = []
        self.seenInThisPacket = set()
        self.firstPacket = True
        self.dirname = os.path.join(settingsInstance.cfg.dataDirectory,
                                    os.path.basename(motename))
        if not os.path.exists(self.dirname):
            os.makedirs(self.dirname)

    def finishPacket(self):
        self.seenInThisPacket = set()
        if len(self.tempData) == 0:
            return

        if len(self.columns) == 0:
            self.columns.append("serverTimestamp")
            for (n,v) in self.tempData:
                if n.lower() != "serverTimestamp":
                    self.columns.append(n)
        else:
            self.firstPacket = False

        if len(self.tempData) + 1 != len(self.columns):
            self.tempData = []
            return

        tmpRow = []
        timestamp = time.strftime("%d %b %Y %H:%M:%S", time.localtime())
        tmpRow.append(timestamp)
        for (n,v) in self.tempData:
            if n.lower() != "serverTimestamp":
                tmpRow.append(v)
        self.data.append(tmpRow)
        self.tempData = []

        # save to file if required (single file)
        if settingsInstance.cfg.saveToFilename \
                and settingsInstance.cfg.saveProcessedData \
                and not settingsInstance.cfg.saveMultipleFiles:
            # filename is determined by config and mote name only
            filename = os.path.join(self.dirname,
                                    settingsInstance.cfg.saveToFilename + ".csv")
            with open(filename, "a") as f:
                if self.firstPacket:
                    f.write(str(self.getColumns()))
                f.write(str(tmpRow))
                f.close()

    def addNewData(self, string):
        string = string.rstrip()
        eqSignPos = string.find('=')
        if eqSignPos == -1: return

        if eqSignPos == 0:
            self.finishPacket()
            return

        dataName = string[:eqSignPos].strip().lower()
        # sanity check of dataName
        if not isasciiString(dataName):
            return

        if dataName in self.seenInThisPacket:
            self.finishPacket()
        self.seenInThisPacket.add(dataName)

        valueString = string[eqSignPos + 1:].strip()

        if len(valueString) > 3 and valueString.find(",") == len(valueString) - 3:
            # checksum detected
            calcCrc = crc8(string[:-3])
            recvCrc = int(valueString[-2:], 16) 
            if calcCrc != recvCrc:
                print("Received bad checksum:\n" + string)
                return

            # remove the crc bytes from the value string
            valueString = valueString[:-3]

        try:
            # try to parse the vakue as int (in any base)
            value = int(valueString, 0)
        except:
            try:
                # try to parse the value as float
                value = float(valueString)
            except:
                print("Sensor " + dataName + " value is in unknown format: " + value + "\n")
                value = 0

        self.tempData.append((dataName, value))

        # save to file if required (multiple files)
        if settingsInstance.cfg.saveToFilename \
                and settingsInstance.cfg.saveProcessedData \
                and settingsInstance.cfg.saveMultipleFiles:

            # one more sanity check of dataName
            if len(dataName) > 64:
                return

            # filename is determind by config + mote name + sensor name
            filename = os.path.join(self.dirname,
                                    dataName + ".csv")
            with open(filename, "a") as f:
                if os.path.getsize(filename) == 0:
                    f.write(dataName + ",serverTimestamp\n")
                f.write("{}, {}\n".format(value, time.strftime("%d %b %Y %H:%M:%S", time.localtime())))
                f.close()


    def resize(self, newMaxSize):
        self.data = self.data[:newMaxSize]

    def reset(self):
        self.resize(0)
        self.columns = []
        self.firstPacket = True

    def getColumns(self):
        return self.columns

    def getRows(self):
        return self.data

    # return all sensor readings 
    def getData(self):
        return [self.getColumns()] + self.getRows()

    def hasData(self):
        return len(self.columns) != 0

#############################################

class MoteData(object):
    def __init__(self):
        # unformatted data
        self.listenTxt = []
        # parsed and formatted data
        self.data = {}

    def reset(self):
        self.listenTxt = []
        self.data = {}

    def addNewData(self, newString, motename):
        self.listenTxt.append(newString)
        
        # TODO: if the new string contains address of a data, use it instead of mote's name!
        
        if motename not in self.data:
            self.data[motename] = SensorData(motename)
        self.data[motename].addNewData(newString)

    def fixSizes(self):
        # use only last 30 lines of all motes
        self.listenTxt = self.listenTxt[-30:]
        # use only last 40 readings for graphing
        for sensorData in self.data.itervalues():
            sensorData.resize(40)

moteData = MoteData()
