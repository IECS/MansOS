#
# MansOS web server - mote handler class
#

import serial, subprocess, sys, time
#from serial.tools import list_ports
from settings import *

def runSubprocess1(args, server):
    retcode = -1
    try:
        proc = subprocess.Popen(args, stderr = subprocess.STDOUT, stdout = subprocess.PIPE, shell = False)
        while proc.poll() is None:
            line = proc.stdout.readline()
            if line:
                if not server.uploadCallback(line):
                    break
            time.sleep(0.01)
        proc.wait()
        retcode = proc.returncode
    except OSError as e:
        print("runSubprocess1 OSError:" + str(e))
    except CalledProcessError as e:
        print("runSubprocess1 CalledProcessError:" + str(e))
        retcode = e.returncode
    except Exception as e:
        print("runSubprocess1 exception:" + str(e))
    finally:
        return retcode


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
        except Exception as e:
            print("\nSerial exception:\n\t" + str(e))
            self.port = None
            return

        print("Listening to serial port: " + self.port.portstr + ", rate: " + str(baudrate))

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
            try:
              while self.port.inWaiting():
                c = self.port.read(1)

                # save to file if required
                if settingsInstance.cfg.saveToFilename \
                        and not settingsInstance.cfg.saveProcessedData:
                    filename = settingsInstance.cfg.dataDirectory + "/" \
                        + settingsInstance.cfg.saveToFilename
                    with open(filename, "a") as f:
                        f.write(c)
                        f.close()

                # add to the local buffer
                if binaryToo or isascii(c):
                    self.buffer += c
                    numRead += 1
            except Exception as e:
              print("\nserial read exception:\t" + str(e))
              self.port.close()
              self.port = None

        return numRead

    def tryToUpload(self, server, filename):
        self.tryToOpenSerial(False)
        if not self.port: return 1

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

        retcode = runSubprocess1(arglist, server)

        return retcode

    def tryToCompileAndUpload(self, server, filename):
        self.tryToOpenSerial(False)
        if not self.port: return 1

        arglist = ["make", self.platform, "upload"]
        retcode = runSubprocess1(arglist, server)

        return retcode


class MoteCollection(object):
    def __init__(self):
        self.motes = []

    def storeSelected(self):
        selected = []
        platforms = []
        for m in self.motes:
            if m.isSelected:
                selected.append(m.portName)
            platforms.append(m.portName + ":" + m.platform)
        settingsInstance.setCfgValue("selectedMotes", selected)
        settingsInstance.setCfgValue("motePlatforms", platforms)
        settingsInstance.save()

    def retrieveSelected(self):
        selected = settingsInstance.getCfgValue("selectedMotes")
        platforms = settingsInstance.getCfgValue("motePlatforms")
        for m in self.motes:
            m.isSelected = m.portName in selected
        for p in platforms:
            try:
                (portName, platform) = p.split(':')
                for m in self.motes:
                    if m.portName == portName:
                        m.platform = platform
            except:
                pass

    def addAll(self):
        self.motes = []
        staticPorts = set(settingsInstance.getCfgValue("motes"))

        dynamicPorts = set()
#        for x in list_ports.comports():
#            # skip ports that are not connected
#            if x[2] == "n/a": continue
#            portName = x[0]
#            if not ("USB" in portName or "ACM" in portName): continue
#            dynamicPorts.add(portName)

        allPorts = dynamicPorts.union(staticPorts)
        for port in allPorts:
            self.motes.append(Mote(port))

        self.retrieveSelected()

    def getMotes(self):
        return self.motes

    def getMote(self, num):
        return self.motes[num]

    def isEmpty(self):
        return len(self.motes) == 0

    def anySelected(self):
        for m in self.motes:
            if m.isSelected: return True
        return False

    def closeAll(self):
        for m in self.motes:
            m.closeSerial()
