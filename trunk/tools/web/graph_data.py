#
# MansOS web server - data parsing, storing and visualization
#

import time, os
from settings import *

class GraphData(object):
    def __init__(self):
        self.columns = []
        self.data = []
        self.tempData = []
        self.seenInThisPacket = set()
        self.firstPacket = True

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
            filename = settingsInstance.cfg.dataDirectory + "/" \
                + settingsInstance.cfg.saveToFilename + ".csv"
            with open(filename, "a") as f:
                if self.firstPacket:
                    f.write(str(self.getColumns()))
                f.write(str(tmpRow))
                f.close()

    def addNewData(self, string):
        eqSignPos = string.find('=')
        if eqSignPos == -1: return

        if eqSignPos == 0:
            self.finishPacket()
            return

        dataName = string[:eqSignPos].strip().lower()
        if not isasciiString(dataName):
            return

        if dataName in self.seenInThisPacket:
            self.finishPacket()
        self.seenInThisPacket.add(dataName)

        try:
            value = int(string[eqSignPos + 1:].strip(), 0)
        except:
            value = 0
        self.tempData.append((dataName, value))

        # save to file if required (multiple files)
        if settingsInstance.cfg.saveToFilename \
                and settingsInstance.cfg.saveProcessedData \
                and settingsInstance.cfg.saveMultipleFiles:
            filename = settingsInstance.cfg.dataDirectory + "/" \
                + settingsInstance.cfg.saveToFilename + "_" + dataName
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


graphData = GraphData()
