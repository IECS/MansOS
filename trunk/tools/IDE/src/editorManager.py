import wx
import wx.lib.scrolledpanel as scrolled
import Editor
import os
import globals

class EditorManager(scrolled.ScrolledPanel):
    def __init__(self, parent, API):
        scrolled.ScrolledPanel.__init__(self, parent)
        
        self.API = API;
        # Just a shorter name
        self.tr = self.API.translater.translate
        self.initUI()
        ### Editor visible variables
        # @ creation we assume document is saved.
        self.saveState = True
        # Filename or untitled document
        self.fileName = self.tr('Untitled document') + ' ' + str(self.GetParent().nextPageNr)
        # Filename and full path(relative or absolute)
        self.filePath = self.tr('Untitled document') + ' ' + str(self.GetParent().nextPageNr)
        # This marks if document already have a file attached to it
        self.hasAFile = False
        # Define project type
        self.projectType = 0
        
    def update (self, initFilePath = ''):
        if initFilePath == '':
            initFilePath = self.filePath
        if os.path.exists(initFilePath) & os.path.isfile(initFilePath):
            # Load file into editor
            self.code.LoadFile(initFilePath)
            # Draw margins
            self.changeCode('', False)
            # Update editor info
            self.updateInfo(initFilePath, saveState = True, hasAFile = True)
            self.GetParent().titleChange(self.fileName)
            self.detectSEAL()
        else:
            self.changeCode()
            
    def initUI(self):
        self.main = wx.BoxSizer(wx.HORIZONTAL)

        self.code = Editor.CodeEditor(self, self.API)
        self.main.Add(self.code, 
                      1,            # make vertically stretchable
                      wx.EXPAND |    # make horizontally stretchable
                      wx.ALL,        # and make border all around
                      10 );         # set border width to 10)
                      
    def redrawAll(self):
        #Layout sizers
        self.SetSizer(self.main)
        self.SetAutoLayout(1)
        #self.actionButtons.L
        self.main.Fit(self)
        self.SetupScrolling()
        self.Show()
        
    def changeCode(self, newCode = '', overwrite = True):
        if overwrite == True:
            self.code.AddText(newCode)
        self.code.setLineNumbers()
        self.redrawAll()
    
    def save(self):
        self.code.SaveFile(self.filePath)
        self.saveState = True
        self.GetParent().markAsSaved()
    
    def updateInfo(self, path = '', fileName = '', saveState = '', 
                   hasAFile = '', projectType = None):
        if path != '':
            self.filePath = path
            self.fileName = path.split("/")[-1]
        if fileName != '':
            self.fileName = fileName
        if saveState != '':
            self.saveState = saveState
        if hasAFile != '':
            self.hasAFile = hasAFile
        if projectType != None:
            self.projectType = projectType
    
    def detectSEAL(self):
        if self.fileName[-2:] == "sl":
            self.projectType = globals.SEAL_PROJECT
        else:
            self.projectType = globals.MANSOS_PROJECT