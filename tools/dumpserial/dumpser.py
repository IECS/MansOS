#!/usr/bin/env python

# Dump serial output to console

import operator
import time
import string

import serial
import sys
import threading
import argparse

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


def getCliArgs():
    defaultSerialPort = "/dev/ttyUSB0"
    defaultBaudRate = 38400
    version = "0.2/2012.04.25"

    parser = argparse.ArgumentParser(description="MansOS serial listener", prog="dumpser")

    parser.add_argument('-s', '--serial_port', dest='serialPort', action='store', default=defaultSerialPort,
        help='serial port to listen (default: ' + defaultSerialPort + ' )')
    parser.add_argument('-b', '--baud_rate', dest='baudRate', action='store', default=defaultBaudRate,
        help='baud rate (default: ' + str(defaultBaudRate) + ')')
    parser.add_argument('--version', action='version', version='%(prog)s ' + version)
    return parser.parse_args()



def main():
    global flDone
    global serPort
    global baudRate
    flDone = False

    args = getCliArgs()

    serPort = args.serialPort

    if serPort in ("ACM", "chronos") :
        serPort = "/dev/ttyACM0" 
        baudRate = 115200

    baudRate = args.baudRate

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
