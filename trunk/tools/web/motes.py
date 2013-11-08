#
# MansOS web server - mote handler class
#

from __future__ import print_function
import serial, subprocess, sys, time, os, threading
from motelist import Motelist
import configuration
import utils

def runSubprocess1(args, server):
#    print("runSubprocess1: " + ",".join(args))
    retcode = -1
    try:
        try:
            outFileName = os.path.join("build", "child_output.txt")
            outFile = open(outFileName, "ab")
        except:
            outFile = None
        proc = subprocess.Popen(args, stderr = subprocess.STDOUT, stdout = outFile,
                                shell = False)
        proc.wait()
        if outFile: outFile.close()
#        print("proc finished, retcode={}".format(proc.returncode))
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
    def __init__(self, moteDescription):
        self.moteDescription = moteDescription
        self.port = None
        self.isSelected = False
        self.buffer = ""
        self.platform = "telosb"
        self.bufferLock = threading.Lock()
        self.portLock = threading.Lock()
        baseDir = os.path.basename(moteDescription.getPort())
        if baseDir[:3].lower() == "com":
            # Windows does not allow to create files named "com0", "com1" etc.
            baseDir = "_" + baseDir
        dirname = os.path.join(configuration.c.getCfgValue("dataDirectory"), baseDir)
        if not os.path.exists(dirname):
            os.makedirs(dirname)

    def getPortName(self):
        return self.moteDescription.getPort()

    def getPortBasename(self):
        return os.path.basename(self.getPortName())

    def openSerial(self):
        try:
            baudrate = configuration.c.getCfgValueAsInt("baudrate")
            self.port = serial.Serial(self.moteDescription.getPort(),
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

    def tryToOpenSerial(self, makeSelected):
        with self.portLock:
            if not self.port:
                if makeSelected: self.isSelected = True
                if self.isSelected: self.openSerial()
            result = self.port is not None
        return result

    def ensureSerialIsClosed(self):
        with self.portLock:
            tmp = self.port
            self.port = None
            if tmp:
                tmp.close()
                print("Serial port " + self.moteDescription.getPort() + " closed")

    def tryRead(self, binaryToo):
        if not self.port: return 0

        numRead = 0
        with self.portLock:
            self.bufferLock.acquire()
            try:
              while self.port.inWaiting():
                c = self.port.read(1)

                # save to file if required (raw data)
                if configuration.c.getCfgValue("saveToFilename") \
                        and not configuration.c.getCfgValueAsBool("saveProcessedData"):
                    if self.moteDescription.getPort().startswith("/dev/"):
                         filename = os.path.join(configuration.c.getCfgValue("dataDirectory"),
                                                self.moteDescription.getPort()[5:],
                                                configuration.c.getCfgValue("saveToFilename"))                      
                    else: 
                        filename = os.path.join(configuration.c.getCfgValue("dataDirectory"),
                                                self.moteDescription.getPort(),
                                                configuration.c.getCfgValue("saveToFilename"))
                    with open(filename, "a") as f:
                        f.write(c)
                        f.close()

                # add to the local buffer
                if binaryToo or utils.isascii(c):
                    self.buffer += c
                    numRead += 1
            except Exception as e:
                print("\nserial read exception:\t" + str(e))
                self.port.close()
                self.port = None
            finally:
                self.bufferLock.release()

        return numRead

    def tryToUpload(self, server, filename):
#        print("tryToUpload for " + self.getPortName())
        if not self.port: return 1

        bsl = os.path.join(configuration.c.getCfgValue("mansosDirectory"), 
                           "mos", "make", "scripts", "bsl.py")
        if self.platform == "telosb":
            platformArgs = ["--telosb"]
        elif self.platform == "xm1000":
            platformArgs = ["--tmote2618"]
        elif self.platform == "z1":
            platformArgs = ["--z1"]
        else:
            # assume "generic" MSP430 board
            platformArgs = ["--invert-reset", "--invert-test"]

        arglist = ["python", bsl, "-c", self.port.portstr, "-r", "-e", "-I", "-p", filename]
        arglist.extend(platformArgs)
        if configuration.c.getCfgValueAsBool("slowUpload"):
            arglist.append("--slow")

        os.environ['BSLPORT'] = self.moteDescription.getPort()
        retcode = runSubprocess1(arglist, server)

        return retcode

    def tryToCompileAndUpload(self, server, filename):
#        print("tryToCompileAndUpload for " + self.getPortName())
        if not self.port: return 1

        os.environ['BSLPORT'] = self.moteDescription.getPort()
        arglist = ["make", "-C", "build", self.platform, "upload"]
        retcode = runSubprocess1(arglist, server)

#        print ("compile and upload done!")

        return retcode


class MoteCollection(object):
    def __init__(self):
        self.motes = dict()

    @staticmethod
    def motesUpdated():
        motes.refreshMotes(Motelist.getMotelist(False))

    def addAll(self):
        cfgMotes = configuration.c.getCfgValueAsList("motes")
        for portName in cfgMotes:
            Motelist.addMote(portName, "A statically added mote", "")
        Motelist.initialize(self.motesUpdated, startPeriodicUpdate = True)
        self.refreshMotes(Motelist.getMotelist(False))
        self.retrieveSelected()

    def refreshMotes(self, newMotelist):
        toRemove = set()

        for m in self.motes.values():
            found = False
            for d in newMotelist:
                if d.getPort() == m.getPortName():
                    found = True
                    break
            if not found:
                toRemove.add(m)

        for m in toRemove:
            print("remove " + m.getPortName())
            m.ensureSerialIsClosed()
            del self.motes[m.getPortName()]

        for d in newMotelist:
            if d.getPort() not in self.motes:
                print("add " + d.getPort())
                self.motes[d.getPort()] = Mote(d)

    def storeSelected(self):
        selected = []
        platforms = []
        for m in self.motes.values():
            if m.isSelected:
                selected.append(m.getPortName())
            platforms.append(m.getPortName() + ":" + m.platform)
        configuration.c.setCfgValue("selectedMotes", selected)
        configuration.c.setCfgValue("motePlatforms", platforms)
        configuration.c.save()

    def retrieveSelected(self):
        selected = configuration.c.getCfgValueAsList("selectedMotes")
        platforms = configuration.c.getCfgValueAsList("motePlatforms")
        for m in self.motes.values():
            m.isSelected = m.getPortName() in selected
        for p in platforms:
            try:
                (portName, platform) = p.split(':')
                self.motes[portName].platform = platform
            except:
                pass

    def getMotes(self):
        return self.motes.values()

    def getMote(self, portname):
        return self.motes.get(portname, None)

    def isEmpty(self):
        return len(self.motes) == 0

    def anySelected(self):
        for m in self.motes.values():
            if m.isSelected: return True
        return False

motes = MoteCollection()
