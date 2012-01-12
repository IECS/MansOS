#!/usr/bin/env python
# -*- coding: UTF-8 -*-
import wx
import os
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
        self.tabManager = tabManager.tabManager(self, API)
        self.API = API
        self.API.path = self.path
        # Just a shorter name
        self.tr = self.API.translater.translate
        self.toolbar = None
        self.menubar = None
        self.InitUI()
        
    def InitUI(self):
        fileMenu = wx.Menu()        
        new = fileMenu.Append(wx.ID_NEW, '&' + self.tr('New') + '\tCtrl+N', 
                              self.tr('Create empty document'))
        save = fileMenu.Append(wx.ID_SAVE, '&' + self.tr('Save') + '\tCtrl+N', 
                              self.tr('Save document'))
        open = fileMenu.Append(wx.ID_OPEN, '&' + self.tr('Open') + '\tCtrl+N', 
                              self.tr('Open document'))
        upload = fileMenu.Append(wx.ID_ANY, '&' + self.tr('Upload') + '\tCtrl+N', 
                              self.tr('Open upload window'))
        output = fileMenu.Append(wx.ID_ANY, '&' + self.tr('Read output') + '\tCtrl+N', 
                              self.tr('Open read output window'))
        close = fileMenu.Append(wx.ID_EXIT, '&' + self.tr('Exit') + '\tCtrl+N', 
                              self.tr('Exit application'))
        
        optionMenu = wx.Menu()
        language = wx.Menu()
        self.langs = []
        for i in self.API.translater.translations.keys():
            self.langs.append(language.Append(wx.ID_ANY, 
                    self.API.translater.translations[i]['langName'], i, kind = wx.ITEM_RADIO))
            if i == self.API.translater.activeLanguage:
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
        self.Bind(wx.EVT_MENU, self.OnSave, save)
        self.Bind(wx.EVT_MENU, self.OnOpen, open)
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
        uplTool = self.toolbar.AddLabelTool(wx.ID_ANY, self.tr('Upload'),
                                wx.Bitmap(self.path + '/src/Icons/upload.png'))
        outputTool = self.toolbar.AddLabelTool(wx.ID_ANY, self.tr('Read output'),
                                wx.Bitmap(self.path + '/src/Icons/read.png'))
        self.toolbar.AddSeparator()
        extTool = self.toolbar.AddLabelTool(wx.ID_EXIT, self.tr('Quit'),
                                wx.Bitmap(self.path + '/src/Icons/exit.png'))
        self.toolbar.Realize()

        # Second bind to toolbar, but only items with ID_ANY, because all
        # defined ID already binded from menu, weird side effect.
        self.Bind(wx.EVT_TOOL, self.OnUpload, uplTool)
        self.Bind(wx.EVT_TOOL, self.OnOutput, outputTool)
        
    def OnQuit(self, e):
        if self.tabManager.onQuitCheck() == True:
            self.Close()
         
    def OnSave(self, e):
        self.tabManager.doPopupSave(None)
        
    def OnUpload(self, e):
        dialog = uploadModule.UploadModule(None, 
                                self.tr('Upload and compile'), self.API)
        dialog.ShowModal()
        dialog.Destroy()

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
    
    def changeLanguage(self, event):
        for i in self.langs:
            if i.IsChecked() == True:
                self.API.translater.activeLanguage = i.GetHelp()
                self.InitUI()

def main():
    ex = wx.App()
    frame = Example(None, title="SEAL IDE", 
                    size = (800,500), pos=(100,100),
                    API = APIcore.ApiCore())
    frame.Show()
    
    ex.MainLoop()

if __name__ == '__main__':
    main()
