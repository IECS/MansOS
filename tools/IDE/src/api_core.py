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

import re
import string
import wx
import os
from time import gmtime, strftime

from frame import Frame
from seal_struct import SealStruct
from seal_syntax import SealSyntax
from translater import Translater
from seal_parser import SealParser
from output_area import OutputArea
from tab_manager import TabManager
from upload_core import UploadCore
from output_tools import OutputTools
from listen_module import ListenModule
from globals import * #@UnusedWildImport

class ApiCore:
    def __init__(self):
        self.path = os.getcwd()
        # Read settings from file
        if os.path.exists(SETTING_FILE) and os.path.isfile(SETTING_FILE):
            f = open(SETTING_FILE, 'r')
            lines = f.readlines()
            self.__settings = {}
            for x in lines:
                if x != '':
                    key, value = x.split(":")
                    self.__settings[key] = value.strip()
            f.close()
        else:
            # All variables placed here will be saved to configuration file and 
            # reloaded next run time. See setSetting() and getSetting()
            self.__settings = {
                   "activeLanguage" : "LV"
               }
        # All functions here will be called upon exit
        self.onExit = [self.saveSettings]


        if LOG_TO_FILE:
            path = os.getcwd()
            os.chdir(self.path)
            self.logFile = open(LOG_FILE_NAME, "a")
            os.chdir(path)
            self.onExit.append(self.logFile.close)

### Shortcuts etc

# Visual objects here can be used in forms only after they have been re-parented 
# using their Reparent() function, else they won't be visible!

        self.emptyFrame = wx.Frame(None)

        # Defines seal syntax
        self.sealSyntax = SealSyntax()

        # Compile regex for finding actuators
        self.__reActuators = re.compile(string.join(self.sealSyntax.actuators.keys(), '|'), re.I)

        # Init translation module
        self.translater = Translater(self)
        self.tr = self.translater.translate

        # Init output_tools
        self.outputTools = OutputTools(self.emptyFrame, self)

        # Init outputArea for info, 1st tab
        self.infoArea = OutputArea(self.emptyFrame, self, 0)
        self.printInfo = self.infoArea.printLine
        self.clearInfoArea = self.infoArea.clear

        # Init outputArea for output, 2nd tab
        self.outputArea = OutputArea(self.emptyFrame, self, 1)
        self.printOutput = self.outputArea.printLine
        self.clearOutputArea = self.outputArea.clear

        # Init seal parser
        self.sealParser = SealParser(self.printInfo)
        self.seal = SealStruct(self)

        self.editorSplitter = wx.SplitterWindow(self.emptyFrame,
                                                style = wx.SP_LIVE_UPDATE)
        self.editorSplitter.doSplit = None
        self.editorSplitter.SetMinimumPaneSize(305)
        self.editorSplitter.SetSashGravity(1)

        # Init tab manager 
        self.tabManager = TabManager(self.emptyFrame, self)

        self.uploadCore = UploadCore(self, self.printInfo)
        self.uploadTargets = ([], self.tr('default device'))

        # Init listenModule
        self.listenModule = ListenModule(self.emptyFrame, self)

        self.outputTools.addTools()

        self.editPanel = wx.Panel(self.emptyFrame)

        self.frame = Frame(None, "MansOS IDE", (800, 500), (100, 100), self)
        self.onExit.append(self.frame.Close)

        print "List of parentless objects(empty list is good!):"
        print str(self.emptyFrame.GetChildren()).replace(", ", "\n").strip("wxWindowList: []")

    # Return regex for finding actuators
    def getReActuators(self):
        return self.__reActuators

    def getRole(self, actuator):
        return self.sealSyntax.actuators[actuator]['role']

    def getStatementType(self, line):
        actuator = line.split(None, 1)
        if actuator != []:
            if actuator[0] in self.sealSyntax.actuators:
                return self.sealSyntax.actuators[actuator[0]]['role']
        return UNKNOWN

    def getActuatorInfo(self, actuator):
        if actuator in self.sealSyntax.actuators:
            return self.sealSyntax.actuators[actuator]
        # Return empty object
        return {
                'objects': [],
                'parameters': [],
                'role': UNKNOWN
                }

    # Get all actuators, who have role == self.STATEMENT
    def getAllStatementActuators(self):
        result = []
        for x in self.sealSyntax.actuators.keys():
            if self.sealSyntax.actuators[x]['role'] == STATEMENT:
                result.append(x)
        return result

    def getDefaultConditions(self):
        return self.sealSyntax.actuators['when']['objects']

    def getPlatforms(self):
        return self.sealSyntax.platforms

    def getSetting(self, setting):
        if setting in self.__settings:
            return self.__settings[setting]
        return ''

    def setSetting(self, name, value):
        self.__settings[name] = value
        # Make sure, that settings are saved on unexpected exit
        # XXX: is this correct???
        self.saveSettings()

    def saveSettings(self):
        os.chdir(self.path)
        f = open(SETTING_FILE, 'w')
        for key in self.__settings:
            f.write(key + ":" + self.__settings[key] + '\n')
        f.close()

    def logMsg(self, msgType, msgText):
        if msgType <= LOG:
            # Generate message
            dbgTime = str(strftime("%H:%M:%S %d.%m.%Y", gmtime())) + ": "
            dbgMsg = LOG_TEXTS[msgType] + " - " + str(msgText) + '\n'
            if LOG_TO_CONSOLE:
                print dbgMsg
                self.printInfo(dbgMsg)
            if LOG_TO_FILE:
                self.logFile.write(dbgTime + dbgMsg)

    def performExit(self):
        print "Prepering to exit:"
        for function in self.onExit:
            print "    Calling ", function
            function()
