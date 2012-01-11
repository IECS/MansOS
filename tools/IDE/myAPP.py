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
    
    def __init__(self,  *args, **kwargs):
        super(Example, self).__init__(*args, **kwargs)
        # Get path, here must use only file name, __file__ sometimes contains more than that
        self.path = os.path.dirname(os.path.realpath(__file__.split("/")[-1]))
        self.tabManager = None
        
    def addAPI(self, API):
        self.API = API
        self.API.path = self.path
        self.InitUI()
        
    def InitUI(self):
        menubar = wx.MenuBar()
        fileMenu = wx.Menu()
        new = fileMenu.Append(wx.ID_NEW, '&New\tCtrl+N', 'Create empty document')
        save = fileMenu.Append(wx.ID_SAVE, '&Save\tCtrl+S', 'Save generated code')
        open = fileMenu.Append(wx.ID_OPEN, '&Open\tCtrl+O', 'Open document')
        upload = fileMenu.Append(wx.ID_ANY, '&Upload\tCtrl+U', 'Upload generated code')
        output = fileMenu.Append(wx.ID_ANY, '&Output\tCtrl+R', 'Listen to mote\'s output')
        close = fileMenu.Append(wx.ID_EXIT, '&Quit\tCtrl+Q', 'Quit application')
        menubar.Append(fileMenu, '&File')
        self.SetMenuBar(menubar)
        self.toolbar = self.CreateToolBar()
        
        # Note that all icons must be 32x32, so they look good :)
        newTool = self.toolbar.AddLabelTool(wx.ID_NEW, 'New', wx.Bitmap(self.path + '/src/Icons/new.png'))
        self.toolbar.AddSeparator()
        self.toolbar.SetToolBitmapSize((32,32))
        openTool = self.toolbar.AddLabelTool(wx.ID_OPEN, 'Open', wx.Bitmap(self.path + '/src/Icons/open.png'))
        saveTool = self.toolbar.AddLabelTool(wx.ID_SAVE, 'Save', wx.Bitmap(self.path + '/src/Icons/save.png'))
        self.toolbar.AddSeparator()
        uplTool = self.toolbar.AddLabelTool(wx.ID_ANY, 'Upload', wx.Bitmap(self.path + '/src/Icons/upload.png'))
        outputTool = self.toolbar.AddLabelTool(wx.ID_ANY, 'Output', wx.Bitmap(self.path + '/src/Icons/read.png'))
        self.toolbar.AddSeparator()
        extTool = self.toolbar.AddLabelTool(wx.ID_EXIT, 'Quit', wx.Bitmap(self.path + '/src/Icons/exit.png'))
        self.toolbar.Realize()
        
        # First bind to menu
        self.Bind(wx.EVT_MENU, self.OnQuit, close)
        self.Bind(wx.EVT_MENU, self.OnSave, save)
        self.Bind(wx.EVT_MENU, self.OnOpen, open)
        self.Bind(wx.EVT_MENU, self.OnUpload, upload)
        self.Bind(wx.EVT_MENU, self.OnOutput, output)
        self.Bind(wx.EVT_MENU, self.OnNew, new)

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
        dialog = uploadModule.UploadModule(None, 'Upload and compile', self.API)
        dialog.ShowModal()
        dialog.Destroy()

    def OnOutput(self, e):
        dialog = listenModule.ListenModule(None, 'Listen to output', self.API)
        dialog.ShowModal()
        dialog.Destroy()
    
    def OnNew(self, e):
        self.tabManager.addPage()
    
    def OnOpen(self, e):
        open = wx.FileDialog(self, 
            "Open new document",
            wildcard = 'Seal files (*.sl)|*.sl|All files|*',
            style = wx.FD_OPEN)
        if open.ShowModal() == wx.ID_OK:
            self.tabManager.addPage(open.GetPath())
        open.Destroy()
    
    def setTabManager(self, tabManager):
        self.tabManager = tabManager

def main():
    # TODO: This is not right, API is used here for one reason, getting path and
    # platforms and each editor have separate API copy for accessing other 
    # structures and sealStruct. Should separate this, maybe...
    API = APIcore.ApiCore()
    ex = wx.App()
    frame = Example(None, title="SEAL IDE", size = (800,500), pos=(100,100))
    frame.addAPI(API)
    frame.setTabManager(tabManager.tabManager(frame))
    frame.Show()
    
    ex.MainLoop()

if __name__ == '__main__':
    main()
