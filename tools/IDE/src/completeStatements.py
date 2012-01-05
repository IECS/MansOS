import wx
import wx.lib.scrolledpanel as scrolled
import Editor

class MainPanel(scrolled.ScrolledPanel):
    def __init__(self, parent, API):
        scrolled.ScrolledPanel.__init__(self, parent)
        
        # Global sizer for current tab
        
        self.API = API;
        self.main = wx.BoxSizer(wx.HORIZONTAL)

        self.code = Editor.CodeEditor(self, self.API)
        self.main.Add(self.code, 
                      1,            # make vertically stretchable
                      wx.EXPAND |    # make horizontally stretchable
                      wx.ALL,        #   and make border all around
                      10 );         # set border width to 10)
        

        self.redrawAll()
        
    def redrawAll(self):
        #Layout sizers
        self.SetSizer(self.main)
        self.SetAutoLayout(1)
        #self.actionButtons.L
        self.main.Fit(self)
        self.SetupScrolling()
        self.Show()
        
    def redraw(self):
        self.code.AddText(self.API.seal.generateAllCode())
        self.code.setLineNumbers()
        self.redrawAll()
        