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
    def __init__(self, portName):
        self.portName = portName
        self.port = None
        self.performAction = False
        self.buffer = ""
        self.writebuffer = ""

    def openSerial(self):
        if not self.performAction: return
        try:
            baudrate = settingsInstance.getCfgValueAsInt("baudrate", SERIAL_BAUDRATE)
            self.port = serial.Serial(self.portName,
                                  baudrate,
                                  timeout=1,
                                  parity=serial.PARITY_NONE,
                                  rtscts=1)
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


    def tryRead(self):
        if self.port:
            while self.port.inWaiting():
                c = self.port.read(1)
                if isascii(c):
                    self.buffer += c

    def tryToUpload(self, filename):
        self.performAction = True #XXX
        self.openSerial()
        if not self.port: return 0

        # TODO: this should be platform specific!
        # bsl = settingsInstance.getCfgValue("pathToMansOS") + "/mos/make/scripts/bsl.py"
        # arglist = ["python", bsl, "--invert-reset", "--invert-test", "-c", self.port.portstr, "-r", "-e", "-I", "-p", filename]

        bsl = settingsInstance.getCfgValue("pathToMansOS") + "/mos/make/scripts/tos-bsl"
        arglist = ["python", bsl, "--telosb", "-c", self.port.portstr, "-r", "-e", "-I", "-p", filename]
        if SLOW: arglist.append(SLOW)

        try:
            retcode = subprocess.call(" ".join(arglist), shell=True)
        except OSError as e:
            sys.stderr.write("execution failed: {}".format(str(e)))
            retcode = 1

        return retcode

    def tryToCompileAndUpload(self, filename):
        self.performAction = True #XXX
        self.openSerial()
        if not self.port: return 0

        arglist = ["make", "telosb", "upload"]
        try:
            retcode = subprocess.call(" ".join(arglist), shell=True)
        except OSError as e:
            sys.stderr.write("execution failed: {}".format(str(e)))
            retcode = 1

        return retcode
