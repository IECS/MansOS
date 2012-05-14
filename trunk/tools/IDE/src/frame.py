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

        # Just a shorter name
        self.tr = self.API.tr
        self.toolbar = None
        self.menubar = None
        self.examples = dict()
        self.initUI()
        self.SetBackgroundColour("white")

        self.split1 = wx.SplitterWindow(self, style = wx.SP_LIVE_UPDATE)

        self.API.editorSplitter.Reparent(self.split1)
        self.API.outputTools.Reparent(self.split1)

        self.split1.SplitHorizontally(self.API.editorSplitter,
                                      self.API.outputTools, -100)
        self.split1.SetMinimumPaneSize(100)

        self.API.tabManager.Reparent(self.API.editorSplitter)
        self.API.editPanel.Reparent(self.API.editorSplitter)
        self.tabManager = self.API.tabManager

        # Must show&hide to init
        self.API.editorSplitter.SplitVertically(self.API.tabManager,
                                                    self.API.editPanel, -305)
        self.API.editorSplitter.Unsplit()

        # Makes outputArea to maintain it's height when whole window is resized
        self.split1.SetSashGravity(1)

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
        output = fileMenu.Append(wx.ID_ANY, '&' + self.tr('Read output') + '\tCtrl+R',
                              self.tr('Open read output window'))
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
                                wx.Bitmap(self.path + '/src/Icons/new.png'))
        self.toolbar.AddSeparator()
        self.toolbar.AddLabelTool(wx.ID_OPEN, self.tr('Open'),
                                wx.Bitmap(self.path + '/src/Icons/open.png'))
        self.toolbar.AddLabelTool(wx.ID_SAVE, self.tr('Save'),
                                wx.Bitmap(self.path + '/src/Icons/save.png'))
        self.toolbar.AddSeparator()
        addStatementTool = self.toolbar.AddLabelTool(wx.ID_ADD, self.tr('Add statement'),
                                wx.Bitmap(self.path + '/src/Icons/add_statement.png'))
        # Used ID_APPLY for identification, hope nothing else uses it
        addConditionTool = self.toolbar.AddLabelTool(wx.ID_APPLY, self.tr('Add condition'),
                                wx.Bitmap(self.path + '/src/Icons/add_condition.png'))
        self.toolbar.AddSeparator()
        uplTool = self.toolbar.AddLabelTool(wx.ID_ANY, self.tr('Upload'),
                                wx.Bitmap(self.path + '/src/Icons/upload.png'))
        outputTool = self.toolbar.AddLabelTool(wx.ID_ANY, self.tr('Read output'),
                                wx.Bitmap(self.path + '/src/Icons/read.png'))
        self.toolbar.AddSeparator()
        self.toolbar.AddLabelTool(wx.ID_EXIT, self.tr('Exit'),
                                wx.Bitmap(self.path + '/src/Icons/exit.png'))
        self.toolbar.Realize()

        # Second bind to toolbar, but only items with ID_ANY, because all
        # defined ID already binded from menu, weird side effect.
        self.Bind(wx.EVT_TOOL, self.OnUpload, uplTool)
        self.Bind(wx.EVT_TOOL, self.OnOutput, outputTool)
        self.Bind(wx.EVT_TOOL, self.OnAddStatement, addStatementTool)
        self.Bind(wx.EVT_TOOL, self.OnAddCondition, addConditionTool)

    def OnQuit(self, event):
        if self.tabManager.onQuitCheck() == True:
            self.API.performExit()
            exit()

    def OnSave(self, event):
        self.tabManager.doPopupSave(None)

    def OnSaveAs(self, event):
        self.tabManager.doPopupSaveAs(None)

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
