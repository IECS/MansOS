#!/usr/bin/env python
import wx
import subprocess
import sys
import os
import shutil
import globals as g

class UploadModule(wx.Dialog):
    def __init__(self, parent, title, API):
        super(UploadModule, self).__init__(parent = parent, 
            title = title, size = (500, 400), style=wx.DEFAULT_DIALOG_STYLE | wx.RESIZE_BORDER)
        self.filename = "SEALlang"
        self.API = API
        self.editorManager = self.GetParent().tabManager.getPageObject()
        # Just a shorter name
        self.tr = self.API.translater.translate
        self.tmpDir = self.API.path + '/temp/'
        self.haveMote = False
        self.platform = "telosb"
        self.pathToMansos = self.API.path + "/../.."     # this is path from temp directory
        
        self.main = wx.BoxSizer(wx.VERTICAL)
        self.controls = wx.GridBagSizer(10,10)
        
        self.output = wx.TextCtrl(self, style = wx.EXPAND | wx.ALL, size = (470, 300))
        self.output.SetBackgroundColour("Black")
        self.output.SetForegroundColour("White")
        
        self.ports = wx.ComboBox(self, choices = [], size = (300, -1))
        self.platforms = wx.ComboBox(self, choices = self.API.getPlatforms(), size = (300, -1))
        self.platforms.SetValue(self.API.getPlatforms()[0])
        self.compile = wx.Button(self, label=self.tr("Compile"))
        self.upload = wx.Button(self, label=self.tr("Upload"))
        self.refresh = wx.Button(self, label=self.tr("Refresh"))
        
        self.controls.Add(self.compile, (0,0), flag = wx.EXPAND | wx.ALL);
        self.controls.Add(self.platforms, (0,1), flag = wx.EXPAND | wx.ALL)
        
        self.controls.Add(self.upload, (1,0), flag = wx.EXPAND | wx.ALL)
        self.controls.Add(self.ports, (1,1), flag = wx.EXPAND | wx.ALL)
        self.controls.Add(self.refresh, (1,2), flag = wx.EXPAND | wx.ALL)
        
        self.main.Add(self.controls, 
                      0,            # make vertically unstretchable
                      wx.EXPAND |    # make horizontally stretchable
                      wx.ALL,        #   and make border all around
                      3 );
        self.main.Add(self.output, 
                      1,            # make vertically stretchable
                      wx.EXPAND |    # make horizontally stretchable
                      wx.ALL,        #   and make border all around
                      10 );         # set border width to 10)
        
        self.getMotelist()
        
        self.Bind(wx.EVT_BUTTON, self.doCompile, self.compile)
        self.Bind(wx.EVT_BUTTON, self.doUpload, self.upload)
        self.Bind(wx.EVT_BUTTON, self.getMotelist, self.refresh)
        self.Bind(wx.EVT_COMBOBOX, self.changeTarget, self.ports)
        self.Bind(wx.EVT_COMBOBOX, self.changePlatform, self.platforms)
        
        self.SetSizer(self.main)
        self.main.Fit(self)
        self.Show()
    
    def changePlatform(self, event):
        self.platform = event.GetEventObject().GetValue()
        self.updateStatus(self.tr("Changed platform to") + " "  + self.platform)
    
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
        
        for x in motes[motes.rfind("-----") + 6: ].split("\n"):
            # seperate ID, port, description
            y = x.split(None, 2)
            if y != []:
                motelist.append(y)
        for x in motelist:
            if len(x) > 2:
                self.ports.Append(x[1]+" - "+ x[2])
        self.haveMote = True
        self.ports.SetValue(self.tr("Use default device"))
        return motelist
    
    def changeTarget(self, event):
        target = event.GetEventObject().GetValue()
        os.environ["BSLPORT"] = target[:target.find(" ")]
        #print "Changed target to",os.environ["BSLPORT"]

    def updateStatus(self, message, overwrite = True):
        if overwrite:
            self.output.SetValue(message)
        else:
            self.output.SetValue(message + "\n" + self.output.GetValue())
    
    def doCompile(self, event = None):
        goodPath = self.editorManager.filePath.rfind('/')
        if goodPath != -1:
            os.chdir(self.editorManager.filePath[:goodPath])
        else:
            os.chdir(self.API.path)
        if not os.path.exists("Makefile") or not os.path.isfile("Makefile"):
            with open("Makefile", "w") as out:
                if self.editorManager.projectType == g.SEAL_PROJECT:
                    out.write(self.generateMakefile(self.editorManager.fileName, "SEAL_SOURCES"));
                else:
                    out.write(self.generateMakefile(self.editorManager.fileName));
        try:
            #retcode = subprocess.call(["make", "telosb", "upload-msp430", " > ", "lastCallOutput"], True)
            self.updateStatus(self.tr("Starting compile..."))
            # Realy should call this in new thread
            upload = subprocess.Popen(["make", self.platform], 
                                      stderr = subprocess.STDOUT,
                                      stdout = subprocess.PIPE)
            out = upload.communicate()[0]
            print out
            successfull = out.rfind("saving Makefile.platform")
            
            if event == None:
                clean = subprocess.Popen(["make", "clean"], 
                                      stderr = subprocess.STDOUT,
                                      stdout = subprocess.PIPE)
                clean.communicate()
                
            if successfull != -1:
                self.updateStatus(self.tr("Compiled successfully"))
                return True
            else:
                self.updateStatus(out)
                return False
            
        except OSError, e:
            print >>sys.stderr, "execution failed:", e
            self.updateStatus(str(e))
            
    def doUpload(self, event):
        if self.haveMote == False:
            self.updateStatus(self.tr("No devices found"))
            return
        if self.doCompile() == False:
            return
        try:
            #retcode = subprocess.call(["make", "telosb", "upload-msp430", " > ", "lastCallOutput"], True)
            self.updateStatus(self.tr("Starting upload..."))
            # Realy should call this in new thread
            upload = subprocess.Popen(["make", self.platform, "upload"], 
                                      stderr = subprocess.STDOUT,
                                      stdout = subprocess.PIPE)
            out = upload.communicate()[0]
            print out
            haveUploadError = out.rfind("An error occoured")
            if haveUploadError != -1:
                self.updateStatus(out[haveUploadError:])
                return
            haveCompileError = out.rfind("Error")
            if haveCompileError != -1:
                self.updateStatus(out)
                return
            self.updateStatus(self.tr("Uploaded successfully!"))
            clean = subprocess.Popen(["make", "clean"], 
                                      stderr = subprocess.STDOUT,
                                      stdout = subprocess.PIPE)
            clean.communicate()
        except OSError, e:
            print >>sys.stderr, "execution failed:", e
            self.updateStatus(str(e))
        
    def generateMakefile(self, fileName='main.c', sourceType = 'SOURCES'):
        return """#-*-Makefile-*- vim:syntax=make
#
# Copyright (c) 2008-2010 Leo Selavo and the contributors. All rights reserved.
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
# --------------------------------------------------------------------
#    Makefile for the sample application
#
#  The developer must define at least SOURCES and APPMOD in this file
#
#  In addition, PROJDIR and MOSROOT must be defined, before including 
#  the main Makefile at ${MOSROOT}/mos/make/Makefile
# --------------------------------------------------------------------

# Sources are all project source files, excluding MansOS files
""" + sourceType + " = " + fileName + """

# Module is the name of the main module built by this makefile
APPMOD = """ + self.editorManager.fileName + """

# --------------------------------------------------------------------
# Set the key variables
PROJDIR = $(CURDIR)
ifndef MOSROOT
  MOSROOT = """ + self.pathToMansos + """
endif

# Include the main makefile
include ${MOSROOT}/mos/make/Makefile
"""
    
    def saveToFile(self):
        self.checkForTempDir()
        with open(self.tmpDir + self.filename, "w") as out:
            # Maybe should try to make this shorter
            out.write(self.editorManager.code.GetText())
    
    def checkForTempDir(self):
        if not os.path.exists(self.tmpDir):
            os.makedirs(self.tmpDir)

    def removeTmpDir(self):
        #os.rmdir(self.tmpDir)
        shutil.rmtree(self.tmpDir)


