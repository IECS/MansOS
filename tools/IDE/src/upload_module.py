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

class UploadModule(wx.Panel):
    def __init__(self, parent, API):
        super(UploadModule, self).__init__(parent = parent)

        self.API = API
        self.editorManager = self.API.tabManager.GetCurrentPage()
        self.filename = self.editorManager.fileName
        self.API.motelistCallbacks.append(self.updateMotelist)
        # Just a shorter name
        self.tr = self.API.translater.translate
        self.tmpDir = self.API.path + '/temp/'
        self.haveMote = False
        self.platform = "telosb"
        self.moteOrder = list()
        # this is path from /mansos/tools/IDE
        self.pathToMansos = self.API.path + "/../.."
        self.motes = []

        self.main = wx.BoxSizer(wx.VERTICAL)
        self.controls = wx.GridBagSizer(10, 10)

        self.source = wx.ComboBox(self, choices = ["USB", "Shell"])
        self.source.SetValue("USB")
        self.platforms = wx.ComboBox(self, choices = self.API.getPlatforms())
        self.platforms.SetValue(self.API.getActivePlatform())
        self.compile = wx.Button(self, label = self.tr("Compile"))
        self.upload = wx.Button(self, label = self.tr("Upload"))
        self.refresh = wx.Button(self, label = self.tr("Refresh"))

        self.controls.Add(self.compile, (0, 0), flag = wx.EXPAND | wx.ALL)
        self.controls.Add(self.platforms, (0, 1), flag = wx.EXPAND | wx.ALL)
        self.controls.Add(self.upload, (0, 2), span = (2, 2),
                          flag = wx.EXPAND | wx.ALL)

        self.controls.Add(self.source, (1, 1), flag = wx.EXPAND | wx.ALL)
        self.controls.Add(self.refresh, (1, 0), flag = wx.EXPAND | wx.ALL)

        self.list = wx.CheckListBox(self, wx.ID_ANY, style = wx.MULTIPLE)

        self.main.Add(self.controls, 0, wx.EXPAND | wx.ALL, 3);
        self.main.Add(self.list, 0, wx.EXPAND | wx.ALL, 3);

        self.Bind(wx.EVT_BUTTON, self.API.doCompile, self.compile)
        self.Bind(wx.EVT_BUTTON, self.API.doUpload, self.upload)
        self.Bind(wx.EVT_BUTTON, self.populateMotelist, self.refresh)
        self.Bind(wx.EVT_COMBOBOX, self.populateMotelist, self.source)
        self.Bind(wx.EVT_COMBOBOX, self.API.changePlatform, self.platforms)
        self.Bind(wx.EVT_CHECKLISTBOX, self.modifyTargets, self.list)

        self.SetSizerAndFit(self.main)
        self.SetAutoLayout(1)
        self.Show()

        self.updateMotelist()

    def __del__(self):
        self.API.motelistCallbacks.remove(self.updateMotelist)

    def populateMotelist(self, event = None):
        self.list.Clear()
        self.list.Insert(self.tr("Searching devices") + "...", 0)
        self.list.Disable()
        self.API.populateMotelist()

    def updateMotelist(self):
        motelist = self.API.motelist
        self.list.Clear()

        if len(motelist) == 0:
            self.list.Insert(self.tr("No devices found!"), 0)
        else:
            for i in range(len(motelist)):
            #    print motelist[i][0] + "(" + motelist[i][2] + ") @ " + motelist[i][1], i, motelist[i]
                self.list.Insert(motelist[i][0] + "(" + motelist[i][2] +
                                 ") @ " + motelist[i][1], i)
            self.list.Enable()

    def modifyTargets(self, event):
        self.API.targets = list(self.list.GetChecked())

