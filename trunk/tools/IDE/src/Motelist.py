# -*- coding: utf-8 -*-
#
# Copyright (c) 2008-2012 the MansOS team. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#  * Redistributions of source code must retain the above copyright notice,
#    this list of  conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

from platform import system
from serial.tools import list_ports

class Mote(object):
    def __init__(self, mote):
        if mote == None:
            self.port = None
            self.name = "No motes found!"
            self.description = "Make sure mote(s) are connected and drivers are installed."
        elif len(mote) == 3:
            self.port = mote[0]
            self.name = mote[1]
            self.description = mote[2]
        else:
            print ("Failed to initialize mote from " + str(mote))

    def getNiceName(self):
        if system() == 'Windows':
            return self.name
        else:
            return "{} ({})".format(self.name, self.port)

    # Makes equal work on different mote classes
    def __eq__(self, other):
        if type(other) is type(self):
            return self.__dict__ == other.__dict__
        return False

class Motelist(object):
    # Holds current motelist as list of Mote instances
    motes = list()
    # Holds active Mote instance from Motelist, defaults to first Mote
    activeMote = None
    # Hols functions to call on motelist change
    updateCallbacks = list()

    def __init__(self):
        Motelist.update()

    @staticmethod
    def update():
        Motelist.motes = list()
        gotActiveMote = False
        for x in list_ports.comports():
            if x[2] != "n/a":
                mote = Mote(x)
                Motelist.motes.append(mote)
                # Make pointer from old active mote's instance to new one.
                # Funny? but it works! :P
                if mote == Motelist.activeMote:
                    Motelist.activeMote = mote
                    gotActiveMote = True

        if len(Motelist.motes) == 0:
            Motelist.motes.append(Mote(None))

        if not gotActiveMote:
            Motelist.activeMote = Motelist.motes[0]
        for x in Motelist.updateCallbacks:
            x()

    @staticmethod
    def setActiveMote(activeMote):
        if activeMote in Motelist.motes:
            Motelist.activeMote = activeMote
        else:
            print ("Can't set " + str(activeMote) + " as active mote, it's not in motelist!")

    @staticmethod
    def getActiveMote():
        return Motelist.activeMote

    @staticmethod
    def addUpdateCallback(callback):
        Motelist.updateCallbacks.append(callback)

    @staticmethod
    def removeUpdateCallback(callback):
        try:
            Motelist.updateCallbacks.remove(callback)
        except:
            pass
