#!/usr/bin/env python
import wx
import subprocess
import sys
import os
import shutil

class UploadModule(wx.Dialog):
    def __init__(self, parent, title, API):
        super(UploadModule, self).__init__(parent = parent, 
            title = title, size = (500, 400), style=wx.DEFAULT_DIALOG_STYLE | wx.RESIZE_BORDER)
        self.filename = "SADlang"
        self.API = API
        self.tmpDir = self.API.path + '/temp/'
        self.haveMote = False
        self.platform = "telosb"
        self.pathToMansos = self.API.path + "/../.."     # this is path from temp directory
        
        self.main = wx.BoxSizer(wx.VERTICAL)
        self.compileControls = wx.BoxSizer(wx.HORIZONTAL)
        self.uploadControls = wx.BoxSizer(wx.HORIZONTAL)
        
        self.output = wx.TextCtrl(self, style = wx.EXPAND | wx.ALL, size = (470, 300))
        self.output.SetBackgroundColour("Black")
        self.output.SetForegroundColour("White")
        
        self.ports = wx.ComboBox(self, choices = [], size = (300, -1))
        self.platforms = wx.ComboBox(self, choices = self.API.getPlatforms(), size = (300, -1))
        self.platforms.SetValue(self.API.getPlatforms()[0])
        self.compile = wx.Button(self, label="Compile")
        self.upload = wx.Button(self, label="Upload")
        self.refresh = wx.Button(self, label="Refresh")
        
        self.compileControls.Add(self.compile)
        self.compileControls.Add(self.platforms)
        
        self.uploadControls.Add(self.upload)
        self.uploadControls.Add(self.ports)
        self.uploadControls.Add(self.refresh)
        
        self.main.Add(self.compileControls, 
                      0,            # make vertically unstretchable
                      wx.EXPAND |    # make horizontally stretchable
                      wx.ALL,        #   and make border all around
                      3 );
        self.main.Add(self.uploadControls, 
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
        print "Changed platform to", self.platform
        self.updateStatus("Changed platform to " + self.platform)
    
    def getMotelist(self, event = None):
        motelist = []
        # get motelist output as string... |-(
        process = subprocess.Popen([self.API.path + "/../../mos/make/scripts/motelist"], 
                                      stderr = subprocess.STDOUT,
                                      stdout = subprocess.PIPE)
        motes, err = process.communicate()
        
        # remove table header and split into lines, ("-----") + 6 used to be 
        # sure it's end of header, not random symbol in data
        self.ports.Clear()
        if motes.find("No devices found") == False:
            self.haveMote = False
            self.ports.SetValue("No devices found")
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
        self.ports.SetValue("Use default device")
        return motelist
    
    def changeTarget(self, event):
        target = event.GetEventObject().GetValue()
        os.environ["BSLPORT"] = target[:target.find(" ")]
        print "Changed target to",os.environ["BSLPORT"]

    def updateStatus(self, message, overwrite = True):
        if overwrite:
            self.output.SetValue(message)
        else:
            self.output.SetValue(message + "\n" + self.output.GetValue())
        
    def doCompile(self, event):
        try:
            self.updateStatus("Starting compile...")
            self.saveToFile()
            os.chdir(self.tmpDir)
            upload = subprocess.Popen([self.API.path + "/../seal/seal", self.tmpDir + self.filename], 
                                      stderr = subprocess.STDOUT,
                                      stdout = subprocess.PIPE)
            out, err = upload.communicate()
            haveError = out.rfind("error in")
            if haveError != -1:
                self.updateStatus(out[haveError:])
            else:
                self.updateStatus("Compiled successfully!")
            if event != None:
                self.removeTmpDir()
        except OSError, e:
            wx.MessageBox("Exception" + str(sys.stderr), 'Error', 
                          wx.OK | wx.ICON_ERROR)
            print >>sys.stderr, "execution failed:", e
            self.updateStatus(str(e))
            
    def doUpload(self, event):
        if self.haveMote == False:
            self.updateStatus("No devices found")
            return
        self.doCompile(None)
        self.createMakefile()
        try:
            #retcode = subprocess.call(["make", "telosb", "upload-msp430", " > ", "lastCallOutput"], True)
            self.updateStatus("Starting upload...")
            # Realy shpuld call this in new thread
            os.chdir(self.tmpDir)
            upload = subprocess.Popen(["make", self.platform, "upload"], 
                                      stderr = subprocess.STDOUT,
                                      stdout = subprocess.PIPE)
            out, err = upload.communicate()
            print out
            haveUploadError = out.rfind("An error occoured")
            if haveUploadError != -1:
                self.updateStatus(out[haveUploadError:])
                return
            haveCompileError = out.rfind("Error")
            if haveCompileError != -1:
                self.updateStatus(out)
                return
            self.updateStatus("Uploaded successfully!")
            if event != None:
                self.removeTmpDir()
        except OSError, e:
            print >>sys.stderr, "execution failed:", e
            self.updateStatus(str(e))
        
    def createMakefile(self):
        self.checkForTempDir()
        #if os.path.exists("Makefile") & os.path.isfile("Makefile"):
        #   self.updateStatus("Overwriting Makefile.")
        with open(self.tmpDir +"Makefile", "w") as out:
            out.write("""#-*-Makefile-*- vim:syntax=make
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
SOURCES = main.c

# Module is the name of the main module built by this makefile
APPMOD = Blink

# --------------------------------------------------------------------
# Set the key variables
PROJDIR = $(CURDIR)
ifndef MOSROOT
  MOSROOT = $(PROJDIR)""" + self.pathToMansos + """
endif

# Include the main makefile
include ${MOSROOT}/mos/make/Makefile
""")
    
    def saveToFile(self):
        self.checkForTempDir()
        with open(self.tmpDir + self.filename, "w") as out:
            # Maybe should try to make this shorter
            out.write(self.GetParent().tabManager.GetPage(
                self.GetParent().tabManager.GetSelection()).code.GetText())
    
    def checkForTempDir(self):
        if not os.path.exists(self.tmpDir):
            os.makedirs(self.tmpDir)

    def removeTmpDir(self):
        #os.rmdir(self.tmpDir)
        shutil.rmtree(self.tmpDir)


