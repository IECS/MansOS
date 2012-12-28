import time

class GraphData(object):
    def __init__(self):
        self.columns = []
        self.data = []
        self.tempData = []

    def finishPacket(self):
        if len(self.tempData) == 0:
            return
        # print "finishPacket"
        if len(self.columns) == 0:
            self.columns.append("timestamp")
            for (n,v) in self.tempData:
                if n.lower() != "timestamp":
                    self.columns.append(n)
        # print "self.columns", self.columns

        if len(self.tempData) != len(self.columns):
            self.tempData = []
            return

        tmpRow = []
        timestamp = time.strftime("%H:%M:%S", time.localtime())
        tmpRow.append(timestamp)
        for (n,v) in self.tempData:
            if n.lower() != "timestamp":
                tmpRow.append(v)
        #print "tmpRow", tmpRow
        self.data.append(tmpRow)
        self.tempData = []

    def addNewData(self, string):
        # print "addNewData", string
        eqSignPos = string.find('=')
        if eqSignPos == -1: return

        if eqSignPos == 0:
            self.finishPacket()
            return

        dataName = string[:eqSignPos].strip()
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


graphData = GraphData()
