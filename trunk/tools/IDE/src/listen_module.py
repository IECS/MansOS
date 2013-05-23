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

from output_area import OutputArea
from helperFunctions import listenSerialPort
from myThread import MyThread
from motelist import Motelist
from Translater import localize

class ListenModule(wx.Panel):
    def __init__(self, parent, API):
        super(ListenModule, self).__init__(parent)

        self.API = API

        self.haveMote = False
        self.listening = False
        self.args = {
             'serialPort': '/dev/ttyUSB0',
             'baudrate': 38400
         }

        Motelist.addUpdateCallback(self.updateMotelist)

        self.SetBackgroundColour("white")
        self.main = wx.BoxSizer(wx.VERTICAL)
        self.listenControls = wx.BoxSizer(wx.HORIZONTAL)

        self.ports = wx.ComboBox(self, choices = [], size = (300, -1))
        self.clear = wx.Button(self, label = localize("Start listening"))
        self.refresh = wx.Button(self, label = localize("Refresh"))
        self.clearOutput = wx.Button(self, label = localize("Clear"))
        self.saveOutput = wx.Button(self, label = localize("Save to file"))
        self.baudrate = wx.ComboBox(self, choices = ['2400', '4800', '9600', '19200', '38400', '57600', '115200'])
        self.baudrate.SetSelection(4);

        self.ports.SetEditable(False)

        # Init outputArea for output
        self.outputArea = OutputArea(self, self.API, 1)
        self.updateStatus = self.outputArea.printLine
        self.clearOutputArea = self.outputArea.clear

        self.listenControls.Add(self.ports)
        self.listenControls.Add(self.baudrate)
        self.listenControls.Add(self.refresh)     
        self.listenControls.Add(self.clear)
        self.listenControls.Add(self.clearOutput)
        self.listenControls.Add(self.saveOutput)

        self.main.Add(self.listenControls, 0,
                      wx.EXPAND | wx.wx.TOP | wx.LEFT | wx.RIGHT, 10);
        self.main.Add(self.outputArea, 1, wx.EXPAND | wx.ALL, 5);

        self.Bind(wx.EVT_BUTTON, self.doClear, self.clear)         
        self.Bind(wx.EVT_BUTTON, self.updateMotelist, self.refresh)
        self.Bind(wx.EVT_BUTTON, self.clearOutputArea, self.clearOutput)
        self.Bind(wx.EVT_BUTTON, self.saveOutputArea, self.saveOutput)
        self.Bind(wx.EVT_COMBOBOX, self.changeTarget, self.ports)
        self.Bind(wx.EVT_COMBOBOX, self.changeBaudrate, self.baudrate)
        self.Bind(wx.EVT_TEXT, self.changeTarget, self.ports)

        #self.updateMotelist()

        self.SetSizer(self.main)
        self.main.Fit(self)
        self.Show()

    def doClear(self, event = None):
        self.API.stopThread("Serial port listener")

        # String is returned when callback triggers, so by that we always know
        # that we do not listen now
        if type(event) is str:
            if self.listening:
                self.listening = False
                self.clear.SetLabel(localize('Start listening'))
                self.updateStatus("\nListening stopped.\n", False)
            return

        if self.ports.GetValue() == "No devices found":
            return;

        self.listening = not self.listening

        if self.listening:
            self.clear.SetLabel(localize('Stop listening'))
            self.updateStatus("\nListening started.\n", False)
        else:
            self.clear.SetLabel(localize('Start listening'))
            self.updateStatus("\nListening stopped.\n", False)

        # Redraw button if size have changed
        self.clear.SetSize(self.clear.GetEffectiveMinSize())

        if self.API.getActivePlatform() not in ['xm1000', 'z1'] :
            # make sure reset pin is low for the platforms that need it
            self.args['reset'] = True
        else:
            self.args['reset'] = False

        if self.listening:
            thread = MyThread(listenSerialPort, self.args, \
                              self.doClear, False, True, \
                              "Serial port listener", self.updateStatus)
            self.API.startThread(thread)
        else:
            self.API.stopThread("Serial port listener")

    def updateMotelist(self, event = None):
        oldVal = self.ports.GetValue()
        self.haveMote = False

        self.ports.Clear()

        for mote in Motelist.getMotelist(False):
            if not self.haveMote:
                self.args['serialPort'] = mote.getPort()
                self.haveMote = True

            self.ports.Append(mote.getNiceName())

        self.ports.SetStringSelection(oldVal)

        if self.ports.GetValue() == "":
            if self.haveMote:
                self.ports.SetValue(localize("Use default device"))
            else:
                self.ports.SetValue(localize("No devices found"))

    def changeTarget(self, event):
        # Stop listening
        if self.listening:
            self.doClear(None)

        val = event.GetEventObject().GetValue()

        if val.count("(") != 0:
            self.args['serialPort'] = val.split("(")[1].split(")")[0]

    def changeBaudrate(self, event):
        # Stop listening
        if self.listening:
            self.doClear(None)

        try:
            self.args['baudrate'] = event.GetEventObject().GetValue()
        except:
            self.baudrate.setValue(self.args['baudrate']);

    def saveOutputArea(self, event):
        save = wx.FileDialog(self,
            localize("Save node output") + " \"",
            wildcard = localize('All files') + '|*',
            style = wx.FD_SAVE | wx.FD_OVERWRITE_PROMPT,
            defaultFile = self.args["serialPort"])
        if save.ShowModal() == wx.ID_OK:
            with open(save.GetPath(),"w") as out:
                out.write(self.outputArea.outputArea.GetValue())
        save.Destroy()
