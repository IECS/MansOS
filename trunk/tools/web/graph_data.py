#
# MansOS web server - data parsing, storing and visualization
#

import time

class GraphData(object):
    def __init__(self):
        self.columns = []
        self.data = []
        self.tempData = []
        self.seenInThisPacket = set()

    def finishPacket(self):
        self.seenInThisPacket = set()
        if len(self.tempData) == 0:
            return
        # print "finishPacket"
        firstPacket = False
        if len(self.columns) == 0:
            firstPacket = True
            self.columns.append("serverTimestamp")
            for (n,v) in self.tempData:
                if n.lower() != "serverTimestamp":
                    self.columns.append(n)
        # print "self.columns", self.columns
        # print "self.data", self.tempData

        if len(self.tempData) + 1 != len(self.columns):
            self.tempData = []
            return

        tmpRow = []
        timestamp = time.strftime("%H:%M:%S", time.localtime())
        tmpRow.append(timestamp)
        for (n,v) in self.tempData:
            if n.lower() != "serverTimestamp":
                tmpRow.append(v)
        self.data.append(tmpRow)
        self.tempData = []

        # save to file if required
        if settingsInstance.saveToFilename \
                and settingsInstance.saveProcessedData:
            with open(settingsInstance.saveToFilename, "a") as f:
                if firstPacket:
                    f.write(str(self.getColumns()))
                f.write(str(tmpRow))
                f.close()

    def addNewData(self, string):
        # print "addNewData", string
        eqSignPos = string.find('=')
        if eqSignPos == -1: return

        if eqSignPos == 0:
            self.finishPacket()
            return

        dataName = string[:eqSignPos].strip()
        if dataName in self.seenInThisPacket:
            self.finishPacket()
        self.seenInThisPacket.add(dataName)

        try:
            value = int(string[eqSignPos + 1:].strip(), 0)
        except:
            value = 0
        self.tempData.append((dataName, value))

    def resize(self, newMaxSize):
        self.data = self.data[:newMaxSize]

    def reset(self):
        self.resize(0)
        self.columns = []

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
