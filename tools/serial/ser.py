#!/usr/bin/env python

#
# Serial port communicator (listener/sender) application
#

import time, serial, sys, threading, argparse

global flDone
global cliArgs
global writeBuffer

def getUserInput(prompt):
    if sys.version[0] >= '3':
        return input(prompt)
    else:
        return raw_input(prompt)

def listenSerial():
    global flDone
    global cliArgs
    global writeBuffer
    
    try:
        ser = serial.Serial(cliArgs.serialPort, cliArgs.baudRate, timeout=cliArgs.timeout, 
                            parity=serial.PARITY_NONE, rtscts=cliArgs.flowcontrol)
        ser.flushInput()
        ser.flushOutput()
        if cliArgs.platform not in ['xm1000', 'z1'] :
            # make sure reset pin is low for the platforms that need it
            ser.setDTR(0)
            ser.setRTS(0)

    except serial.SerialException as ex:
        sys.stderr.write("\nSerial exception:\n\t{}".format(ex))
        flDone = True
        return
    
    sys.stderr.write("Using port {}, baudrate {}\n".format(ser.portstr, cliArgs.baudRate))

    while (not flDone):
        # write
        if writeBuffer:
            for c in writeBuffer:
                ser.write(bytearray([c]))
            writeBuffer = ""
        # read
        serLen = ser.inWaiting()
        if serLen > 0:
            s = ser.read(serLen)

            if type(s) is str: 
                sys.stdout.write( s )
            else:
                for c in s:
                    sys.stdout.write( "{}".format(chr(c)) )
            sys.stdout.flush()
        # allow other threads to run
        time.sleep(0.001)

    sys.stderr.write("\nDone\n")
    ser.close()
    return 0


def getCliArgs():
    defaultSerialPort = "/dev/ttyUSB0"
    defaultBaudRate = 38400
    version = "0.5/2013.01.17"

    parser = argparse.ArgumentParser(description="MansOS serial communicator", prog="ser")

    parser.add_argument('-s', '--serial_port', dest='serialPort', action='store', default=defaultSerialPort,
        help='serial port to listen (default: ' + defaultSerialPort + ' )')
    parser.add_argument('-b', '--baud_rate', dest='baudRate', action='store', default=defaultBaudRate,
        help='baud rate (default: ' + str(defaultBaudRate) + ')')
    parser.add_argument('-p', '--platform', dest='platform', action='store', default='telosb',
        help='platform (default: telosb)')
    parser.add_argument('-f', '--flowcontrol', dest='flowcontrol', action='store', default=False,
        help='enable hardware flow control (default: False)')
    parser.add_argument('-t', '--timeout', dest='timeout', action='store', default=1,
        help='timeout for serial (default: 1)')
    parser.add_argument('--version', action='version', version='%(prog)s ' + version)
    return parser.parse_args()



def main():
    global flDone
    global cliArgs
    global writeBuffer

    flDone = False

    cliArgs = getCliArgs()

    if cliArgs.serialPort in ("ACM", "chronos"):
        cliArgs.serialPort = "/dev/ttyACM0" 
        cliArgs.baudRate = 115200

    sys.stderr.write("MansOS serial port access app, press Ctrl+C to exit\n")
    threading.Thread(target=listenSerial).start() 

    #Keyboard scanning loop
    writeBuffer = ""
    while (not flDone):
        try:
            s = getUserInput("")
        except BaseException as ex:
            sys.stderr.write("\nKeyboard interrupt\n")
            flDone = True
            return 0

        writeBuffer += s + '\r\n'
    
    return 0

if __name__ == "__main__":
    main()
