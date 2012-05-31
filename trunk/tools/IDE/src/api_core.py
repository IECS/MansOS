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
from wx.lib.scrolledpanel import ScrolledPanel
import os
from time import gmtime, strftime

from frame import Frame
from seal_syntax import SealSyntax
from translater import Translater
from output_area import OutputArea
from tab_manager import TabManager
from upload_core import UploadCore
from output_tools import OutputTools
from listen_module import ListenModule
from editor_manager import EditorManager
from globals import * #@UnusedWildImport

from seal_parser import SealParser

class ApiCore:
    def __init__(self):
        self.path = os.getcwd()
        # All variables placed here will be saved to configuration file and 
        # reloaded next run time. See setSetting() and getSetting()
        # Note: settings in file are with higher priority.
        self.__settings = {
                   "activeLanguage" : "LV",
                   "platform" : "telosb"
               }
        # Read settings from file
        if os.path.exists(SETTING_FILE) and os.path.isfile(SETTING_FILE):
            f = open(SETTING_FILE, 'r')
            lines = f.readlines()
            for x in lines:
                if x != '':
                    if x.find("->") != -1:
                        if x[x.find("->") + 2:].find("->") == -1:
                            key, value = x.strip().split("->")
                        else:
                            continue
                    else:
                        continue
                    if key in self.__settings and value != self.__settings[key]:
                        # print because logging is not initialized yet :(
                        print "Replacing setting '{0}' with '{1}'."\
                            .format(self.__settings[key], value)
                    self.__settings[key] = value
            f.close()

        # All functions here will be called upon exit
        self.onExit = [self.saveSettings]

        # All defined platforms
        self.platforms = [
            "telosb",
            "sadmote",
            "atmega",
            "waspmote"
            ]

        if LOG_TO_FILE:
            path = os.getcwd()
            os.chdir(self.path)
            self.logFile = open(LOG_FILE_NAME, "a")
            os.chdir(path)
            self.onExit.append(self.logFile.close)

### Shortcuts etc

# Visual objects here can be used in forms only after they have been re-parented 
# using their Reparent() function, else they won't be visible!

        self.editors = list()
        self.emptyFrame = wx.Frame(None)

        # Defines seal syntax
        self.sealSyntax = SealSyntax(self)
        self.getKeywords = self.sealSyntax.getKeywords

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
        self.sealParser = SealParser("telosb", self.printInfo, False, True)

        # Init tab manager 
        self.tabManager = TabManager(self.emptyFrame, self)

        self.uploadCore = UploadCore(self, self.printInfo)
        self.uploadTargets = ([], self.tr('the default device'))

        # Init listenModule
        self.listenModule = ListenModule(self.emptyFrame, self)

        self.editPanel = ScrolledPanel(self.emptyFrame)

        self.frame = Frame(None, "MansOS IDE", (800, 500), (100, 100), self)

        #self.outputTools.addTools()

        icon = os.path.normpath('../../doc/mansos-32x32.ico')
        if os.path.exists(icon):
            self.frame.SetIcon(wx.Icon(icon, wx.BITMAP_TYPE_ICO, 32, 32))
        else:
            self.logMsg(LOG_WARNING, "Icon not found in '{}'!".format(icon))

        self.onExit.append(self.frame.Close)

        assert len(self.emptyFrame.GetChildren()) == 0, \
            "There are parentless objects after API initialization."

        self.tabManager.loadRememberedTabs()
        self.frame.auiManager.Update()

    def getStatementType(self, line):
        possibleSplitters = [None, ",", ";"]
        # Try split statement to parse actuator and object
        for x in possibleSplitters:
            actuator = line.split(x)
            if actuator != []:
                # If no object found, make sure there is actuator[1] to return :)
                actuator.append(" ")
                if actuator[0] in self.sealSyntax.syntax[0]:
                    return (STATEMENT, actuator[0], actuator[1].strip(",; "))
        return (UNKNOWN, '', '')

    def getActuatorInfo(self, actuator):
        if actuator in self.sealSyntax.actuators:
            return self.sealSyntax.actuators[actuator]
        # Return empty object
        return {
                'objects': [],
                'parameters': [],
                'role': UNKNOWN
                }

    def checkForDeletedEditors(self):
        toDrop = list()
        for x in range(len(self.editors)):
            if type(self.editors[x]) != EditorManager:
                toDrop.append(x)
        # Hack for deleting them in reverse order, so remaining list indexes remain correct
        for x in range(len(toDrop)):
            self.editors.pop(toDrop[len(toDrop) - x - 1])

    # Get all actuators, who have role == self.STATEMENT
    def getAllStatementActuators(self):
        return self.sealSyntax.actuators.keys()

    def getParamByName(self, parameters, name):
        assert type(parameters) is list, "List expected."
        for x in parameters:
            if x[0].lower() == name.lower():
                return x
        return None

    def getDefaultConditions(self):
        return self.sealSyntax.predefinedConditions

    def getPlatforms(self):
        return self.platforms

    def getSetting(self, setting):
        if setting in self.__settings:
            return self.__settings[setting]
        return ''

    def setSetting(self, name, value):
        self.__settings[name] = value
        # Make sure, that settings are saved on unexpected exit
        self.saveSettings()

    def saveSettings(self):
        os.chdir(self.path)
        f = open(SETTING_FILE, 'w')
        for key in self.__settings:
            f.write(str(key) + "->" + str(self.__settings[key]) + '\n')
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
            print "    Calling ", str(function)
            function()
