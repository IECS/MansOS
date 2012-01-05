import wx
import emptyTab
import completeStatements

class tabManager(wx.Notebook):
    def __init__(self, parent, API):
        wx.Notebook.__init__(self, parent)
        self.API = API
        self.API.addEditor(completeStatements.MainPanel(self, self.API))
        with open('sampleCode', 'r') as f:
            read_data = f.read()
            API.seal.parseCode(read_data)
        self.API.getEditor(-1).redraw()
        self.empty = emptyTab.EmptyTab(self)
        self.AddPage(self.API.getEditor(-1), "Unnamed document 1")
        #self.AddPage(ex, "General statements", True)
        self.AddPage(self.empty, "+")
        #generalModule.MainModule(ex, self.API)
        self.Bind(wx.EVT_NOTEBOOK_PAGE_CHANGED, self.OnPageChanged)
        
    def OnPageChanged(self, event):
        sel = self.GetSelection()
        if self.GetPageCount() - 1 == sel:
            self.RemovePage(sel)
            self.API.addEditor(completeStatements.MainPanel(self, self.API))
            self.AddPage(self.API.getEditor(-1), "Untitled Document " + str(self.GetPageCount() + 1))
            self.AddPage(self.empty, "+")
            self.SetSelection(self.GetPageCount() -2)
            self.Layout()
        self.API.setActiveEditorNr(self.GetSelection())
        event.Skip()

    def titleChange(self, newName):
        self.SetPageText(self.GetSelection(), newName)
    
    def getActiveEditorText(self):
        return self.API.getActiveEditor.GetText()
