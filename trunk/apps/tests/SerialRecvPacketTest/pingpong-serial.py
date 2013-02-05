#!/usr/bin/python
# use serial python lib included in MansOS
import sys
sys.path.append('../../../mos/make/scripts')

import serial
import threading
import random
import time

baudRate = 38400

try:

    ser = serial.Serial(
        port='/dev/ttyUSB0',
        baudrate=baudRate,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS
    )
except serial.serialutil.SerialException, ( msg ):
    print "\nSerial exception:\n\t", msg
    flDone = True



def main():
    global flDone, ser
    flDone = False
    print "MansOS serial Ping-Pong"
    print "It sends random binary values to the mote, it sends back"
    print "messages of form \"Pong <x>\", where <x> is the value sent"
    print "Mote also sends \"Ping <y>\" periodically, where <y> is it's local counter"

    threading.Thread(target=listenSerial).start() 
    threading.Thread(target=sendSerial).start() 
    
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


def listenSerial():
    global flDone, ser
    
    print "Listening to serial port: ", ser.portstr, ", rate: ", baudRate
    
    while (not flDone):
        s = ser.read(1)
        sys.stdout.write( s )
        sys.stdout.flush()
            
    print "\nDone"
    ser.close()
    return 0


def sendSerial():
    global ser

    random.seed();    

    while (not flDone):
        b = random.randint(1, 9)
        # send 5 bytes containing the same digit and the newline
        s = str(b) + str(b) + str(b) + str(b) + str(b) + "\n"
        print "-- sending " + s
        ser.write(s);
        time.sleep(random.randint(1,3))

    return 0

if __name__ == "__main__":
    main()

