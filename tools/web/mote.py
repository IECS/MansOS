#
# MansOS web server - mote handler class
#

import serial, subprocess, sys
from serial.tools import list_ports
from settings import *


def isascii(c, printable = True):
    if 0x00 <= ord(c) <= 0x7f:
        if 0x20 <= ord(c) <= 0x7e:
            return True
        if c == '\r' or c == '\n' or c == '\t':
            return True
        return False
    else:
        return False


class Mote(object):
    counter = 0
    def __init__(self, portName):
        self.number = Mote.counter
        Mote.counter += 1
        self.portName = portName
        self.port = None
        self.isSelected = False
        self.isStatic = False
        self.buffer = ""
        self.writebuffer = ""
        self.platform = "telosb"

    def openSerial(self):
        if not self.isSelected: return
        try:
            baudrate = settingsInstance.getCfgValueAsInt("baudrate", SERIAL_BAUDRATE)
            self.port = serial.Serial(self.portName,
                                      baudrate,
                                      timeout=1,
                                      parity=serial.PARITY_NONE)
            self.port.flushInput()
            self.port.flushOutput()
            if self.platform not in ['xm1000', 'z1'] :
                # make sure reset pin is low for the platforms that need it
                self.port.setDTR(0)
                self.port.setRTS(0)
        except Exception, e:
            print "\nSerial exception:\n\t", e
            self.port = None
            return

        print "Listening to serial port: ", self.port.portstr, ", rate: ", baudrate

    def closeSerial(self):
        if self.port:
            self.port.close()
            self.port = None

    def tryToOpenSerial(self, makeSelected):
        if not self.port:
            if makeSelected: self.isSelected = True
            self.openSerial()

    def tryRead(self, binaryToo):
        numRead = 0
        if self.port:
            while self.port.inWaiting():
                c = self.port.read(1)

                # save to file if required
                if settingsInstance.cfg.saveToFilename \
                        and not settingsInstance.cfg.saveProcessedData:
                    with open(settingsInstance.cfg.saveToFilename, "a") as f:
                        f.write(c)
                        f.close()

                # add to the local buffer
                if binaryToo or isascii(c):
                    self.buffer += c
                    numRead += 1

        return numRead

    def tryToUpload(self, filename):
        self.tryToOpenSerial(False)
        if not self.port: return 0

        if self.platform == "telosb":
            bsl = "mos/make/scripts/tos-bsl"
            platformArgs = ["--telosb"]
        elif self.platform == "xm1000":
            bsl = "mos/make/scripts/xm1000-bsl"
            platformArgs = ["--telosb"] # "telosb" is still the right arg
        elif self.platform == "z1":
            bsl = "mos/make/scripts/z1-bsl-nopic"
            platformArgs = ["--z1"]
        else:
            bsl = "mos/make/scripts/bsl.py"
            platformArgs = ["--invert-reset", "--invert-test"]

        bsl = os.path.join(settingsInstance.getCfgValue("pathToMansOS"), bsl)
        arglist = ["python", bsl, "-c", self.port.portstr, "-r", "-e", "-I", "-p", filename]
        argist.extend(platformArgs)
        if settingsInstance.getCfgValueAsInt("slowUpload"):
            arglist.append("--slow")

        try:
            retcode = subprocess.call(" ".join(arglist), shell=True)
        except OSError as e:
            sys.stderr.write("execution failed: {}".format(str(e)))
            retcode = 1

        return retcode

    def tryToCompileAndUpload(self, filename):
        self.tryToOpenSerial(False)
        if not self.port: return 0

        arglist = ["make", "telosb", "upload"]
        try:
            retcode = subprocess.call(" ".join(arglist), shell=True)
        except OSError as e:
            sys.stderr.write("execution failed: {}".format(str(e)))
            retcode = 1

        return retcode


class MoteCollection(object):
    def __init__(self):
        self.motes = []

    def storeSelected(self):
        selected = []
        for m in self.motes:
            if m.isSelected:
                selected.append(m.portName)
        settingsInstance.setCfgValue("selectedMotes", selected)
        settingsInstance.save()

    def retrieveSelected(self):
        selected = settingsInstance.getCfgValue("selectedMotes")
        for m in self.motes:
            m.isSelected = m.portName in selected

    def addAll(self):
        self.motes = []
        print "static ports are", settingsInstance.getCfgValue("motes")
        staticPorts = set(settingsInstance.getCfgValue("motes"))

        dynamicPorts = set()
        for x in list_ports.comports():
            # skip ports that are not connected
            if x[2] == "n/a": continue
            portName = x[0]
            if not ("USB" in portName or "ACM" in portName): continue
            dynamicPorts.add(portName)

        allPorts = dynamicPorts.union(staticPorts)
        for port in allPorts:
            print "add mote", port
            self.motes.append(Mote(port))

        self.retrieveSelected()

    def getMotes(self):
        return self.motes

    def getMote(self, num):
        return self.motes[num]

    def isEmpty(self):
        return len(self.motes) == 0
