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
import emptyTab
import editorManager
import globals as g

class tabManager(wx.Notebook):
    def __init__(self, parent, API):
        wx.Notebook.__init__(self, parent)
        self.empty = emptyTab.EmptyTab(self)
        self.API = API
        # Just a shorter name
        self.tr = self.API.translater.translate
        
        # Need to set because next statement uses it
        self.nextPageNr = 1
        self.AddPage(editorManager.EditorManager(self, self.API), 
                self.tr("Untitled document") + ' ' + str(self.nextPageNr))
        #self.AddPage(ex, "General statements", True)
        self.AddPage(self.empty, "+")
        self.getPageObject().update('sampleCode.sl')
        self.Bind(wx.EVT_NOTEBOOK_PAGE_CHANGED, self.onPageChanged)
        self.Bind(wx.EVT_RIGHT_DOWN, self.showPopupMenu)
        self.nextPageNr += 1
        
    def onPageChanged(self, event):
        sel = self.GetSelection()
        if self.GetPageCount() - 1 == sel:
            self.AddPage(editorManager.EditorManager(self, self.API), 
                self.tr("Untitled document") + ' ' + str(self.nextPageNr))
            self.nextPageNr += 1
            self.RemovePage(sel)
            self.AddPage(self.empty, "+")
            self.ChangeSelection(self.GetPageCount() - 2)
            self.getPageObject().update('')
            self.Layout()
        if event != None:
            event.Skip()
            if self.getPageObject().projectType == g.SEAL_PROJECT:
                self.GetParent().enableAdders()
            else:
                self.GetParent().disableAdders()

        
    def showPopupMenu(self, event):
        # Make clicked tab active, so all actions target this tab.
        self.SetSelection(self.HitTest(event.GetPositionTuple())[0])

        menu = wx.Menu()
        self.popupReload = menu.Append(wx.ID_REPLACE, '&' + self.tr("Reload") + 
                                       '\tCtrl+R', self.tr("Reload"))
        self.popupSave = menu.Append(wx.ID_SAVE,'&' + self.tr('Save') + 
                                     '\tCtrl+S', self.tr("Save"))
        self.popupSaveAs = menu.Append(wx.ID_SAVEAS, '&' + self.tr("Save as") + 
                                       '\tCtrl+A', self.tr("Save as"))
        self.popupClose = menu.Append(wx.ID_CLOSE, '&' + self.tr('Close') + 
                                      '\tCtrl+W',self.tr("Close"))
        self.Bind(wx.EVT_MENU, self.doPopupReload, self.popupReload)
        self.Bind(wx.EVT_MENU, self.doPopupSave, self.popupSave)
        self.Bind(wx.EVT_MENU, self.doPopupSaveAs, self.popupSaveAs)
        self.Bind(wx.EVT_MENU, self.doPopupClose, self.popupClose)
        #Disable control if needed
        if self.getPageObject().saveState == True:
            self.popupSave.Enable(False)

        # Popup the menu. If an item is selected then its handler
        # will be called before PopupMenu returns.
        self.PopupMenu(menu)
        menu.Destroy()
        
    def doPopupReload(self, event):
        self.getPageObject().update()
        
    def doPopupSave(self, event):
        if self.getPageObject().hasAFile == True:
            self.getPageObject().save()
        else:
            save = wx.FileDialog(self, 
                self.tr("Save") + " \""  + 
                    str(self.GetPageText(self.GetSelection())) + '"',
                wildcard = 'Seal ' + self.tr('files') + ' (*.sl)|*.sl|' + 
                    self.tr('All files') + '|*',
                style = wx.FD_SAVE | wx.FD_OVERWRITE_PROMPT,
                defaultFile = self.getPageObject().fileName)
            if save.ShowModal() == wx.ID_OK:
                self.getPageObject().updateInfo(path = save.GetPath())
                self.getPageObject().save()
            save.Destroy()
        return self.getPageObject().hasAFile == True
    
    def doPopupSaveAs(self, event):
        save = wx.FileDialog(self, 
            self.tr("Save as") + " \"" + 
            str(self.GetPageText(self.GetSelection())) +'"',
            wildcard = 'Seal ' + self.tr('files') + ' (*.sl)|*.sl|' + 
                    self.tr('All files') + '|*',
            style = wx.FD_SAVE | wx.FD_OVERWRITE_PROMPT,
            defaultFile = self.getPageObject().fileName)
        if save.ShowModal() == wx.ID_OK:
            if save.GetPath()[-3:] != '.sl':
                self.getPageObject().updateInfo(path = save.GetPath() + '.sl')
            else:
                self.getPageObject().updateInfo(path = save.GetPath())
            self.getPageObject().save()
        save.Destroy()

            
    def doPopupClose(self, event, checkConsequences = True):
        if self.onCloseCheck() == False:
            return False
        # Remove selected page.
        self.RemovePage(self.GetSelection())
        if checkConsequences == True:
            # If this is last page we should add empty page
            if self.GetPageCount() == 1:
                self.onPageChanged(None)
            elif self.GetSelection() == self.GetPageCount() - 1:
                self.ChangeSelection(self.GetSelection() - 1)
        return True
        
    def titleChange(self, newName):
        self.SetPageText(self.GetSelection(), newName)
    
    def markAsUnsaved(self):
        self.titleChange('* ' + self.getPageObject().fileName)
    
    def markAsSaved(self):
        self.titleChange(self.getPageObject().fileName)

    def getPageObject(self):
        return self.GetPage(self.GetSelection())
    
    def addPage(self, newFile = ''):
        # A bit of cheating here!
        # First we select tab marked as '+'
        self.SetSelection(self.GetPageCount()-1)
        # Second we virtually call page changing event
        self.onPageChanged(None)
        # Third we add any file associated with it
        self.getPageObject().update(newFile)
        # And magic is done! :)
    
    def onCloseCheck(self):
        if self.getPageObject().saveState == False:
            # Initiate DialogBox
            dialog = wx.MessageDialog(self, 
                self.tr('Save changes to') + ' "' + 
                    self.getPageObject().fileName + '" ' + 
                    self.tr('before close it?'),
                self.tr('Unsaved document') + ' "'+ 
                    self.getPageObject().fileName + '"', 
                wx.YES_NO | wx.CANCEL | wx.ICON_EXCLAMATION)
            
            retVal = dialog.ShowModal()
            if retVal == wx.ID_YES:
                # Create save dialog
                self.doPopupSave(None)
                # Recursion to make sure it's really saved.
                return self.onCloseCheck()
            elif retVal == wx.ID_CANCEL:
                # Stop action
                return False
        # It's ok to close
        return True
    
    def onQuitCheck(self):
        while self.GetPageCount() > 1:
            # Select first page and try to close it
            self.SetSelection(0)
            if self.doPopupClose(None, False) == False:
                return False
        return True
