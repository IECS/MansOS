#!/usr/bin/env python

#
# What does this program do:
# - send a command and arguments to TestBed device, based on command line parameters
# - listen to output afterwards, if specified
# 

import time

import serial
import sys
import threading
import argparse
import struct

global flDone
global ser
global baudRate

#CONTROL_MODULE        = 0
SIGNAL_MODULE         = 1
POWER_MODULE          = 2
MOTE_MODULE           = 3 # not a module, but the attached mote

START_CHARACTER       = '$'

#MAGIC_KEY             = 0xD19A
MAGIC_KEY           = 0x9A
KEY_LEN             = 8


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


def selectModule(ser, module):
#    ser.setRTS(0)
#    time.sleep(0.001)
#    ser.setRTS(1)
#    time.sleep(0.001)
#    ser.setRTS(0)
#    time.sleep(0.001)
#    ser.setRTS(1)
#    time.sleep(0.001)
#    ser.setRTS(0)
#    time.sleep(0.001)
#    ser.setRTS(1)
#    time.sleep(0.001)
#    ser.setRTS(0)
#    time.sleep(0.001)
#    ser.setRTS(1)
#    time.sleep(0.001)
#    ser.setDTR(1)
#    return

    module = int(module)
    print "selecting module", module
    # send magic key
    for i in range(KEY_LEN):
        # clock low
        ser.setRTS(0)
        time.sleep(0.001)
        # data
        ser.setDTR(not bool(MAGIC_KEY & (1 << (KEY_LEN - 1 - i))))
#        print "key bit ", bool(MAGIC_KEY & (1 << (KEY_LEN - 1 - i)))
        # clock high
        ser.setRTS(1)
        time.sleep(0.001)
    # send address
    for i in range(2):
        # clock low
        ser.setRTS(0)
        time.sleep(0.001)
        # data
        ser.setDTR(not bool(module & (1 << (1 - i))))
#        print "data bit ", bool(module & (1 << (1 - i)))
        # clock high
        ser.setRTS(1)
        time.sleep(0.001)
    print "done!"
    ser.setDTR(0)
    ser.setRTS(0)

def sendCommand(ser, command, args):
    ser.write(START_CHARACTER)
    ser.write(struct.pack('B', command))
    args = args.split(',')
    ser.write(struct.pack('B', len(args) * 4))
    for a in args:
        print "a=",a
        ser.write(struct.pack('>i', int(a, 0)))
    print "command sent!"

def listenSerial():
    global flDone
    global ser
    global baudRate

    print ("Listening to serial port: {}, rate: {}".format(ser.portstr, baudRate))
    
    while (not flDone):
        s = ser.read(1)
        if len(s) >= 1:
            if type(s) is str: sys.stdout.write( s )
            else: sys.stdout.write( "{}".format(chr(s[0])) )
            sys.stdout.flush()
            
    print ("\nDone")
    ser.close()
    return 0


def getCliArgs():
    defaultSerialPort = "/dev/ttyUSB0"
    defaultBaudRate = 38400
    version = "0.1/2012.10.18"

    parser = argparse.ArgumentParser(description="TestBed command sender listener", prog="testbed_cmd_sender")

    parser.add_argument('-m', '--module', dest='module', action='store', default=0,
        help='select specific module (0 - control, 1 - communicatin, 2 - power, 3 - mote')
    parser.add_argument('-c', '--command', dest='command', action='store', default=0,
        help='send command')
    parser.add_argument('-a', '--args', dest='args', action='store', default='',
        help='set arguments to command')
    parser.add_argument('-s', '--serial_port', dest='serialPort', action='store', default=defaultSerialPort,
        help='serial port to listen (default: ' + defaultSerialPort + ' )')
    parser.add_argument('-b', '--baud_rate', dest='baudRate', action='store', default=defaultBaudRate,
        help='baud rate (default: ' + str(defaultBaudRate) + ')')
    parser.add_argument('-l', '--listen', dest='doListen', action='store', default=False,
        help='listen to serial port output after command is sent')
    parser.add_argument('--version', action='version', version='%(prog)s ' + version)
    return parser.parse_args()


def main():
    global flDone
    global ser
    global baudRate

    flDone = False

    args = getCliArgs()

    serPort = args.serialPort
    if serPort in ("ACM", "chronos") :
        serPort = "/dev/ttyACM0" 
        baudRate = 115200

    baudRate = args.baudRate

    try:
        ser = serial.Serial(serPort, baudRate, timeout=1, 
                            parity=serial.PARITY_NONE, rtscts=1)
    except serial.SerialException as ex:
        print ("\nSerial exception {}:\n\t".format(ex))
        return 1

    selectModule(ser, args.module)

    if args.command:
        sendCommand(ser, args.command, args.args)

    if args.doListen:
        print ("Serial listener stating")
        threading.Thread(target=listenSerial).start() 
    
        #Keyboard scanning loop
        while (not flDone):
            try:
                s = input()
            except:
                print ("\nKeyboard interrupt")
                flDone = True
                return 0
             
            if s == 'q':
                flDone = True 
    else:
        ser.close()

    return 0

if __name__ == "__main__":
    main()
