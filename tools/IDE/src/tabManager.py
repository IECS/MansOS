import wx
import generalStatements
import emptyTab
import conditionalStatements
import generalModule
import conditionalModule
import completeStatements

class tabManager(wx.Notebook):
    def __init__(self, parent, API):
        wx.Notebook.__init__(self, parent)
        self.API = API
        self.code = completeStatements.MainPanel(self, self.API)
        ex = generalStatements.MainPanel(self)
        self.empty = emptyTab.EmptyTab(self)
        self.AddPage(self.code, "Generated code")
        self.AddPage(ex, "General statements", True)
        self.AddPage(self.empty, "+")
        generalModule.MainModule(ex, self.API)
        self.Bind(wx.EVT_NOTEBOOK_PAGE_CHANGED, self.OnPageChanged)
        
    def OnPageChanged(self, event):
        sel = self.GetSelection()
        if self.GetPageCount() - 1 == sel:
            self.RemovePage(sel)
            exx = conditionalStatements.MainPanel(self)
            self.AddPage(exx, "When ...")
            self.AddPage(self.empty, "+")
            conditionalModule.ConditionalModule(exx, self.API, self.titleChange)
            #tabCount += 1
            self.SetSelection(self.GetPageCount() -2)
            self.Layout()
        elif sel == 0:
            self.code.redraw()
        event.Skip()

    def titleChange(self, newName):
        self.SetPageText(self.GetSelection(), newName)
