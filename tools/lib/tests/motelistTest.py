#!/usr/bin/python

#
# Motelist test app
#

from sys import path
from time import sleep
import os

path.append("..")

from motelist import Motelist

def manualyScanForMotes(prefix):
    prefix = str(prefix)
    retVal = list()
    
    print "Manualy scaning for motes with prefix: {}".format(prefix)
    for portNumber in range(20):
        if Motelist.addMote(prefix + str(portNumber), "TestMote", "Test description"):
            print "\tFound mote on port:", prefix + str(portNumber)
            retVal.append(prefix + str(portNumber))
            
    print "Founded  mote count: {}".format(len(retVal))
    
    return retVal

# This is function which gets called during next second after motelist have changed
# Note, that this gets called from another thread, so thread safety precautions must
# be used if necessary, this test doesn't need it, because it only prints motelist.
def periodicUpdateCallback():
    motelist = Motelist.getMotelist(False)
    print "Periodic update result:", "Empty" if len(motelist) == 0 else ""
    for mote in motelist:
        print "\t{}".format(mote.getCSVData())
        
# Because python sucks and don't ask more
def readKey():
    try:
        x = input()
    except:
        pass

# Get right prefix for running OS
prefix = ""
if os.name == 'posix':
    prefix = "/dev/ttyUSB" # Not tested
elif os.name == "cygwin":
    prefix = "/dev/com" # Not tested
elif os.name == "nt":
    prefix = "COM" # Tested
elif os.name == 'darwin':
    prefix = "/dev/tty." # Not tested
else:
    print ("Your OS ('{}') is not supported, sorry!".format(os.name))
    exit(1)

# This is done before any other action on Motelist, 
# to ensure that motelist is empty on manual scan, because
# if there are already registred mote on that port, 
# motelist will ignore it.
ports = manualyScanForMotes(prefix)
readKey()

# This actually updates motelist with all connected motes, 
# but found motes are not overwriten, because they already exist in the list.
print "Printing motelist with manualy found motes\n"
Motelist.printMotelist()
readKey()

# Clear stored motelist, to automatically detect conected motes.
print "\nClearing motelist from manually found motes."
Motelist.motes = list()
readKey()

# Automatically detect and print all connected motes.
print "\nRunning automatic mote detection and printing motelist\n"
Motelist.printMotelist()
readKey()

# Try to manually add previously found motes, should fail!
print "\nTrying to add back previously manualy found motes(this should FAIL)"
for port in ports:
    print "\tAdding mote on port {}, result: {}".format(
		port, "Success" if Motelist.addMote(port, "TestMote", "Test description") else "Failure")
readKey()

print "\nStarting periodic updates, press enter to stop!"
print "\nTry to attach and reattach motes, to see any changes!"

# Add our function to callbacks.
# Start periodic update, which checks for motelist changes
# each second and calls all callbacks only if motelist have changed.
# Note that this is not blocking, it runs on different thread.
Motelist.initialize(periodicUpdateCallback, True)

readKey()
print "\nMotelist once again:"

# Simple check, to see if no problems are caused by this.
Motelist.getMotelist(False)
Motelist.getMotelist(True)
Motelist.printMotelist()

# Clean up.
Motelist.removeUpdateCallback(periodicUpdateCallback)
Motelist.stopPeriodicUpdate()

# Cya.
print "\nThe end, to understand more, please look into the source of this test."
readKey()