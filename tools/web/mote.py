import serial, subprocess, sys
from settings import *

#SLOW = "--slow"
SLOW = ""

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
        self.performAction = False
        self.buffer = ""
        self.writebuffer = ""
        self.platform = "telosb"

    def openSerial(self):
        if not self.performAction: return
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

    def tryToOpenSerial(self):
        if not self.port:
            self.performAction = True
            self.openSerial()

    def tryRead(self, binaryToo):
        numRead = 0
        if self.port:
            while self.port.inWaiting():
                c = self.port.read(1)
                if binaryToo or isascii(c):
                    self.buffer += c
                    numRead += 1
        return numRead

    def tryToUpload(self, filename):
        self.tryToOpenSerial()
        if not self.port: return 0

        if self.platform == "telosb":
            bsl = settingsInstance.getCfgValue("pathToMansOS") + "/mos/make/scripts/tos-bsl"
            platformArgs = ["--telosb"]
        else:
            bsl = settingsInstance.getCfgValue("pathToMansOS") + "/mos/make/scripts/bsl.py"
            platformArgs = ["--invert-reset", "--invert-test"]

        arglist = ["python", bsl, "-c", self.port.portstr, "-r", "-e", "-I", "-p", filename]
        argist.extend(platformArgs)
        if SLOW: arglist.append(SLOW)

        try:
            retcode = subprocess.call(" ".join(arglist), shell=True)
        except OSError as e:
            sys.stderr.write("execution failed: {}".format(str(e)))
            retcode = 1

        return retcode

    def tryToCompileAndUpload(self, filename):
        self.tryToOpenSerial()
        if not self.port: return 0

        arglist = ["make", "telosb", "upload"]
        try:
            retcode = subprocess.call(" ".join(arglist), shell=True)
        except OSError as e:
            sys.stderr.write("execution failed: {}".format(str(e)))
            retcode = 1

        return retcode
