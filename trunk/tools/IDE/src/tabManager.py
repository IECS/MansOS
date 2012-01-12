import wx
import emptyTab
import editorManager

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
        self.getPageObject().update('sampleCode')
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
        
    def showPopupMenu(self, event):
        # Make clicked tab active, so all actions target this tab.
        self.SetSelection(self.HitTest(event.GetPositionTuple())[0])

        menu = wx.Menu()
        self.popupRename = menu.Append(wx.ID_REPLACE, '&Rename\tCtrl+R', "Rename")
        self.popupSave = menu.Append(wx.ID_SAVE,'&Save\tCtrl+S', "Save")
        self.popupSaveAs = menu.Append(wx.ID_SAVEAS, '&Save As\tCtrl+A', "Save As")
        self.popupClose = menu.Append(wx.ID_CLOSE, '&Close\tCtrl+W',"Close")
        self.Bind(wx.EVT_MENU, self.doPopupRename, self.popupRename)
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
        
    def doPopupRename(self, event):
        print "rename"
    
    def doPopupSave(self, event):
        if self.getPageObject().hasAFile == True:
            self.getPageObject().save()
        else:
            save = wx.FileDialog(self, 
                self.tr("Save") + " " + 
                    str(self.GetPageText(self.GetSelection())),
                wildcard = 'Seal ' + self.tr('files') + ' (*.sl)|*.sl|' + 
                    self.tr('All files') + '|*',
                style = wx.FD_SAVE | wx.FD_OVERWRITE_PROMPT)
            if save.ShowModal() == wx.ID_OK:
                self.getPageObject().code.SaveFile(save.GetPath())
                self.getPageObject().saveState = True
                self.markAsSaved()
            save.Destroy()
            
    def doPopupSaveAs(self, event):
        save = wx.FileDialog(self, 
            "Save as " + str(self.GetPageText(self.GetSelection())),
            wildcard = 'Seal ' + self.tr('files') + ' (*.sl)|*.sl|' + 
                    self.tr('All files') + '|*',
            style = wx.FD_SAVE | wx.FD_OVERWRITE_PROMPT)
        if save.ShowModal() == wx.ID_OK:
            self.getPageObject().code.SaveFile(save.GetPath())
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
