#!/usr/bin/env python

# draw waveforms

import operator
import time
import string

import serial
import sys
import threading

global flDone

def listenSerial():
    global flDone
    
    try:
        ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=1, 
            parity=serial.PARITY_NONE, rtscts=1)
    except serial.SerialException, ( msg ):
        print "\nSerial exception:\n\t", msg
        flDone = True
        return
    
    print "Listening to serial port ", ser.portstr
    
    while (not flDone):
        s = ser.read(16)   #read up to one hundred bytes
        sys.stdout.write( s )
            
    print "\nDone"
    ser.close()
    return 0


def main():
    global flDone
    flDone = False
    print "MansOS serial listener"
    threading.Thread(target=listenSerial).start() 
    
    #Keyboard scanning loop
    while (not flDone):
        try:
            s = raw_input()
        except:
            print "\nKeyboard interrupt"
            flDone = True
            return 0
             
        if s == 'q' :
            flDone = True 
    
    return 0

if __name__ == "__main__":
    main()
