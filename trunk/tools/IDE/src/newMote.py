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

from motelist import Motelist
from Translater import localize

class NewMote(wx.Dialog):
    def __init__(self, parent, API):
        self.API = API

        super(NewMote, self).__init__(parent = parent, title = localize("Add new mote"))


        self.main = wx.BoxSizer(wx.VERTICAL)
        self.controls = wx.GridBagSizer(10, 10)
        self.newMote = wx.Button(self, label = localize("Add mote"))

        self.close = wx.Button(self, label = localize("Close"))

        self.controls.Add(self.close, (2, 0), flag = wx.EXPAND | wx.ALL)
        self.controls.Add(self.newMote, (2, 1), flag = wx.EXPAND | wx.ALL)

        nameText = wx.StaticText(self, label = localize("Mote name") + ":")
        self.controls.Add(nameText, (0, 0), flag = wx.EXPAND | wx.ALL)
        self.name = wx.TextCtrl(self)
        self.controls.Add(self.name, (0, 1), flag = wx.EXPAND | wx.ALL)

        portText = wx.StaticText(self, label = localize("Mote port") + ":")
        self.controls.Add(portText, (1, 0), flag = wx.EXPAND | wx.ALL)
        self.port = wx.TextCtrl(self)
        self.controls.Add(self.port, (1, 1), flag = wx.EXPAND | wx.ALL)
        self.portError = wx.StaticText(self, label = "")
        self.controls.Add(self.portError, (1, 2), flag = wx.EXPAND | wx.ALL)

        self.main.Add(self.controls, 0, wx.EXPAND | wx.ALL, 3);


        self.Bind(wx.EVT_BUTTON, self.addNewMote, self.newMote)
        self.Bind(wx.EVT_BUTTON, self.doClose, self.close)

        self.SetSizerAndFit(self.main)
        self.SetAutoLayout(1)
        self.Show()

    def doClose(self, event):
        self.Close()

    def addNewMote(self, event):
        if not Motelist.portExists(self.port.GetValue()):
            self.portError.SetLabel(localize("No device found on this port") + "!")

            self.SetSizerAndFit(self.main)
            self.SetAutoLayout(1)

            self.Show()
        else:
            if Motelist.addMote(self.port.GetValue(), self.name.GetValue(), "User defined"):
                self.API.updateUserMotes()
                self.Close()
            else:
                self.portError.SetLabel(localize("There already is device on that port in list") + "!")

                self.SetSizerAndFit(self.main)
                self.SetAutoLayout(1)

                self.Show()
