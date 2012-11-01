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

def getUserInput(prompt):
    if sys.version[0] >= '3':
        return input(prompt)
    else:
        return raw_input(prompt)

def listenSerial():
    global flDone
    global serPort
    global baudRate
    
    try:
        ser = serial.Serial(serPort, baudRate, timeout=1, 
            parity=serial.PARITY_NONE, rtscts=1)
        # make sure reset pin is low
        ser.setDTR(0)
        ser.setRTS(0)
    except serial.SerialException as ex:
        sys.stderr.write("\nSerial exception:\n\t{}".format(ex))
        flDone = True
        return
    
    sys.stderr.write("Using port {}, baudrate {}\n".format(ser.portstr, baudRate))

    while (not flDone):
        s = ser.read(1)
        if len(s) >= 1:
            if type(s) is str: sys.stdout.write( s )
            else: sys.stdout.write( "{}".format(chr(s[0])) )
            sys.stdout.flush()

    sys.stderr.write("\nDone\n")
    ser.close()
    return 0


def getCliArgs():
    defaultSerialPort = "/dev/ttyUSB0"
    defaultBaudRate = 38400
    version = "0.3/2012.10.26"

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

    sys.stderr.write("MansOS serial listener, press Ctrl+C or Q+Enter to exit\n")
    threading.Thread(target=listenSerial).start() 
    
    #Keyboard scanning loop
    while (not flDone):
        try:
            s = getUserInput("")
        except BaseException as ex:
            sys.stderr.write("\nKeyboard interrupt\n")
            flDone = True
            return 0
             
        if s.strip().lower() == 'q' :
            flDone = True 
    
    return 0

if __name__ == "__main__":
    main()
