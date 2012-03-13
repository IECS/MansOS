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

import wx
import subprocess
import serial
import time

class ListenModule(wx.Panel):
    def __init__(self, parent, API):
        super(ListenModule, self).__init__(parent)
            #title = title, size = (500, 400), style=wx.DEFAULT_DIALOG_STYLE | wx.RESIZE_BORDER)
        self.API = API
        # Just a shorter name
        self.tr = self.API.translater.translate
        self.haveMote = False
        self.listening = False
        self.baudrate = 38400
        self.serialPort = '/dev/ttyUSB0'
        
        self.API.outputArea.Reparent(self)
        self.updateStatus = self.API.printOutput
        
        self.SetBackgroundColour("white")
        self.main = wx.BoxSizer(wx.VERTICAL)
        self.listenControls = wx.BoxSizer(wx.HORIZONTAL)
        
        self.ports = wx.ComboBox(self, choices = [], size = (300, -1))
        self.clear = wx.Button(self, label = self.tr("Start listening"))
        self.refresh = wx.Button(self, label = self.tr("Refresh"))
        
        self.listenControls.Add(self.ports)
        self.listenControls.Add(self.refresh)
        self.listenControls.Add(self.clear)
        
        self.main.Add(self.listenControls, 0, wx.EXPAND | wx.ALL, 5);
        self.main.Add(self.API.outputArea, 1, wx.EXPAND | wx.ALL, 5);
        
        self.getMotelist()
        
        self.Bind(wx.EVT_BUTTON, self.doClear, self.clear)
        self.Bind(wx.EVT_BUTTON, self.getMotelist, self.refresh)
        self.Bind(wx.EVT_COMBOBOX, self.changeTarget, self.ports)

        self.SetSizer(self.main)
        self.main.Fit(self)
        self.Show()

    def doClear(self, event = None):
        self.listening = not self.listening
        if self.listening:
            self.clear.SetLabel(self.tr('Stop listening'))
        else:
            self.clear.SetLabel(self.tr('Start listening'))
        # Redraw button if size have changed
        self.clear.SetSize(self.clear.GetEffectiveMinSize())
        
        if self.listening:
            if self.listenSerialPort() == False:
                self.updateStatus(self.tr("Error conecting to device") + '\n', 
                                  False)
                self.doClear(None)

    # TODO: new thread!!!
    def listenSerialPort(self):
        try:
            ser = serial.Serial(self.serialPort, self.baudrate, timeout = 0, 
                               parity = serial.PARITY_NONE, rtscts = 1)
            while self.listening:
                s = ser.read(1000)
                if len(s) > 0:
                    self.updateStatus(s, False)
                # Keep interface alive, long sleep makes it to delay responses.
                # Listen once every .2 sec
                for _ in range(0, 200):
                    wx.Yield()
                    time.sleep(0.001)
            ser.close()
            return True
        except serial.SerialException, msg:
            print "\nSerial exception:\n\t", msg
            return False


    def getMotelist(self, event = None):
        motelist = []
        # get motelist output as string... |-(
        process = subprocess.Popen([self.API.path + "/../../mos/make/scripts/motelist"], 
                                      stderr = subprocess.STDOUT,
                                      stdout = subprocess.PIPE)
        motes = process.communicate()[0]
        
        # remove table header and split into lines, ("-----") + 6 used to be 
        # sure it's end of header, not random symbol in data
        self.ports.Clear()
        if motes.find("No devices found") == False:
            self.haveMote = False
            self.ports.SetValue(self.tr("No devices found"))
            return None
        
        for x in motes[motes.rfind("-----") + 6:].split("\n"):
            # seperate ID, port, description
            y = x.split(None, 2)
            if y != []:
                motelist.append(y)
        for x in motelist:
            if len(x) > 2:
                self.ports.Append(x[1] + " - " + x[2])
        self.haveMote = True
        self.ports.SetValue(self.tr("Use default device"))
        self.serialPort = motelist[0][1]
        return motelist
    
    def changeTarget(self, event):
        global serialPort
        target = event.GetEventObject().GetValue()
        serialPort = target[:target.find(" ")]
        self.serialPortChanged = True
