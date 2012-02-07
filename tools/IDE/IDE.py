#!/usr/bin/env python
# -*- coding: UTF-8 -*-

import os
wxModuleOK = True
serialModuleOK = True
try:
	import wx
except ImportError:
	wxModuleOK = False
try:
	import serial
except ImportError:
	serialModuleOK = False
if not wxModuleOK or not serialModuleOK:
	if os.name == 'posix': installStr = "Make sure you have installed required modules. Run:\n\tapt-get install"
	else: installStr = "Make sure you have installed modules:"
	print "Cannot run MansOS IDE:"
	if not wxModuleOK:
		print "\twx module not found"
		installStr += " python-wxtools"
	if not serialModuleOK:
		print "\tserial module not found"
		installStr += " python-serial"
	print installStr
	exit(1)
from subprocess import Popen, PIPE, STDOUT
import sys
import shlex
from time import sleep
# Go to real directory for those imports
os.chdir(os.path.dirname(os.path.realpath(__file__)))
from src import tabManager
from src import uploadModule
from src import listenModule
from src import APIcore
from src import sealStruct

class Example(wx.Frame):
    def __init__(self, parent, title, size, pos, API):
        super(Example, self).__init__(parent, wx.ID_ANY, title, size = size, pos = pos)
        # Get path, here must use only file name, __file__ sometimes contains more than that
        self.path = os.path.dirname(os.path.realpath(__file__.split("/")[-1]))
        
        
        self.API = API
        self.API.path = self.path
        # Just a shorter name
        self.tr = self.API.translater.translate
        self.toolbar = None
        self.menubar = None
        self.InitUI()
        self.tabManager = tabManager.tabManager(self, API)
        
    def InitUI(self):
        fileMenu = wx.Menu()
        new = fileMenu.Append(wx.ID_NEW, '&' + self.tr('New') + '\tCtrl+N', 
                              self.tr('Create empty document'))
        open = fileMenu.Append(wx.ID_OPEN, '&' + self.tr('Open') + '\tCtrl+O', 
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
        
        # Check if we need to update existing menubar(for rerun)
        if self.menubar == None:
            self.menubar = wx.MenuBar()
        else:
            for i in range(0, self.menubar.GetMenuCount()):
                self.menubar.Remove(0)
            
        self.menubar.Append(fileMenu, '&' + self.tr('File'))
        self.menubar.Append(optionMenu, '&' + self.tr('Options'))
        self.SetMenuBar(self.menubar)
        
        # First bind to menu
        self.Bind(wx.EVT_MENU, self.OnQuit, close)
        self.Bind(wx.EVT_MENU, self.OnOpen, open)
        self.Bind(wx.EVT_MENU, self.OnSave, save)
        self.Bind(wx.EVT_MENU, self.OnSaveAs, saveAs)
        self.Bind(wx.EVT_MENU, self.OnUpload, upload)
        self.Bind(wx.EVT_MENU, self.OnOutput, output)
        self.Bind(wx.EVT_MENU, self.OnNew, new)
        
        # Check if we need to update existing toolbar(for rerun)
        if self.toolbar == None:
            self.toolbar = self.CreateToolBar()
            self.toolbar.SetToolBitmapSize((32,32))
        else:
            self.toolbar.ClearTools()
            
        # Note that all icons must be 32x32, so they look good :)
        newTool = self.toolbar.AddLabelTool(wx.ID_NEW, self.tr('New'),
                                wx.Bitmap(self.path + '/src/Icons/new.png'))
        self.toolbar.AddSeparator()
        openTool = self.toolbar.AddLabelTool(wx.ID_OPEN, self.tr('Open'),
                                wx.Bitmap(self.path + '/src/Icons/open.png'))
        saveTool = self.toolbar.AddLabelTool(wx.ID_SAVE, self.tr('Save'),
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
        multUplTool = self.toolbar.AddLabelTool(wx.ID_ANY, self.tr('Multiple upload'),
                                wx.Bitmap(self.path + '/src/Icons/upload_multiple.png'))
        outputTool = self.toolbar.AddLabelTool(wx.ID_ANY, self.tr('Read output'),
                                wx.Bitmap(self.path + '/src/Icons/read.png'))
        self.toolbar.AddSeparator()
        extTool = self.toolbar.AddLabelTool(wx.ID_EXIT, self.tr('Exit'),
                                wx.Bitmap(self.path + '/src/Icons/exit.png'))
        self.toolbar.Realize()

        # Second bind to toolbar, but only items with ID_ANY, because all
        # defined ID already binded from menu, weird side effect.
        self.Bind(wx.EVT_TOOL, self.OnUpload, uplTool)
        self.Bind(wx.EVT_TOOL, self.OnMultipleUpload, multUplTool)
        self.Bind(wx.EVT_TOOL, self.OnOutput, outputTool)
        self.Bind(wx.EVT_TOOL, self.OnAddStatement, addStatementTool)
        self.Bind(wx.EVT_TOOL, self.OnAddCondition, addConditionTool)
        
    def OnQuit(self, e):
        if self.tabManager.onQuitCheck() == True:
            self.API.saveSettings()
            self.Close()
         
    def OnSave(self, e):
        self.tabManager.doPopupSave(None)
        
    def OnSaveAs(self, e):
        self.tabManager.doPopupSaveAs(None)
        
    def OnUpload(self, e):
        if self.tabManager.doPopupSave(None) == True:
            dialog = uploadModule.UploadModule(self, 
                                self.tr('Upload and compile'), self.API)
            dialog.ShowModal()
            dialog.Destroy()
    def OnMultipleUpload(self, e):
        print "TODO"
        try:
            os.chdir("../shell")
            upload = Popen(["./shell"], stdin = PIPE, 
                           stderr = STDOUT, stdout = PIPE)
            out = upload.communicate(input = "ls")[0]
            print out
            #sleep(5)
            upload = Popen(["./shell"], stdin = PIPE, 
                           stderr = STDOUT, stdout = PIPE)
            out = upload.communicate(input = "sense")[0]
            print out
        except OSError, e:
            print >>sys.stderr, "execution failed:", e

        
    def OnOutput(self, e):
        dialog = listenModule.ListenModule(None, 
                                self.tr('Listen to output'), self.API)
        dialog.ShowModal()
        dialog.Destroy()
    
    def OnNew(self, e):
        self.tabManager.addPage()
    
    def OnOpen(self, e):
        open = wx.FileDialog(self, 
            self.tr("Open new document"),
            wildcard = 'Seal ' + self.tr('files') + ' (*.sl)|*.sl|' + 
                    self.tr('All files') + '|*',
            style = wx.FD_OPEN)
        if open.ShowModal() == wx.ID_OK:
            self.tabManager.addPage(open.GetPath())
        open.Destroy()
    
    def OnAddStatement(self, event):
        self.tabManager.getPageObject().code.addStatement()
    
    def OnAddCondition(self, event):
        self.tabManager.getPageObject().code.addCondition()
        
    def changeLanguage(self, event):
        for i in self.langs:
            if i.IsChecked() == True:
                self.API.setSetting("activeLanguage", i.GetHelp())
                self.InitUI()
                
    def disableAdders(self):
        self.toolbar.EnableTool(wx.ID_ADD, False)
        self.toolbar.EnableTool(wx.ID_APPLY, False)
        #print "disabled"
    
    def enableAdders(self):
        self.toolbar.EnableTool(wx.ID_ADD, True)
        self.toolbar.EnableTool(wx.ID_APPLY, True)
        #print "enabled"
    
def main():
    ex = wx.App()
    frame = Example(None, title="MansOS IDE", 
                    size = (800,500), pos=(100,100),
                    API = APIcore.ApiCore())
    frame.Show()
    
    ex.MainLoop()

if __name__ == '__main__':
    main()
