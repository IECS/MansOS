#!/usr/bin/env python

import time

import serial
import sys
import threading
import argparse
import struct

global flDone
global ser
global baudRate


def main():
    global flDone
    global ser
    global baudRate

    serPort = "/dev/ttyUSB0"
    baudRate = 38400

    try:
        ser = serial.Serial(serPort, baudRate, timeout=1, 
                            parity=serial.PARITY_NONE, rtscts=1)
    except serial.SerialException as ex:
        print ("\nSerial exception {}:\n\t".format(ex))
        return 1
 
    while 1:
        ser.setDTR(0)
#        ser.setRTS(0)
        time.sleep(2)
        print "aaa"
        ser.setDTR(1)
#        ser.setRTS(1)
        time.sleep(2)
    ser.close()

    return 0

if __name__ == "__main__":
    main()
