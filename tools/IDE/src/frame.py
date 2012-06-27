#!/usr/bin/env python
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

import os
import wx
from stat import S_ISDIR
from wx.lib.agw import aui

from upload_module import UploadModule
from globals import * #@UnusedWildImport

class Frame(wx.Frame):
    def __init__(self, parent, title, size, pos, API):
        super(Frame, self).__init__(parent, wx.ID_ANY, title, size = size, pos = pos)

        # Get path, here must use only file name, __file__ sometimes contains more than that
        self.path = os.path.dirname(os.path.realpath(os.path.split(__file__)[1]))
        self.API = API
        self.API.path = self.path
        self.lastPanel = None
        self.auiManager = aui.AuiManager()
        self.loadPositioning()
        self.exitCalled = False
        self.blocklyToolVisible = False

        # Just a shorter name
        self.tr = self.API.tr
        self.toolbar = None
        self.menubar = None
        self.examples = dict()
        self.initUI()
        self.SetBackgroundColour("white")

        flag = aui.AUI_MGR_ALLOW_ACTIVE_PANE | aui.AUI_MGR_LIVE_RESIZE | \
        aui.AUI_MGR_AUTONB_NO_CAPTION | aui.AUI_MGR_SMOOTH_DOCKING | \
        aui.AUI_MGR_TRANSPARENT_HINT | aui.AUI_NB_CLOSE_ON_TAB_LEFT | \
        aui.AUI_MGR_AERO_DOCKING_GUIDES | aui.AUI_MGR_TRANSPARENT_DRAG
        self.auiManager.SetAGWFlags(self.auiManager.GetAGWFlags() ^ flag)
        self.auiManager.SetAutoNotebookStyle(aui.AUI_NB_TOP | aui.AUI_NB_SMART_TABS)

        #self.API.outputTools.Reparent(self)
        self.API.tabManager.Reparent(self)
        self.API.editPanel.Reparent(self)
        self.API.blockly.Reparent(self)

        self.tabManager = self.API.tabManager

        self.auiManager.SetManagedWindow(self)
        self.auiManager.AddPane(self.API.tabManager,
                aui.AuiPaneInfo().CenterPane().
                CloseButton(False).
                Caption("Editors").CaptionVisible(False).
                MinimizeButton(True).MaximizeButton(True).
                BestSize(wx.Size(400, 400))
                                )
        self.auiManager.UpdateNotebook()
        rightPane = (aui.AuiPaneInfo().Floatable(True).
                Right().
                CloseButton(False).
                Caption("Visual edit").CaptionVisible(True).
                MinimizeButton(True).MaximizeButton(True).
                BestSize(wx.Size(250, 400)))
        self.auiManager.AddPane(self.API.editPanel, rightPane)

        self.bottomPane = (aui.AuiPaneInfo().
                Bottom().CloseButton(False).
                CaptionVisible(True).Caption("Listen").
                MinimizeButton(True).MaximizeButton(True).
                BestSize(wx.Size(500, 150)))
        self.API.listenModule.Reparent(self)
        self.auiManager.AddPane(self.API.listenModule, self.bottomPane)

        self.blocklyPane = (aui.AuiPaneInfo().Caption("Seal-Blockly handler").CloseButton(False).
                CaptionVisible(True).
                MinimizeButton(True).MaximizeButton(True).
                BestSize(wx.Size(500, 150)))
        self.auiManager.AddPane(self.API.blockly, self.blocklyPane, target = self.bottomPane)

        self.API.infoArea.Reparent(self)
        infoPane = (aui.AuiPaneInfo().Caption("Info").CloseButton(False).
                CaptionVisible(True).
                MinimizeButton(True).MaximizeButton(True).
                BestSize(wx.Size(500, 150)))
        self.auiManager.AddPane(self.API.infoArea, infoPane, target = self.bottomPane)

        self.auiManager.Update()
        self.Bind(wx.EVT_CLOSE, self.OnQuit)

    def initUI(self):
        fileMenu = wx.Menu()
        new = fileMenu.Append(wx.ID_NEW, '&' + self.tr('New') + '\tCtrl+N',
                              self.tr('Create empty document'))
        open_ = fileMenu.Append(wx.ID_OPEN, '&' + self.tr('Open') + '\tCtrl+O',
                              self.tr('Open document'))
        save = fileMenu.Append(wx.ID_SAVE, '&' + self.tr('Save') + '\tCtrl+S',
                              self.tr('Save document'))
        saveAs = fileMenu.Append(wx.ID_SAVEAS, '&' + self.tr('Save as') + '\tCtrl+A',
                              self.tr('Save document as'))
        upload = fileMenu.Append(wx.ID_ANY, '&' + self.tr('Upload') + '\tCtrl+U',
                              self.tr('Open upload window'))
        close = fileMenu.Append(wx.ID_EXIT, '&' + self.tr('Exit') + '\tCtrl+Q',
                              self.tr('Exit application'))

        # show menu with mansos demo applications
        pathToMansosApps = self.API.path + os.path.normcase("/../../apps/")
        exampleMenu = wx.Menu()
        # list all directories in mansos/apps
        dirlist = os.listdir(pathToMansosApps)
        dirlist.sort()
        for dirname in dirlist:
            pathname = os.path.join(pathToMansosApps, dirname)
            mode = os.stat(pathname).st_mode
            if not S_ISDIR(mode): continue
            # it's a directory; process it
            dirMenu = wx.Menu()
            emptyMenu = True
            # list all directories in the current subdirectory
            applist = os.listdir(pathname)
            applist.sort()
            for filename in applist:
                appname = os.path.join(pathname, filename)
                mode = os.stat(appname).st_mode
                if not S_ISDIR(mode): continue
                # it's a directory; test whether it has a Makefile in it
                if not os.path.isfile(os.path.join(appname, 'Makefile')): continue
                # append the name of this example to directory menu
                ex = dirMenu.Append(wx.ID_ANY, filename, os.path.join(dirname, appname))
                # save this id/path combination, to be used in callback code
                self.examples[ex.GetId()] = appname
                self.Bind(wx.EVT_MENU, self.OnOpenExample, ex)
                emptyMenu = False
            if not emptyMenu:
                exampleMenu.AppendMenu(wx.ID_ANY, dirname, dirMenu)

        optionMenu = wx.Menu()
        language = wx.Menu()
        self.langs = []
        for i in self.API.translater.translations.keys():
            self.langs.append(language.Append(wx.ID_ANY,
                    self.API.translater.translations[i]['langName'], i, kind = wx.ITEM_RADIO))
            if i == self.API.getSetting("activeLanguage"):
                language.Check(self.langs[-1].GetId(), True)
            self.Bind(wx.EVT_MENU, self.changeLanguage, self.langs[-1])

        optionMenu.AppendMenu(wx.ID_ANY, self.tr('Change language'), language)

        output = optionMenu.Append(wx.ID_ANY, '&' + self.tr('Configure upload and compile') + '\tCtrl+R',
                              self.tr('Open read output window'))
        # Check if we need to update existing menubar(for translate)
        if self.menubar == None:
            self.menubar = wx.MenuBar()
        else:
            for i in range(0, self.menubar.GetMenuCount()):
                self.menubar.Remove(0)

        self.menubar.Append(fileMenu, '&' + self.tr('File'))
        self.menubar.Append(exampleMenu, '&' + self.tr('Examples'))
        self.menubar.Append(optionMenu, '&' + self.tr('Options'))
        self.SetMenuBar(self.menubar)

        # First bind to menu
        self.Bind(wx.EVT_MENU, self.OnQuit, close)
        self.Bind(wx.EVT_MENU, self.OnOpen, open_)
        self.Bind(wx.EVT_MENU, self.OnSave, save)
        self.Bind(wx.EVT_MENU, self.OnSaveAs, saveAs)
        self.Bind(wx.EVT_MENU, self.OnUpload, upload)
        self.Bind(wx.EVT_MENU, self.OnOutput, output)
        self.Bind(wx.EVT_MENU, self.OnNew, new)

        # Check if we need to update existing toolbar(for rerun)
        if self.toolbar == None:
            self.toolbar = self.CreateToolBar()
            self.toolbar.SetToolBitmapSize((32, 32))
        else:
            self.toolbar.ClearTools()

        # Note that all icons must be 32x32, so they look good :)
        self.toolbar.AddLabelTool(wx.ID_NEW, self.tr('New'),
                                wx.Bitmap(self.path + '/src/Icons/new.png'),
                                shortHelp = self.tr('New'))
        self.toolbar.AddSeparator()
        self.toolbar.AddLabelTool(wx.ID_OPEN, self.tr('Open'),
                                wx.Bitmap(self.path + '/src/Icons/open.png'),
                                shortHelp = self.tr('Open'))
        self.toolbar.AddLabelTool(wx.ID_SAVE, self.tr('Save'),
                                wx.Bitmap(self.path + '/src/Icons/save.png'),
                                shortHelp = self.tr('Save'))
        self.toolbar.AddSeparator()
        addStatementTool = self.toolbar.AddLabelTool(wx.ID_ADD, self.tr('Add statement'),
                                wx.Bitmap(self.path + '/src/Icons/add_statement.png'),
                                shortHelp = self.tr('Add statement'))
        # Used ID_APPLY for identification, hope nothing else uses it
        addConditionTool = self.toolbar.AddLabelTool(wx.ID_APPLY, self.tr('Add condition'),
                                wx.Bitmap(self.path + '/src/Icons/add_condition.png'),
                                shortHelp = self.tr('Add condition'))
        #self.toolbar.AddSeparator()
        #addBlocklyTool = self.toolbar.AddLabelTool(wx.ID_MORE, self.tr('Open Seal-Blockly editor'),
        #                        wx.Bitmap(self.path + '/src/Icons/Seal_blockly.png'),
        #                        shortHelp = self.tr('Open Seal-Blockly editor'))
        self.toolbar.AddSeparator()
        compileTool = self.toolbar.AddLabelTool(wx.ID_PREVIEW, self.tr('Compile'),
                                wx.Bitmap(self.path + '/src/Icons/compile.png'),
                                shortHelp = self.tr('Compile'))
        uplTool = self.toolbar.AddLabelTool(wx.ID_PREVIEW_GOTO, self.tr('Upload'),
                                wx.Bitmap(self.path + '/src/Icons/upload.png'),
                                shortHelp = self.tr('Upload'))
        outputTool = self.toolbar.AddLabelTool(wx.ID_PREVIEW_ZOOM, self.tr('Configure upload and compile'),
                                wx.Bitmap(self.path + '/src/Icons/read.png'),
                                shortHelp = self.tr('Configure upload and compile'))
        self.toolbar.AddSeparator()
        self.toolbar.AddLabelTool(wx.ID_EXIT, self.tr('Exit'),
                                wx.Bitmap(self.path + '/src/Icons/exit.png'),
                                shortHelp = self.tr('Exit'))
        self.toolbar.Realize()

        # Second bind to toolbar, but only items with ID_ANY, because all
        # defined ID already binded from menu, weird side effect.
        self.Bind(wx.EVT_TOOL, self.OnCompile, compileTool)
        self.Bind(wx.EVT_TOOL, self.OnUpload, uplTool)
        self.Bind(wx.EVT_TOOL, self.OnOutput, outputTool)
        self.Bind(wx.EVT_TOOL, self.OnAddStatement, addStatementTool)
        self.Bind(wx.EVT_TOOL, self.OnAddCondition, addConditionTool)
        #self.Bind(wx.EVT_TOOL, self.OnBlocklyTool, addBlocklyTool)

    def OnQuit(self, event):
        # Workaround, because wx.exit calls wx.ON_CLOSE, which is binded to this 
        # function, result is recursive calling of this function.
        if self.exitCalled:
            return
        self.exitCalled = True
        self.API.tabManager.rememberOpenedTabs()
        self.rememberPositioning()
        if self.tabManager.onQuitCheck() == True:
            self.API.performExit()
            wx.Exit()

    def OnSave(self, event):
        self.tabManager.doPopupSave(None)

    def OnSaveAs(self, event):
        self.tabManager.doPopupSaveAs(None)

    def OnCompile(self, event):
        if self.tabManager.doPopupSave(None) == True:
            self.API.uploadCore.manageCompile()

    def OnUpload(self, event):
        if self.tabManager.doPopupSave(None) == True:
            self.API.uploadCore.manageUpload()

    def OnOutput(self, event):
        if self.tabManager.doPopupSave(None) == True:

            dialog = wx.Dialog(self, wx.ID_ANY, self.tr('Configure upload and compile'),
                style = wx.DEFAULT_DIALOG_STYLE | wx.RESIZE_BORDER)

            UploadModule(dialog, self.API)
            dialog.Fit()
            dialog.ShowModal()
            dialog.Destroy()

    def OnNew(self, event):
        self.tabManager.addPage()

    def OnOpen(self, event):
        open_ = wx.FileDialog(self,
            self.tr("Open new document"),
            wildcard = 'Seal or MansOS ' + self.tr('files') + ' (*.sl, *.c)|*.sl;*.c|' +
                    self.tr('All files') + '|*',
            style = wx.FD_OPEN)
        if open_.ShowModal() == wx.ID_OK:
            self.tabManager.addPage(open_.GetPath())
        open_.Destroy()

    def findFirstSourceFile(self, path):
        # look for files in this order: first main.sl
        filename = os.path.join(path, "main.sl")
        if os.path.isfile(filename): return filename
        # then main.c
        filename = os.path.join(path, "main.c")
        if os.path.isfile(filename): return filename
        # then any other .sl file
        for _ in os.listdir(path):
            filename = os.path.join(path, filename)
            if filename[len(filename) - 3:] == '.sl': return filename
        # then any other .c file
        for _ in os.listdir(path):
            filename = os.path.join(path, filename)
            if filename[len(filename) - 2:] == '.c': return filename
        # then give up
        return None

    def OnOpenExample(self, event):
        path = self.examples.get(event.GetId())
        filename = self.findFirstSourceFile(path)
        if filename:
            self.tabManager.addPage(filename)
        else:
            self.API.logMsg(LOG_WARNING, "No source files in {}".format(path))

    def OnAddStatement(self, event):
        self.tabManager.getPageObject().code.addStatement()

    def OnAddCondition(self, event):
        self.tabManager.getPageObject().code.addCondition()

    def changeLanguage(self, event):
        for i in self.langs:
            if i.IsChecked() == True:
                self.API.setSetting("activeLanguage", i.GetHelp())
                self.initUI()

    def disableAdders(self):
        self.toolbar.EnableTool(wx.ID_ADD, False)
        self.toolbar.EnableTool(wx.ID_APPLY, False)

    def enableAdders(self):
        self.toolbar.EnableTool(wx.ID_ADD, True)
        self.toolbar.EnableTool(wx.ID_APPLY, True)

    def rememberPositioning(self):
        # This approach have a small bug.
        #    1. Maximize window
        #    2. Exit
        #    3. Reopen
        #    4. Re-maximize
        # Result is that window stays with maximized height and width, 
        # but is not maximized.
        # TODO: FIX: cache all sizes when resizing and not in maximized mode.
        self.API.setSetting("Width", self.GetSize()[0])
        self.API.setSetting("Height", self.GetSize()[1])
        self.API.setSetting("LocX", self.GetScreenPositionTuple()[0])
        self.API.setSetting("LocY", self.GetScreenPositionTuple()[1])
        self.API.setSetting("Maximized", self.IsMaximized())

    def loadPositioning(self):
        width = self.API.getSetting("Width")
        if width == '':
            width = 600
        else:
            width = int(width)

        height = self.API.getSetting("Height")
        if height == '':
            height = 800
        else:
            height = int(height)

        self.SetSize((width, height))

        locX = self.API.getSetting("LocX")
        if locX == '':
            locX = wx.GetDisplaySize()[0] / 2
        else:
            locX = int(locX)

        locY = self.API.getSetting("LocY")
        if locY == '':
            locY = wx.GetDisplaySize()[1] / 2
        else:
            locY = int(locY)

        self.SetPosition((locX, locY))

        maximized = self.API.getSetting("Maximized")
        if maximized == '':
            maximized = False
        else:
            maximized = bool(maximized == "True")

        self.Maximize(maximized)

    def deactivateNoEditorMode(self):
        self.toolbar.EnableTool(wx.ID_SAVE, True)
        self.toolbar.EnableTool(wx.ID_ADD, True)
        self.toolbar.EnableTool(wx.ID_APPLY, True)
        self.toolbar.EnableTool(wx.ID_PREVIEW, True)
        self.toolbar.EnableTool(wx.ID_PREVIEW_GOTO, True)
        self.toolbar.EnableTool(wx.ID_PREVIEW_ZOOM, True)

    def activateNoEditorMode(self):
        self.toolbar.EnableTool(wx.ID_SAVE, False)
        self.toolbar.EnableTool(wx.ID_ADD, False)
        self.toolbar.EnableTool(wx.ID_APPLY, False)
        self.toolbar.EnableTool(wx.ID_PREVIEW, False)
        self.toolbar.EnableTool(wx.ID_PREVIEW_GOTO, False)
        self.toolbar.EnableTool(wx.ID_PREVIEW_ZOOM, False)

    #def OnBlocklyTool(self, event):
    #    wx.YieldIfNeeded()
    #    self.auiManager.RestorePane(self.blocklyPane)
    #    self.blocklyToolVisible = not self.blocklyToolVisible
