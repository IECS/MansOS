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
import os
from multiprocessing import Pipe, Process
import time
import webbrowser

from blockly_handler import listen

class Blockly(wx.Panel):
    def __init__(self, parent, API):
        wx.Panel.__init__(self, parent)
        self.API = API
        self.tr = self.API.translater.translate
        self.active = False

        label = wx.StaticText(self, label = self.tr("When code recieved") + ":")
        self.choice = wx.ComboBox(self, style = wx.CB_DROPDOWN | wx.CB_READONLY,
                            choices = ['open', 'save', 'upload'],
                            name = "action")
        self.choice.SetValue("open") #TODO: remember
        self.start = wx.Button(self, label = self.tr("Start Seal-Blockly editor"))
        controls = wx.BoxSizer(wx.HORIZONTAL)
        controls.Add(label, 0, wx.EXPAND | wx.ALL)
        controls.Add(self.choice, 0, wx.EXPAND | wx.ALL)
        controls.Add(self.start, 0, wx.EXPAND | wx.LEFT | wx.RIGHT)

        self.outputArea = wx.TextCtrl(self, style = wx.TE_MULTILINE)
        self.outputArea.SetBackgroundColour("black")
        self.outputArea.SetForegroundColour("white")
        self.outputArea.SetEditable(False)

        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(controls, 0, wx.EXPAND | wx.wx.TOP | wx.LEFT | wx.RIGHT, 10)
        sizer.Add(self.outputArea, 1, wx.EXPAND | wx.ALL, 5)

        self.Bind(wx.EVT_BUTTON, self.toggleListen, self.start)
        self.SetBackgroundColour("white")
        self.SetSizerAndFit(sizer)
        self.SetAutoLayout(1)
        self.Hide()

    def toggleListen(self, event = None):
        self.active = not self.active
        if self.active:
            self.start.SetLabel(self.tr("Stop Seal-Blockly editor"))
            self.run()
        else:
            self.start.SetLabel(self.tr("Start Seal-Blockly editor"))

    def run(self):
        filename = os.path.join(self.API.path, self.API.getSetting('blocklyLocation'))
        host = self.API.getSetting('blocklyHost')
        port = int(self.API.getSetting('blocklyPort'))
        con1, con2 = Pipe()
        p1 = Process(target = listen, args = (host, port, con2, True))
        p1.daemon = False
        p1.name = "Socket listening thread"

        try:
            p1.start()
            self.printLine("Service started successfully.")
            if p1.is_alive():
                #self.printLine('Try to open "{}"'.format(filename))
                webbrowser.open(filename)
            else:
                self.printLine("Failed to open {}:{}, port might be in use.".format(host, port))
            lastSync = time.time() + 10
            self.API.onExit.append(p1.terminate)
            while p1.is_alive() and self.active:
                if con1.poll(0.1):
                    data = con1.recv()
                    if data != "Sync recieved":
                        self.handleRecievedCode(data)
                    lastSync = time.time()
                if time.time() - lastSync > 10:
                    self.printLine("No sync for 10 sec.\nTerminating...")
                    self.API.onExit.remove(p1.terminate)
                    p1.terminate()
                    break
                wx.YieldIfNeeded()
            if p1.is_alive():
                self.printLine("Service terminated successfully.")
                self.API.onExit.remove(p1.terminate)
                p1.terminate()
        except:
            self.printLine("Exception occurred, terminating...")
            self.API.onExit.remove(p1.terminate)

    def handleRecievedCode(self, code):
        self.printLine("\nGot code:\n" + code + "\n")
        if self.choice.GetValue() == 'open':
            self.onOpen(code)
        if self.choice.GetValue() == 'save':
            self.onSave(code)
        if self.choice.GetValue() == 'upload':
            self.onUpload(code)

    def onOpen(self, code):
        self.API.tabManager.addPage()
        self.API.tabManager.GetCurrentPage().changeCode(code)

    def onSave(self, code):
        self.onOpen(code)
        self.API.tabManager.doPopupSave(None)

    def onUpload(self, code):
        self.onSave(code)
        self.API.uploadCore.manageUpload()

    def printLine(self, text, clear = False):
        # TODO moves cursor to output area, away from editor if cursor was there!
        # self.API.outputTools.SetSelection(self.nr)
        if clear:
            self.clear()
        self.outputArea.AppendText(text.strip() + '\n')
        self.outputArea.ScrollLines(1)

    def clear(self):
        self.outputArea.Clear()
