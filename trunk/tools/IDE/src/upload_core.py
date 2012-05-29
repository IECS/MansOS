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

from globals import * #@UnusedWildImport
from do_compile import DoCompile
from do_upload import DoUpload
from get_motelist import GetMotelist
from thread_runner import ThreadRunner

class UploadCore():
    def __init__(self, API, printLine):

        self.API = API
        self.syncWithTabManager()
        # Just a shorter name
        self.tr = self.API.translater.translate
        self.tmpDir = self.API.path + '/temp/'
        self.haveMote = False
        self.platform = "telosb"
        self.printMsg = printLine

        # this is path from /mansos/tools/IDE
        self.pathToMansos = self.API.path + "/../.."
        self.motes = []

        # Used classes
        self.compiler = DoCompile(self.API)
        self.uploader = DoUpload(self.pathToMansos)
        self.motelist = GetMotelist(self.pathToMansos)

    def populateMotelist(self, event = None, source = None, quiet = False):
        self.motes = []
        res = []
        if source == 'Shell':
            res = self.managePopen(self.motelist.getShellMotelist)
            self.targetType = SHELL

        else:
            res = self.managePopen(self.motelist.getMotelist)
            self.targetType = USB

        motelist = res[1]
        self.haveMote = res[0]

        if not quiet:
            self.updateStatus(self.tr("Got") + " " + str(len(motelist)) + " " +
                    self.tr("devices in") + " " + str(round(res[2], 3)) + " s.")

        if type(motelist) is not list:
            return []

        if len(motelist) != 0:
            for i in range(0, len (motelist)):
                if len(motelist[i]) > 1:
                    self.motes.append(motelist[i][self.targetType])

        return motelist

    def manageCompile(self, event = None):
        self.updateStatus(self.tr("Starting compile") + "...")
        res = self.managePopen(self.runCompile)
        self.compiler.clean()
        if res[0] == True:
            self.updateStatus(self.tr("Compiled successfully in") + " " +
                              str(round(res[2], 3)) + " s.")
        else:
            self.updateStatus(self.tr("Compile failed with message:") +
                              "\n" + res[1])

    # Wrapping for compiler function to run in new thread
    def runCompile(self, dataIn):
        self.syncWithTabManager()
        return self.compiler.doCompile(self.editorManager.fileName,
                                self.platform, self.editorManager.filePath,
                                self.editorManager.projectType, False)

    def manageUpload(self, event = None):
        self.populateMotelist(None, self.platform)

        if not self.haveMote:
            self.updateStatus(self.tr("No devices found!"))
            return

        self.targets, targetText = self.API.uploadTargets
        self.updateStatus(self.tr("Starting upload on") + " " + targetText)

        res = self.managePopen(self.runUpload)
        self.compiler.clean()

        if res[0] == True:
            self.updateStatus(self.tr("Uploaded successfully in") +
                              " " + str(round(res[2], 3)) + " s.")
        else:
            self.updateStatus(self.tr("Upload failed with message:") +
                              "\n" + res[1])

    # Wrapping for upload function to run in new thread
    def runUpload(self, dataIn):
        return self.uploader.doUpload(self.targets, self.runCompile,
                                      self.targetType, self.platform)

    def managePopen(self, funct):
        # Must be list, here returned values will be stored
        dataOut = []
        dataIn = []
        newThread = ThreadRunner(funct, dataIn, dataOut, self.API)
        newThread.start()
        # Wait for thread to finish and yield, so UI doesn't die
        # TODO: add indicator that working now
        while newThread.isAlive():
            wx.YieldIfNeeded()
            newThread.join(0.001)
        return dataOut

    def changePlatform(self, event):
        self.platform = event.GetEventObject().GetValue()
        self.updateStatus(self.tr("Changed platform to") + " " + self.platform)

    def updateStatus(self, message):
        self.printMsg(message)

    def syncWithTabManager(self):
        self.editorManager = self.API.tabManager.GetCurrentPage()
        self.filename = self.editorManager.fileName

