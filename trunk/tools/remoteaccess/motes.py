#
# Remote access proxy server - mote handler class
#

import serial, subprocess, sys, time, os, threading
from motelist import Motelist
import configuration

MAX_DATA = 32 * 1024  # 32kb

class Mote(object):
    def __init__(self, moteDescription):
        self.moteDescription = moteDescription
        self.port = None
        self.buffer = ""
        self.platform = "telosb"
        self.bufferLock = threading.Lock()
        self.portLock = threading.Lock()

    def getPortName(self):
        return self.moteDescription.getPort()

    def openSerial(self):
        baudrate = configuration.c.getCfgValueAsInt("baudrate")
        stopbitsCfg = configuration.c.getCfgValueAsInt("stopbits")
        if stopbitsCfg == 2:
            stopbits = serial.STOPBITS_TWO
        else:
            stopbits = serial.STOPBITS_ONE
        parityCfg = configuration.c.getCfgValue("parity")
        if parityCfg == "odd":
            parity = serial.PARITY_ODD
        elif parityCfg == "even":
            parity = serial.PARITY_EVEN
        else:
            parity = serial.PARITY_NONE

        try:
            self.port = serial.Serial(self.moteDescription.getPort(),
                                      baudrate,
                                      timeout = 1,
                                      parity = parity,
                                      stopbits = stopbits)
            print("Serial port " + self.moteDescription.getPort() + " opened")
            self.port.flushInput()
            self.port.flushOutput()
#            if self.platform not in ['xm1000', 'z1'] :
#                # make sure reset pin is low for the platforms that need it
#                self.port.setDTR(0)
#                self.port.setRTS(0)
        except Exception as e:
            print("\nSerial exception:\n\t" + str(e))
            self.port = None
            return

        print("Listening to serial port: " + self.port.portstr + ", rate: " + str(baudrate))

    def tryRead(self):
        numRead = 0
        if self.port:
            self.bufferLock.acquire()
            try:
                self.buffer += self.port.read(self.port.inWaiting())
                if len(self.buffer) > MAX_DATA:
                    # resize the buffer to MAX_DATA
                    self.buffer = self.buffer[-MAX_DATA:]
            except Exception as e:
                print("\nserial read exception:\t" + str(e))
                self.port.close()
                self.port = None
            finally:
                self.bufferLock.release()

        return numRead

    def getData(self, doClean, maxData):
        with self.bufferLock:
            if maxData:
                data = self.buffer[-maxData:]
            else:
                data = self.buffer
            if doClean:
                self.buffer = ""
        return data

    def writeData(self, data):
        self.portLock.acquire()
        try:
            if self.port:
                result = self.port.write(data)
            else:
                result = -1
        except:
            raise
        finally:
            self.portLock.release()
        return result

    def ensureSerialIsOpen(self):
        with self.portLock:
            if self.port is None:
                self.openSerial()
            result = self.port is not None
        return result

    def ensureSerialIsClosed(self):
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

# -----------------------------------
class MoteCollection(object):
    def __init__(self):
        self.motes = dict()

    def addAll(self):
        cfgMotes = configuration.c.getCfgValueAsList("motes")
        for portName in cfgMotes:
            Motelist.addMote(portName, "A statically added mote", "")
        Motelist.initialize(self.motesUpdated, False, False)
        Motelist.startPeriodicUpdate()
        self.refreshMotes(Motelist.getMotelist(False))

    @staticmethod
    def motesUpdated():
        motes.refreshMotes(Motelist.getMotelist(False))

    def refreshMotes(self, newMotelist):
#        print("refresh motes")

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

    def getMotes(self):
        return self.motes.values()

    def getMote(self, portname):
        return self.motes.get(portname, None)


motes = MoteCollection()
