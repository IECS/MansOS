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
from listen_module import ListenModule
from editor_manager import EditorManager
from get_motelist import GetMotelist
from do_compile import DoCompile
from do_upload import DoUpload
from edit_statement import EditStatement
from blockly import Blockly
from globals import * #@UnusedWildImport

#from seal_parser import SealParser
from seal import seal_parser

class ApiCore:
    def __init__(self, argv):
        self.loaded = False
        self.config = wx.Config("MansOS-IDE", style = wx.CONFIG_USE_LOCAL_FILE)

        self.path = os.getcwd()
        # All variables placed here will be saved to configuration file and 
        # reloaded next run time. See setSetting() and getSetting()
        # Note: settings in file are with higher priority.
        self.__settings = {
                   "activeLanguage" : "LV",
                   "platform" : "telosb",
                   "blocklyPort" : '8090',
                   "blocklyHost" : "localhost",
                   "blocklyLocation" : "../../../seal-blockly/blockly/seal/playground-seal.html",
                   "recentlyOpenedMaxCount" : 10
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
                    if key in self.__settings and str(value) != str(self.__settings[key]):
                        # print because logging is not initialized yet :(
                        print "Replacing setting '{0}' with '{1}'."\
                            .format(self.__settings[key], value)
                    self.__settings[key] = value
            f.close()

        # All functions here will be called upon exit
        self.onExit = [self.saveSettings]

        # All defined platforms
        self.platforms = self.getPlatformsFromMakefile()

        self.platformOnly = None
        self.excludedPlatforms = list()

        self.activePlatform = self.platforms.index("telosb")
        #
        self.motelist = []
        self.motelistCallbacks = []

        # Flag indicates that next thread's output shouldn't trigger 
        # force switching to info area tab.
        self.supressTabSwitching = False

        self.targets = [None]
        self.targetType = "USB"

        self.activeThreads = {}

        self.onExit.append(self.killAllThreads)

        if LOG_TO_FILE:
            path = os.getcwd()
            os.chdir(self.path)
            self.logFile = open(LOG_FILE_NAME, "a")
            os.chdir(path)
            self.onExit.append(self.logFile.close)

        # this is path from /mansos/tools/IDE
        self.pathToMansos = os.path.join(self.path, "../..")

        # Try to get system default font
        #font = wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT)
        #self.fontName = font.GetFaceName()
        #if self.fontName != "":
        #    print "Using system default font: {}".format(self.fontName)
        #else:
        self.fontName = "Courier New"
        #    print "Can't find system default font, defaulting to {}".\
        #                format(self.fontName)

        self.listenModules = list()

        self.editors = list()

        icon = os.path.normpath('../../doc/mansos-32x32.ico')

### Module initializations

# Visual objects here can be used in forms only after they have been re-parented 
# using their Reparent() function, else they won't be visible!

        self.emptyFrame = wx.Frame(None)

        # Defines seal syntax
        self.sealSyntax = SealSyntax(self)

        # Init translation module
        self.translater = Translater(self)
        self.tr = self.translater.translate

        # Init output_tools
        #self.outputTools = OutputTools(self.emptyFrame, self)

        # Init outputArea for info, 1st tab
        self.infoArea = OutputArea(self.emptyFrame, self, 0)
        self.printInfo = self.infoArea.printLine
        self.clearInfoArea = self.infoArea.clear

        # Init blockly handler
        self.blockly = Blockly(self.emptyFrame, self)

        # Init seal parser
        self.sealParser = seal_parser.SealParser("msp430", self.printInfo, False, True)

        # Init tab manager 
        self.tabManager = TabManager(self.emptyFrame, self)

        # Init listenModule
        self.listenModules.append(ListenModule(self.emptyFrame, self))

        self.editPanel = ScrolledPanel(self.emptyFrame)

        self.editWindow = EditStatement(self.editPanel, self)

        self.frame = Frame(None, "MansOS IDE", (0, 0), (0, 0), self)

        #self.outputTools.addTools()

        self.motelistClass = GetMotelist(self.pathToMansos, self)
        self.compiler = DoCompile(self)
        self.uploader = DoUpload(self)

### Shortcuts

# This allows modules to be disabled and dummy functions attached, so other 
# modules can keep saving the day... Each module updates his functions at 
# startup and restores them at termination. All function calls between modules 
# should go through here, but this ain't perfect world :(

        self.getKeywords = self.sealSyntax.getKeywords
        self.tr = self.translater.translate
        #self.printInfo = self.dummyPrint
        self.printOutput = self.dummyPrint

# Check if icon can be found
        if os.path.exists(icon):
            self.frame.SetIcon(wx.Icon(icon, wx.BITMAP_TYPE_ICO, 32, 32))
        else:
            self.logMsg(LOG_WARNING, "Icon not found in '{}'!".format(icon))

# Check that everything is OK
        assert len(self.emptyFrame.GetChildren()) == 0, \
        "There are parentless objects after API initialization.\n{}".format(\
                            self.emptyFrame.GetChildren())

        self.syncModuleCheckboxes()

# Initialize upload targets
        self.uploadTargets = ([], self.tr('the default device'))

# Load last used tabs
        self.tabManager.loadRememberedTabs()
        for x in argv:
            self.tabManager.addPage(x)
        self.frame.auiManager.Update()

        self.loaded = True
        self.frame.checkToggleState()
# Populate motelist
        self.populateMotelist()

    def getPlatformsFromMakefile(self):
        makefile = os.path.join(self.path, "../../mos/make/Makefile.options")
        if os.path.exists(makefile) and os.path.isfile(makefile):
            f = open(makefile, "r")
            for line in f.readlines():
                if line.startswith("PLATFORMS"):
                    line = line.split("?=")[1].strip()
                    return line.split(" ")
        return [
            "No platforms found! Check MansOS installation."
            ]

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
        assert type(parameters) is dict, "Dict expected."
        for x in parameters:
            if x.lower() == name.lower():
                return (x, parameters[x])
        return (None, None)

    def getDefaultConditions(self):
        return self.sealSyntax.predefinedConditions

    def getPlatforms(self):
        retVal = list()
        for x in self.platforms:
            if x not in self.excludedPlatforms:
                retVal.append(x)
        return retVal

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

    def killAllThreads(self):
        for x in self.activeThreads:
            if self.activeThreads[x]:
                self.activeThreads[x].process.terminate()

    def populateMotelist(self, event = None):
        self.printInfo("Populating motelist ... ", False, not self.supressTabSwitching)
        self.motelistClass.getMotelist()

    def doCompile(self, event = None):
        self.printInfo("Starting to compile ... \n", False)
        self.compiler.doCompile()

    def doUpload(self, event = None):
        # Stop all listening
        for x in self.listenModules:
            x.doClear("")
        self.printInfo("Starting to upload ... \n", False)
        self.uploader.doUpload()

    def startThread(self, thread):
        thread.EVT_ID = wx.NewId()
        thread.notifyWindow = self.frame
        self.frame.Connect(-1, -1, thread.EVT_ID, self.onResult)
        self.activeThreads[thread.EVT_ID] = thread
        thread.run()

    def stopThread(self, name):
        for x in self.activeThreads:
            if self.activeThreads[x].name == name:
                self.activeThreads[x].stop = True
                return

    def onResult(self, event):
        thread = self.activeThreads[event.GetEventType()]
        if event.data is None:
            # Call callback
            if thread.callbackFunction:
                thread.callbackFunction(thread.output)
            # Clear info about this thread
            self.activeThreads.pop(thread.EVT_ID)
        elif type(event.data) is str:
            if thread.printFunction:
                thread.printFunction(event.data)
            else:
                if thread.printToInfo:
                    self.infoArea.printLine(event.data)
                if thread.printToListen:
                    self.outputArea.printLine(event.data)
            thread.output += event.data
        elif type(event.data) is int:
            if event.data == 0 or event.data == 2: # motelist returns 2 if no mote found!
                # If no callback defined, no Done printed!
                if thread.callbackFunction:
                    self.printInfo("Done!\n", False, not self.supressTabSwitching)
            else:
                self.printInfo("Failed!\n", False, not self.supressTabSwitching)
            self.supressTabSwitching = False
        else:
            self.infoArea.printLine("Wrong format recieved {}\n".format(type(event.data)))

    def motelistChangeCallback(self):
        for x in self.motelistCallbacks:
            x()

    def changePlatform(self, event):
        if event is not None:
            platform = event.GetEventObject().GetValue()
        else:
            platform = "telosb"
        if platform in self.platforms:
            self.activePlatform = self.platforms.index(platform)
        else:
            self.activePlatform = self.platforms.index("telosb")
        self.printInfo(self.tr("Changed platform to") + " " + self.getActivePlatform() + "\n")

    def getActivePlatform(self):
        if self.platformOnly == None:
            if self.platforms[self.activePlatform] not in self.excludedPlatforms:
                return self.platforms[self.activePlatform]
            else:
                return self.platforms[0]
        else:
            return self.platforms[self.platforms.index(self.platformOnly)]

    def dummyPrint(self, msg, arg1 = "", arg2 = ""):
        print msg

    def addListenWindow(self, event):
        listenModule = ListenModule(self.emptyFrame, self)
        self.listenModules.append(listenModule)
        self.frame.layoutListenPane(listenModule, "Listen module {}".format(len(self.listenModules)))
        self.frame.auiManager.Update()

    def showBlocklyWindow(self, event):
        blocklyPane = self.frame.auiManager.GetPaneByName("blocklyPane")
        if blocklyPane.IsShown() and blocklyPane.IsOk():
            self.blocklyPane = blocklyPane
            blocklyPane.Float()
            blocklyPane.Hide()
            self.frame.auiManager.DetachPane(self.blockly)
        else:
            self.frame.layoutBlocklyPane()
        self.frame.auiManager.UpdateNotebook()
        self.frame.auiManager.Update()

    def showEditWindow(self, event):
        editPane = self.frame.auiManager.GetPaneByName("editPane")
        if editPane.IsShown() and editPane.IsOk():
            self.frame.auiManager.ClosePane(editPane)
            self.frame.auiManager.DetachPane(self.editPanel)
        else:
            self.frame.layoutEditPane()
        self.frame.auiManager.Update()

    def syncModuleCheckboxes(self):
        if self.frame.auiManager.GetPaneByName("editPane").IsShown():
            self.frame.editCheck.Check(True)
        else:
            self.frame.editCheck.Check(False)

        if self.frame.auiManager.GetPaneByName("blocklyPane").IsShown():
            self.frame.blocklyCheck.Check(True)
        else:
            self.frame.blocklyCheck.Check(False)
