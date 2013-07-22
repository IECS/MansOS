#
# MansOS web server - mote handler class
#

import serial, subprocess, sys, time, os, threading
from motelist import Motelist
import configuration

def runSubprocess1(args, server):
    retcode = -1
    try:
        proc = subprocess.Popen(args, stderr = subprocess.STDOUT, stdout = subprocess.PIPE,
                                shell = False)
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
#    counter = 0
    def __init__(self, moteDescription):
#        self.number = Mote.counter
#        Mote.counter += 1
        self.moteDescription = moteDescription
        self.port = None
        self.isSelected = False
        self.isStatic = False
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

    def openSerial(self):
        if not self.isSelected: return
        try:
            baudrate = configuration.c.getCfgValueAsInt("baudrate", SERIAL_BAUDRATE)
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
        self.portLock.acquire()
        if not self.port:
            if makeSelected: self.isSelected = True
            self.openSerial()
        result = self.port is not None
        self.portLock.release()
        return result

    def closeSerial(self):
        self.portLock.acquire()
        try:
            if self.port:
                self.port.close()
                print("Serial port " + self.moteDescription.getPort() + " closed")
                self.port = None
        except:
            raise
        finally:
            self.port = None
            self.portLock.release()

    def tryRead(self, binaryToo):
        numRead = 0
        if self.port:
            self.bufferLock.acquire()
            try:
              while self.port.inWaiting():
                c = self.port.read(1)

                # save to file if required (raw data)
                if configuration.c.getCfgValue("saveToFilename") \
                        and not configuration.c.getCfgValue("saveProcessedData"):
                    filename = os.path.join(configuration.c.getCfgValue("dataDirectory"),
                                            self.moteDescription.getPort(),
                                            configuration.c.getCfgValue("saveToFilename"))
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
            finally:
                self.bufferLock.release()

        return numRead

    def tryToUpload(self, server, filename):
        if not self.tryToOpenSerial(False): return 1

        bsl = "mos/make/scripts/bsl.py"
        if self.platform == "telosb":
            platformArgs = ["--telosb"]
        elif self.platform == "xm1000":
            platformArgs = ["--tmote2618"]
        elif self.platform == "z1":
            platformArgs = ["--z1"]
        else:
            # assume "generic" MSP430 board
            platformArgs = ["--invert-reset", "--invert-test"]

        bsl = os.path.join(configuration.c.getCfgValue("mansosDirectory"), bsl)
        arglist = ["python", bsl, "-c", self.port.portstr, "-r", "-e", "-I", "-p", filename]
        argist.extend(platformArgs)
        if configuration.c.getCfgValueAsInt("slowUpload"):
            arglist.append("--slow")

        os.environ['BSLPORT'] = self.moteDescription.getPort()
        retcode = runSubprocess1(arglist, server)

        return retcode

    def tryToCompileAndUpload(self, server, filename):
        if not self.tryToOpenSerial(False): return 1

        os.environ['BSLPORT'] = self.moteDescription.getPort()
        arglist = ["make", self.platform, "upload"]
        retcode = runSubprocess1(arglist, server)

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
        Motelist.initialize(self.motesUpdated)
        Motelist.startPeriodicUpdate()
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
                self.motes[m.getPortName()].platform = platform
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
