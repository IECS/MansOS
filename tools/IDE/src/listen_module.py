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
from helperFunctions import listenSerialPort
from myThread import MyThread

class ListenModule(wx.Panel):
    def __init__(self, parent, API):
        super(ListenModule, self).__init__(parent)
            #title = title, size = (500, 400), style=wx.DEFAULT_DIALOG_STYLE | wx.RESIZE_BORDER)
        self.API = API
        # Just a shorter name
        self.tr = self.API.translater.translate
        self.haveMote = False
        self.listening = False
        self.args = {
             'serialPort': '/dev/ttyUSB0',
             'baudrate': 38400
         }

        self.API.outputArea.Reparent(self)
        self.updateStatus = self.API.printOutput
        self.API.motelistCallbacks.append(self.updateMotelist)

        self.SetBackgroundColour("white")
        self.main = wx.BoxSizer(wx.VERTICAL)
        self.listenControls = wx.BoxSizer(wx.HORIZONTAL)

        self.ports = wx.ComboBox(self, choices = [], size = (300, -1))
        self.clear = wx.Button(self, label = self.tr("Start listening"))
        self.refresh = wx.Button(self, label = self.tr("Refresh"))

        self.listenControls.Add(self.ports)
        self.listenControls.Add(self.refresh)
        self.listenControls.Add(self.clear)

        self.main.Add(self.listenControls, 0,
                      wx.EXPAND | wx.wx.TOP | wx.LEFT | wx.RIGHT, 10);
        self.main.Add(self.API.outputArea, 1, wx.EXPAND | wx.ALL, 5);

        self.Bind(wx.EVT_BUTTON, self.doClear, self.clear)
        self.Bind(wx.EVT_BUTTON, self.getMotelist, self.refresh)
        self.Bind(wx.EVT_COMBOBOX, self.changeTarget, self.ports)

        self.SetSizer(self.main)
        self.main.Fit(self)
        self.Show()

    def doClear(self, event = None):
        # String is returned when callback triggers, so by that we always know
        # that we do not listen now
        if type(event) is str:
            if self.listening:
                self.listening = False
                self.clear.SetLabel(self.tr('Start listening'))
                self.updateStatus("\nListening stopped.\n", False)
            return

        self.listening = not self.listening
        if self.listening:
            self.clear.SetLabel(self.tr('Stop listening'))
            self.updateStatus("\nListening started.\n", False)
        else:
            self.clear.SetLabel(self.tr('Start listening'))
            self.updateStatus("\nListening stopped.\n", False)
        # Redraw button if size have changed
        self.clear.SetSize(self.clear.GetEffectiveMinSize())
        if self.listening:
            thread = MyThread(listenSerialPort, self.args, \
                              self.doClear, False, True, "Serial port listener")
            self.API.startThread(thread)
        else:
            self.API.stopThread("Serial port listener")

    def getMotelist(self, event = None):
        self.API.supressTabSwitching = True
        self.updateStatus("Populating motelist ... ", False)
        self.ports.Clear()
        self.ports.Append(self.tr("Searching devices") + "...", 0)
        self.ports.Disable()
        self.API.populateMotelist()

    def updateMotelist(self):
        self.ports.Clear()
        motelist = self.API.motelist
        if not self.ports.IsEnabled():
            self.updateStatus("Done!\n")
        if len(motelist) > 0:
            for x in motelist:
                if len(x) > 2:
                    self.ports.Append(x[1] + " - " + x[2])
            self.haveMote = True
            self.ports.SetValue(self.tr("Use default device"))
            if len(motelist[0]) > 1:
                self.args['serialPort'] = motelist[0][1]
            self.ports.Enable()
        else:
            self.ports.SetValue(self.tr("No devices found"))
            self.haveMote = False

    def changeTarget(self, event):
        # Stop listening
        if self.listening:
            self.doClear(None)
        self.args['serialPort'] = event.GetEventObject().GetValue()
