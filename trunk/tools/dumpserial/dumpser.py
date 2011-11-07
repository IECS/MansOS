#!/usr/bin/env python

# Dump serial output to console

import operator
import time
import string

import serial
import sys
import threading

global flDone
global baudRate
global serPort

def listenSerial():
    global flDone
    global serPort
    global baudRate
    
    try:
        ser = serial.Serial(serPort, baudRate, timeout=1, 
            parity=serial.PARITY_NONE, rtscts=1)
    except serial.SerialException, ( msg ):
        print "\nSerial exception:\n\t", msg
        flDone = True
        return
    
    print "Listening to serial port: ", ser.portstr, ", rate: ", baudRate
    
    while (not flDone):
        s = ser.read(1)
        sys.stdout.write( s )
        sys.stdout.flush()
            
    print "\nDone"
    ser.close()
    return 0


def main():
    global flDone
    global serPort
    global baudRate

    flDone = False
    serPort = '/dev/ttyUSB0'
    baudRate = 38400

    for arg in sys.argv : 
        print arg

    if len(sys.argv) > 1 :
        serPort = sys.argv[1]

    if serPort in ("ACM", "chronos") :
        serPort = "/dev/ttyACM0" 
        baudRate = 115200

    if len(sys.argv) > 2 :
        baudRate = sys.argv[2]

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
