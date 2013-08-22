#!/usr/bin/env python

import threading
import time

from motes import motes
import moteconfig
import sensor_data
import configuration
import data_utils

isListening = False
listenThread = None

def listenSerial():
    global isListening
    
    while isListening:
        for m in motes.getMotes():

            length = m.tryRead(binaryToo = moteconfig.instance.configMode)
            if length == 0:
                continue

            if moteconfig.instance.configMode:
                for c in m.buffer:
                    moteconfig.instance.byteRead(c)
                m.buffer = ""
                continue

            while '\n' in m.buffer:
                pos = m.buffer.find('\n')
                if pos != 0:
                    newString = m.buffer[:pos].strip()
                    saveToDB = configuration.c.getCfgValue("saveToDB")
                    sendToOpenSense = configuration.c.getCfgValue("sendToOpenSense")
                    if saveToDB or sendToOpenSense:
                        data_utils.maybeAddDataToDatabase(m.port.port, newString)
                    # print "got", newString
                    sensor_data.moteData.addNewData(newString, m.port.portstr)
                m.buffer = m.buffer[pos + 1:]

        sensor_data.moteData.fixSizes()
        # pause for a bit
        time.sleep(0.01)
        
def openAllSerial():
    global isListening
    global listenThread
    
    if isListening: return
    isListening = True
    listenThread = threading.Thread(target = listenSerial)
    listenThread.start()
    for m in motes.getMotes():
        m.tryToOpenSerial(False)

def closeAllSerial():
    global isListening
    global listenThread
    
    isListening = False
    if listenThread:
        listenThread.join()
        listenThread = None
    for m in motes.getMotes():
        m.ensureSerialIsClosed()