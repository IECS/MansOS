#!/usr/bin/env python
#
# "Copyright (c) 2010  Leo Selavo and the contributors."
# Permission to use and disclaimer are defined in /USE_AND_DISCLAIMER.txt
#

#
# URLs:
#  mote?id=x - read data from mote #x
#  allmotes?start=x&end=y - read data from motes #x to #y
#
# To get plain (undecorated) text output append "raw=1" to query string
#

import os, urlparse, BaseHTTPServer, threading, time, serial, select, socket, cgi, subprocess, struct

BAUDRATE = 38400

# params: on/off, led index
CMD_LED_CONTROL  = 1
# params: channel, sampling period in ms
CMD_ADC_CONTROL  = 2
# params: channel, constant value
CMD_DAC_CONTROL  = 3
# params: enable/disable
CMD_SERIAL_CONTROL  = 4
# params: enable/disable
CMD_SD_CARD_CONTROL  = 5

START_CHARACTER = '$'

defaultUSBPort = "/dev/ttyUSB0"

listenThread = None

def sendCommand(ser, command,  arg1 = None, arg2 = None):
    print "before: ", ser.read()

    ser.write(START_CHARACTER)
    ser.write(struct.pack('B', command))
    if command == CMD_ADC_CONTROL:
        args = struct.pack('<BBI', 5, arg1, arg2)
    elif command == CMD_SERIAL_CONTROL \
            or command == CMD_SD_CARD_CONTROL:
        args = struct.pack('BB', 1, arg1)
    elif command == CMD_LED_CONTROL:
        args = struct.pack('BBB', 2, arg1, arg2)
    else:
        args = None

    crc = ord(START_CHARACTER) ^ command
    if args:
        for a in args:
            crc ^= ord(a)
        ser.write(args)
    ser.write(struct.pack('B', crc))
    print "after: ", ser.read()

def main():
    ser = serial.Serial("/dev/ttyUSB0", BAUDRATE, timeout=1,
                        parity=serial.PARITY_NONE, rtscts=0)
    ser.setDTR(0)
    ser.setRTS(0)

#    if ser.inWaiting():
#        print "data: ", ser.read(ser.inWaiting())
    print "data: ", ser.read()

#    ser.write('$')
#    ser.write('\x01') # command
#    ser.write('\x02') # arg len
#    ser.write('\x01') # arg1
#    ser.write('\x01') # arg2
#    ser.write('\x27') # crc
#    time.sleep(1)
#    print "data: ", ser.read()
#    if ser.inWaiting():
#        print "data: ", ser.read(ser.inWaiting())
#    return

#    if ser.inWaiting():
#        print "data: ", ser.read(ser.inWaiting())
    sendCommand(ser, CMD_LED_CONTROL, 0, 1)
#    if ser.inWaiting():
#        print "got ", ser.read(ser.inWaiting())
    time.sleep(1)
    sendCommand(ser, CMD_LED_CONTROL, 1, 1)
#    if ser.inWaiting():
#        print "got ", ser.read(ser.inWaiting())
    time.sleep(1)
    sendCommand(ser, CMD_LED_CONTROL, 2, 1)
#   c = ser.read()
#   print "got", c
    time.sleep(1)
    sendCommand(ser, CMD_LED_CONTROL, 3, 1)
#   c = ser.read()
#   print "got", c
    time.sleep(1)
    print "done!"

if __name__ == '__main__':
    main()
