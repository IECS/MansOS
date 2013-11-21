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
selectedMote = None

# Process mote data if available
def processMote(m):
    length = m.tryRead(binaryToo = moteconfig.instance.configMode)
    if length == 0:
        return

    if moteconfig.instance.configMode:
        for c in m.buffer:
            moteconfig.instance.byteRead(c)
        m.buffer = ""
        return

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

# Listen to all selected motes
def listenSerial():
    global isListening
    
    while isListening:
        for m in motes.getMotes():
            processMote(m)
        sensor_data.moteData.fixSizes()
        # pause for a bit
        time.sleep(0.01)
   
# Listen to single selected mote    
def listenSerialSingle():
    global isListening
    global selectedMote
    
    while isListening:
        processMote(selectedMote)
        sensor_data.moteData.fixSizes()
        # pause for a bit
        time.sleep(0.01)        

# Open all serial ports to listen for data
def openAllSerial():
    global isListening
    global listenThread
    
    if isListening: return
    isListening = True
    listenThread = threading.Thread(target = listenSerial)
    listenThread.start()
    for m in motes.getMotes():
        m.tryToOpenSerial(False)

# Open serial port for the selected mote to listen for data
def openMoteSerial(mote):
    global isListening
    global isListeningSingle
    global listenThread
    global selectedMote
    
    if isListening: return
    isListening = True
    if mote.port != None:
        selectedMote = mote
        mote.tryToOpenSerial(False)
    listenThread = threading.Thread(target = listenSerialSingle)
    listenThread.start()

# Close all serial ports
def closeAllSerial():
    global isListening
    global listenThread
    
    isListening = False
    if listenThread:
        listenThread.join()
        listenThread = None
    for m in motes.getMotes():
        m.ensureSerialIsClosed()