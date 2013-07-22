#
# MansOS web server - data parsing, storing and visualization
#

import time, os
import configuration

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


# -------------------------------------
class SensorData(object):
    def __init__(self, motename):
        self.columns = []
        self.data = {}
        self.tempData = []
        self.seenInThisPacket = set()
        self.firstPacket = True
        baseDir = os.path.basename(motename)
        if baseDir[:3].lower() == "com":
            baseDir = "_" + baseDir
        self.dirname = os.path.join(configuration.c.getCfgValue("dataDirectory"),
                                    baseDir)
        if not os.path.exists(self.dirname):
            os.makedirs(self.dirname)

    def addNewData(self, string, motename):
        if motename[:5].lower() == "/dev/":
            motename = motename[5:]
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
        
        if not dataName in self.seenInThisPacket:
            self.seenInThisPacket.add(dataName)
            self.data[dataName + "@" + motename] = []

        valueString = string[eqSignPos + 1:].strip()

        try:
            # try to parse the value as int (in any base)
            value = int(valueString, 0)
        except:
            try:
                # try to parse the value as float
                value = float(valueString)
            except:
                print("Sensor " + dataName + " value is in unknown format: " + valueString + "\n")
                value = 0
        self.data[dataName + "@" + motename].append([int(round(time.time()*1000)),value])#miliseconds since 1970
        # save to file if required (multiple files)
        if configuration.c.getCfgValue("saveToFilename") \
                and configuration.c.getCfgValue("saveProcessedData"):

            # one more sanity check of dataName
            if len(dataName) > 64:
                return

            # filename is determined by config + mote name + sensor name
            filename = os.path.join(self.dirname,
                                    dataName + ".csv")
            with open(filename, "a") as f:
                if os.path.getsize(filename) == 0:
                    f.write("serverTimestampUnix\tserverTimestamp\t" + dataName + "\n")
                f.write("{}\t{}\t{}\n".format(int(round(time.time())),
                    time.strftime("%d %b %Y %H:%M:%S", time.localtime()),
                    value))
                f.close()


    def resize(self, newMaxSize):
        for datalist in self.data.keys():
            self.data[datalist] = self.data[datalist][-newMaxSize:]

    def reset(self):
        self.resize(0)
        self.columns = []
        self.firstPacket = True

    def getColumns(self):
        return self.columns

    def getRows(self):
        result = []
        for d in self.data:
            result.append(d[1:])
        return result

    # return all sensor readings, except unix timestamp
    def getData(self):
        return self.data
        #return [self.getColumns()[1:]] + self.getRows()

    def hasData(self):
        return len(self.data) != 0


# -------------------------------------
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

        # if the new string contains address of a data, use it instead of mote's name!
        columnPos = newString.find(":")
        eqPos = newString.find("=")
        # if the string has both ":" and "=", and "=" is before ":"
        if columnPos != -1 and columnPos < eqPos:
            address = newString.split(":")[0]
            if address:
                # use the address instead!
                motename = address
                newString = newString[columnPos + 1:]

        # if the new string contains checksum, check it.
        if len(newString) > 3 and newString.find(",") == len(newString) - 3:
            # checksum detected
            calcCrc = crc8(newString[:-3])
            recvCrc = int(newString[-2:], 16) 
            if calcCrc != recvCrc:
                print("Received bad checksum:\n" + newString)
                return

            # remove the crc bytes from the value string
            newString = newString[:-3]

        if motename not in self.data:
            self.data[motename] = SensorData(motename)
        self.data[motename].addNewData(newString, motename)

    def fixSizes(self):
        # use only last 27 lines of all motes - fits in screen ("listen_div")
        self.listenTxt = self.listenTxt[-27:]
        # use only last 40 readings for graphing
        for sensorData in self.data.itervalues():
            sensorData.resize(40)

    def hasData(self):
        for sensorData in self.data.itervalues():
            if sensorData.hasData():
                return True
        return False

    # return all readings from the first sensor mote
    # TODO: allow the user to select which sensor to return!
    def getData(self):
        returnData = []
        for sensorData in self.data.itervalues():
            if sensorData.hasData():
                returnData.append(sensorData.getData())
        return returnData


# -------------------------------------
moteData = MoteData()
