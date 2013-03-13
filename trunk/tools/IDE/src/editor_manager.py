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

from globals import * #@UnusedWildImport
from editor import Editor
from Translater import localize

class EditorManager(wx.Panel):
    def __init__(self, parent, API):
        wx.Panel.__init__(self, parent)

        self.API = API;
        self.API.editors.append(self)

        self.initUI()
        self.lastSaved = ''
        ### Editor visible variables
        # @ creation we assume document is not saved.
        self.saveState = False
        # Filename or untitled document
        self.fileName = localize('Untitled') + str(self.GetParent().nextPageNr) + '.sl'
        # Filename and full path(relative or absolute)
        self.filePath = localize('Untitled') + str(self.GetParent().nextPageNr) + '.sl'
        # This marks if document already have a file attached to it
        self.hasAFile = False
        # Define project type
        self.projectType = SEAL_PROJECT
        self.detectSEAL()

    def update (self, initFilePath = ''):
        self.API.frame.fileHistory.AddFileToHistory(initFilePath)
        self.API.frame.fileHistory.Save(self.API.config)
        self.API.config.Flush()
        if initFilePath == '':
            initFilePath = self.filePath
        if os.path.exists(initFilePath) and os.path.isfile(initFilePath):
            # Load file into editor
            self.code.LoadFile(initFilePath)
            self.code.SetText(self.code.GetText().replace("\r\n", "\n"))
            # Draw margins
            self.changeCode('', False)
            # Update editor info
            self.updateInfo(initFilePath, saveState = True, hasAFile = True)
            self.GetParent().titleChange(os.path.split(self.fileName)[1])
            self.detectSEAL()
        else:
            self.changeCode()
        self.lastSaved = self.code.GetText()
        self.yieldChanges()

        self.parseConfigFile()

    def initUI(self):
        self.main = wx.BoxSizer(wx.HORIZONTAL)

        self.code = Editor(self, self.API)
        self.main.Add(self.code, 1, wx.EXPAND | wx.ALL, 10);

    def redrawAll(self):
        #Layout sizers
        self.SetSizer(self.main)
        self.SetAutoLayout(1)
        self.main.Fit(self)
        self.Show()

    def changeCode(self, newCode = '', overwrite = True):
        if overwrite == True:
            self.code.SetText(newCode)
        else:
            self.code.AddText(newCode)
        self.yieldChanges()
        self.code.setLineNumbers()
        self.redrawAll()

    def save(self, haveChanged = True):
        self.lastSaved = self.code.GetText()
        if haveChanged:
            self.code.SaveFile(self.filePath)
        self.saveState = True
        self.GetParent().markAsSaved()

    def updateInfo(self, path = '', fileName = '', saveState = '',
                   hasAFile = '', projectType = None):
        if path != '':
            self.filePath = path
            self.fileName = os.path.split(path)[1]
        if fileName != '':
            self.fileName = os.path.split(fileName)[1]
        if saveState != '':
            self.saveState = saveState
        if hasAFile != '':
            self.hasAFile = hasAFile
        if projectType != None:
            self.projectType = projectType

    def detectSEAL(self):
        if self.fileName[-2:] == "sl":
            self.projectType = SEAL_PROJECT
            self.code.highlightSeal()
        else:
            self.projectType = MANSOS_PROJECT
            self.code.highlightC()

        if self.API.loaded:
            self.API.editWindow.update()

    def yieldChanges(self):
        if self.code.GetText() != self.lastSaved:
            self.saveState = False
            self.API.tabManager.markAsUnsaved()
        else:
            self.save(False)

    def parseConfigFile(self):
        self.API.platformOnly = None
        path = os.path.split(self.filePath)[0]
        if os.path.isdir(path):
            if os.path.isfile(os.path.join(path, "config")):
                f = open(os.path.join(path, "config"), "r")
                self.API.excludedPlatforms = list()
                for x in f.readlines():
                    if x.startswith("PLATFORM_ONLY"):
                        platform = x.split("=")[1].strip()
                        if platform in self.API.platforms:
                            self.API.platformOnly = platform
                    if x.startswith("PLATFORM_EXCLUDE"):
                        platforms = x.split("=")[1].strip().split(" ")
                        for target in platforms:
                            if target in self.API.platforms:
                                self.API.excludedPlatforms.append(target)
