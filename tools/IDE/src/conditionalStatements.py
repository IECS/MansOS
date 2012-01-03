import wx
import wx.lib.scrolledpanel as scrolled

class MainPanel(scrolled.ScrolledPanel):
    def __init__(self, parent):
        scrolled.ScrolledPanel.__init__(self, parent)

        # Global sizer for current tab
        self.main = wx.BoxSizer(wx.VERTICAL)
        self.main.AddSpacer((0,20))
        self.whenSizer = wx.BoxSizer(wx.VERTICAL)
        self.elseSizer = wx.BoxSizer(wx.VERTICAL)
        self.elseSizer.Add(wx.StaticText(self, label = "Else:"))
        self.main.Add(self.whenSizer, 0, flag = wx.EXPAND | wx.RIGHT )
        self.main.Add(self.elseSizer, 0, flag = wx.EXPAND | wx.RIGHT )
        self.redrawAll()

    def redrawAll(self):
        #Layout sizers
        self.elseSizer.Layout()
        self.whenSizer.Layout()
        self.SetSizer(self.main)
        self.SetAutoLayout(1)
        self.main.Fit(self)
        self.SetupScrolling()
        self.Show()

    def addStatementField(self, choices, callback):
        sizer = wx.BoxSizer(wx.HORIZONTAL)
        box = wx.ComboBox(self, choices = choices, style = wx.CB_DROPDOWN)
        self.Bind(wx.EVT_TEXT_ENTER, callback, box)
        self.Bind(wx.EVT_COMBOBOX, callback, box)
        sizer.Add(wx.StaticText(self, label = "When "))
        sizer.Add(box)
        sizer.Add(wx.StaticText(self, label = ":"))
        self.whenSizer.Add(sizer)
        self.redrawAll()
        return box

    def addWhenAction(self, boxClbk, fieldClbk, buttonClbk, choices):
        box = wx.ComboBox(self, choices = choices, style = wx.CB_DROPDOWN, 
                          name = "when")
        field = wx.TextCtrl(self, size = (400, -1))
        button = wx.Button(self, label = "Edit", size=(50, -1))
        self.Bind(wx.EVT_BUTTON, buttonClbk, button)
        self.Bind(wx.EVT_TEXT_ENTER, boxClbk, box)
        self.Bind(wx.EVT_TEXT, boxClbk, box)
        self.Bind(wx.EVT_COMBOBOX, boxClbk, box)
        self.Bind(wx.EVT_TEXT_ENTER, fieldClbk, field) #NOT WORKING
        sizer = wx.BoxSizer(wx.HORIZONTAL)
        sizer.AddSpacer((60, 0))
        sizer.Add(box, 0)
        sizer.Add(button, 0)
        sizer.Add(field, 1)
        self.whenSizer.Add(sizer, 0, flag = wx.EXPAND | wx.RIGHT )
        self.redrawAll()
        return [box, button, field] 

    def addElseAction(self, boxClbk, fieldClbk, buttonClbk, choices):
        box = wx.ComboBox(self, choices = choices, style = wx.CB_DROPDOWN, 
                          name = "else")
        field = wx.TextCtrl(self, size = (400, -1))
        button = wx.Button(self, label = "Edit", size=(50, -1))
        self.Bind(wx.EVT_BUTTON, buttonClbk, button)
        self.Bind(wx.EVT_TEXT_ENTER, boxClbk, box)
        self.Bind(wx.EVT_TEXT, boxClbk, box)
        self.Bind(wx.EVT_COMBOBOX, boxClbk, box)
        self.Bind(wx.EVT_TEXT_ENTER, fieldClbk, field) #NOT WORKING
        sizer = wx.BoxSizer(wx.HORIZONTAL)
        sizer.AddSpacer((60, 0))
        sizer.Add(box, 0)
        sizer.Add(button, 0)
        sizer.Add(field, 1)
        self.elseSizer.Add(sizer, 0, flag = wx.EXPAND | wx.RIGHT )
        self.redrawAll()
        return [box, button, field]
