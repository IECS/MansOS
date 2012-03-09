# -*- coding: UTF-8 -*-
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

import doCompile
import doUpload
import getMotelist
import threadRunner
import globals as g

class UploadModule(wx.Dialog):
    def __init__(self, parent, title, API):
        super(UploadModule, self).__init__(parent = parent, 
            title = title, size = (500, 400), style = wx.DEFAULT_DIALOG_STYLE 
                                                    | wx.RESIZE_BORDER)
        self.API = API
        self.editorManager = self.GetParent().tabManager.getPageObject()
        self.filename = self.editorManager.fileName
        # Just a shorter name
        self.tr = self.API.translater.translate
        self.tmpDir = self.API.path + '/temp/'
        self.haveMote = False
        self.platform = "telosb"
        # this is path from /mansos/tools/IDE
        self.pathToMansos = self.API.path + "/../.." 
        self.motes = []
        
        # Used classes
        self.compiler = doCompile.DoCompile(self.API)
        self.uploader = doUpload.DoUpload(self.pathToMansos)
        self.motelist = getMotelist.GetMotelist(self.pathToMansos)
        
        
        self.main = wx.BoxSizer(wx.VERTICAL)
        self.controls = wx.GridBagSizer(10,10)
        
        self.output = wx.TextCtrl(self, style = wx.EXPAND | wx.ALL, 
                                  size = (470, 300))
        self.output.SetBackgroundColour("Black")
        self.output.SetForegroundColour("White")
        
        self.source = wx.ComboBox(self, choices = ["USB", "Shell"])
        self.source.SetValue("USB")
        self.platforms = wx.ComboBox(self, choices = self.API.getPlatforms())
        self.platforms.SetValue(self.API.getPlatforms()[0])
        self.compile = wx.Button(self, label=self.tr("Compile"))
        self.upload = wx.Button(self, label=self.tr("Upload"))
        self.refresh = wx.Button(self, label=self.tr("Refresh"))
        
        self.controls.Add(self.compile, (0,0), flag = wx.EXPAND | wx.ALL)
        self.controls.Add(self.platforms, (0,1),flag = wx.EXPAND | wx.ALL)
        self.controls.Add(self.upload, (0,2), span = (2,2),flag = wx.EXPAND | wx.ALL)
        
        self.controls.Add(self.source, (1,1), flag = wx.EXPAND | wx.ALL)
        self.controls.Add(self.refresh, (1,0), flag = wx.EXPAND | wx.ALL)
        
        self.list = wx.CheckListBox(self, wx.ID_ANY, style=wx.MULTIPLE)
        
        self.main.Add(self.controls, 
                      0,            # make vertically unstretchable
                      wx.EXPAND |    # make horizontally stretchable
                      wx.ALL,        #   and make border all around
                      3 );
        self.main.Add(self.list, 
                      0,            # make vertically unstretchable
                      wx.EXPAND |    # make horizontally stretchable
                      wx.ALL,        #   and make border all around
                      3 );
        self.main.Add(self.output, 
                      1,            # make vertically stretchable
                      wx.EXPAND |    # make horizontally stretchable
                      wx.ALL,        #   and make border all around
                      10 );         # set border width to 10)
        
        self.Bind(wx.EVT_BUTTON, self.manageCompile, self.compile)
        self.Bind(wx.EVT_BUTTON, self.manageUpload, self.upload)
        self.Bind(wx.EVT_BUTTON, self.populateMotelist, self.refresh)
        self.Bind(wx.EVT_COMBOBOX, self.populateMotelist, self.source)
        self.Bind(wx.EVT_COMBOBOX, self.changePlatform, self.platforms)
        
        self.SetSizer(self.main)
        self.main.Fit(self)
        self.Show()
        
        self.populateMotelist()
        
    def populateMotelist(self, event = None):
        self.updateStatus("")
        self.list.Clear()
        self.motes = []
        self.list.Insert(self.tr("Searching devices") + "...", 0)
        self.list.Disable()
        source = self.source.GetValue()
        res = []
        if source == 'Shell':
            res = self.managePopen(self.motelist.getShellMotelist)
            self.targetType = g.SHELL
            
        else:
            res = self.managePopen(self.motelist.getMotelist)
            self.targetType = g.USB

        motelist = res[1]
        self.haveMote = res[0]
        
        self.updateStatus(self.tr("Got") + " " + str(len(motelist)) + " " +
                    self.tr("devices in") + " " + str(round(res[2], 3)) + " s.")
        self.list.Delete(0)
        
        if len(motelist) == 0:
            self.list.Insert(self.tr("No devices found!"), 0)
        else:
            for i in range(0, len (motelist)):
                self.list.Insert(motelist[i][0] + "(" + motelist[i][2] +
                                 ") @ " + motelist[i][1], i, motelist[i])
                self.motes.append(motelist[i][self.targetType])
            self.list.Enable()
            
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
    def runCompile(self):
        return self.compiler.doCompile(self.editorManager.fileName, 
                                self.platform, self.editorManager.filePath, 
                                self.editorManager.projectType, False)
        
    def manageUpload(self, event = None):
        if not self.haveMote:
            self.updateStatus(self.tr("No devices found!"))
            return
        
        checked = self.list.GetChecked()
        self.targets = []
        targetText = ''
        for x in checked:
            self.targets.append(self.motes[x])
        if self.targets == []:
            self.targets = [None]
            targetText = self.tr('default device')
        else:
            targetText = str(self.targets)
        
        self.updateStatus(self.tr("Starting upload on") + " " + targetText)
        
        res = self.managePopen(self.runUpload)
        self.compiler.clean()
        if res[0] == True:
            self.updateStatus(self.tr("Uploaded successfully in") + 
                              " " + str(round(res[2], 3)) + " s.")
        else:
            self.updateStatus(self.tr("Upload failed with message:") + 
                              "\n" + res[1])
    
    # Wrapping for compiler function to run in new thread
    def runUpload(self):
        return self.uploader.doUpload(self.targets, self.runCompile, 
                                      self.targetType, self.platform)
        
    def managePopen(self, funct):
        # Must be list, here returned values will be stored
        data = []
        newThread = threadRunner.threadRunner(funct, data, self.API)
        newThread.start()
        # Wait for thread to finish and yield, so UI doesn't die
        # TODO: add indicator that working now
        while newThread.isAlive():
            wx.YieldIfNeeded()
            newThread.join(0.001)
        return data

    def changePlatform(self, event):
        self.platform = event.GetEventObject().GetValue()
        self.updateStatus(self.tr("Changed platform to") + " "  + self.platform)
    
    def updateStatus(self, message, overwrite = True):
        if overwrite:
            self.output.SetValue(message)
        else:
            self.output.SetValue(message + "\n" + self.output.GetValue())
        wx.YieldIfNeeded()

