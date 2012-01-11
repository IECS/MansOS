import wx
import wx.lib.scrolledpanel as scrolled
import Editor
import APIcore
import os

class EditorManager(scrolled.ScrolledPanel):
    def __init__(self, parent):
        scrolled.ScrolledPanel.__init__(self, parent)
        
        self.API = APIcore.ApiCore();
        self.initUI()
        ### Editor visible variables
        # @ creation we assume document is saved.
        self.saveState = True
        # Filename or untitled document
        self.fileName = 'Untitled Document ' + str(self.GetParent().nextPageNr)
        # Filename and full path(relative or absolute)
        self.filePath = 'Untitled Document ' + str(self.GetParent().nextPageNr)
        # This marks if document already have a file attached to it
        self.hasAFile = False
        
    def update (self, initFilePath = ''):
        if os.path.exists(initFilePath) & os.path.isfile(initFilePath):
            self.code.LoadFile(initFilePath)
            self.changeCode('', False)
            self.fileName = initFilePath.split('/')[-1]
            self.GetParent().titleChange(self.fileName)
            self.hasAFile = True
            self.filePath = initFilePath
            self.saveState = True
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
        print self.code.GetText()