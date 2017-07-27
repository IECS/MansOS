#!/usr/bin/env python

#
# Serial port communicator (listener/sender) application
#

import time
import serial
import sys
import threading
import argparse
import glob
from sys import platform as _platform

global flDone
global cliArgs
global writeBuffer
global portsList


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
        if cliArgs.platform not in ['xm1000', 'z1']:
            # make sure reset pin is low for the platforms that need it
            ser.setDTR(0)
            ser.setRTS(0)

    except serial.SerialException as ex:
        sys.stderr.write("\nSerial exception:\n\t{}".format(ex))
        flDone = True
        return

    sys.stderr.write("Using port {}, baudrate {}\n".format(
        ser.portstr, cliArgs.baudRate))

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
                sys.stdout.write(s)
            else:
                for c in s:
                    sys.stdout.write("{}".format(chr(c)))
            sys.stdout.flush()
        # allow other threads to run
        time.sleep(0.001)

    sys.stderr.write("\nDone\n")
    ser.close()
    return 0


def serialPortsList():
    """ Lists serial port names
        :raises EnvironmentError:
            On unsupported or unknown platforms
        :returns:
            A list of the serial ports available on the system
    """
    if sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        # this excludes your current terminal "/dev/tty"
        ports = glob.glob('/dev/tty[UA]*')
    elif sys.platform.startswith('darwin'):
        ports = glob.glob('/dev/tty.usb*')
    elif sys.platform.startswith('win'):
        ports = ['COM%s' % (i + 1) for i in range(256)]
    else:
        raise EnvironmentError('Unsupported platform')

    result = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    result = sorted(result)
    return result


def getCliArgs():

    global portsList

    # defaultSerialPort = "/dev/ttyUSB0"
    defaultSerialPort = portsList[0]

    defaultBaudRate = 38400
    version = "0.6/2016.06.30"

    parser = argparse.ArgumentParser(
        description="MansOS serial communicator", prog="ser")

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
    parser.add_argument('--version', action='version',
                        version='%(prog)s ' + version)

    parser.add_argument('-l', '--list', action="store_true", default=False,
                        help='list available serial ports')

    return parser.parse_args()


def main():
    global flDone
    global cliArgs
    global writeBuffer
    global portsList

    flDone = False

    portsList = serialPortsList()
    if len(portsList) <= 0:
        sys.stderr.write("No serial ports found!\n")
        return 1

    cliArgs = getCliArgs()

    if cliArgs.serialPort in ("ACM", "chronos"):
        cliArgs.serialPort = "/dev/ttyACM0"
        cliArgs.baudRate = 115200

    sys.stderr.write("MansOS serial port access app, press Ctrl+C to exit\n")

    # Detect the platform. Serial ports are named differently for each
    if sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        sys.stderr.write("Detected Linux\n")
    elif sys.platform.startswith('darwin'):
        sys.stderr.write("Detected Darwin\n")
    else:
        sys.stderr.write("Assuming Windows\n")

    if cliArgs.list:
        sys.stderr.write("Available serial ports: ")
        sys.stderr.write(str(portsList))
        sys.stderr.write("\n")
        return 0

    threading.Thread(target=listenSerial).start()

    # Keyboard scanning loop
    writeBuffer = ""
    while (not flDone):
        try:
            s = getUserInput("")

            if s == "exit":
                print("Exit command received")
                flDone = True
                return 0
        except BaseException as ex:
            sys.stderr.write("\nKeyboard interrupt\n")
            flDone = True
            return 0

        writeBuffer += s + '\r\n'

    return 0


if __name__ == "__main__":
    main()
