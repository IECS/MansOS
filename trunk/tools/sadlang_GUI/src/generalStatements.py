import wx
import wx.lib.scrolledpanel as scrolled

class MainPanel(scrolled.ScrolledPanel):
    def __init__(self, parent):
        scrolled.ScrolledPanel.__init__(self, parent)
        
        # Global sizer for current tab
        self.main = wx.BoxSizer(wx.VERTICAL)
        
        self.use = wx.GridBagSizer(hgap=2, vgap=1)
        self.read = wx.GridBagSizer(hgap=2, vgap=1)
        self.sink = wx.GridBagSizer(hgap=2, vgap=1)
        self.use.SetCols(2)
        self.use.AddGrowableCol(1)
        self.read.SetCols(2)
        self.read.AddGrowableCol(1)
        self.sink.SetCols(2)
        self.sink.AddGrowableCol(1)
        self.main.Add(self.use, 0, flag = wx.EXPAND | wx.RIGHT );
        self.main.Add(self.read, 0, flag = wx.EXPAND | wx.RIGHT )
        self.main.Add(self.sink, 0, flag = wx.EXPAND | wx.RIGHT )
        
        
    
    def redrawAll(self):
        #Layout sizers
        self.use.Layout()
        self.read.Layout()
        self.sink.Layout()
        self.SetSizer(self.main)
        self.SetAutoLayout(1)
        self.main.Fit(self)
        self.SetupScrolling()
        self.Show()
                
    def addUse(self, keyWord, btnCallback):
        button = wx.Button(self, label = "Edit " + keyWord, size=(150, -1), name = keyWord)
        self.Bind(wx.EVT_BUTTON, btnCallback, button)
        inputField = wx.TextCtrl(self, size=(400,-1))
        inputField.AppendText("")
        self.use.SetRows(self.use.GetRows() + 1)
        self.use.Add(button, (self.use.GetRows(), 0 ))
        self.use.Add(inputField, pos = (self.use.GetRows(), 1),
                      flag = wx.EXPAND | wx.RIGHT );
        return inputField
            
    def addRead(self, keyWord, btnCallback):
        button = wx.Button(self, label = "Edit " + keyWord, size=(150, -1), name = keyWord)
        self.Bind(wx.EVT_BUTTON, btnCallback, button)
        inputField = wx.TextCtrl(self, size=(400,-1))
        inputField.AppendText("")
        self.read.SetRows(self.read.GetRows() + 1)
        self.read.Add(button, (self.read.GetRows(), 0 ))
        self.read.Add(inputField, (self.read.GetRows(), 1),
                      flag = wx.EXPAND | wx.RIGHT ) 
        return inputField
                    
    def addSink(self, keyWord, btnCallback):
        button = wx.Button(self, label = "Edit " + keyWord, size=(150, -1), name = keyWord)
        self.Bind(wx.EVT_BUTTON, btnCallback, button)
        inputField = wx.TextCtrl(self, size=(400,-1))
        inputField.AppendText("")
        self.sink.SetRows(self.sink.GetRows() + 1)
        self.sink.Add(button, (self.sink.GetRows(), 0 ))
        self.sink.Add(inputField, (self.sink.GetRows(), 1),
                      flag = wx.EXPAND | wx.RIGHT ) 
        return inputField
