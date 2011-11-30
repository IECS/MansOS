import wx
import translater

class editDialog(wx.Dialog):
    
    def __init__(self, parent, title, data, saveCallback, name, values):
        super(editDialog, self).__init__(parent = parent, 
            title = title, size = (500, 400))
        self.saveCallback = saveCallback
        self.main = wx.BoxSizer(wx.VERTICAL)
        self.data = wx.GridBagSizer(hgap = 2, vgap = 1)
        self.main.Add(wx.StaticText(self, label = "Here You can edit parameters for " + name))
        self.main.AddSpacer((10, 10))
        self.main.Add(self.data, 0)
        self.main.AddSpacer((10, 10))
        self.choices = []
        self.name = name
        
        row = 0
        for key in data.keys():
            text = wx.StaticText(self, label = translater.translate(key) + ":")
            self.data.Add(text, pos = (row,0))
            if data[key] != None:
                self.choices.append(wx.ComboBox(self, choices = data[key].keys(), 
                                                style = wx.CB_DROPDOWN, name = key))
            else:
                self.choices.append(wx.CheckBox(self, name = key))
            print values
            if values != None:
                self.choices[-1].SetValue(values[key])
            self.data.Add(self.choices[-1], pos = (row, 1))
            row += 1
            
        self.buttonPane = wx.BoxSizer(wx.HORIZONTAL)
        self.save = wx.Button(self, label = "Save", size = (150, -1), name = "save")
        self.Bind(wx.EVT_BUTTON, self.saveClk, self.save)
        self.buttonPane.Add(self.save, 0)
        self.close = wx.Button(self, label = "Close", size = (150, -1), name = "close")
        self.Bind(wx.EVT_BUTTON, self.saveClk, self.close)
        self.buttonPane.Add(self.close, 0)
        self.main.Add(self.buttonPane, 0)
        
        self.SetSizer(self.main)
        self.main.Fit(self)
        self.Show()
    
    def saveClk(self, event):
        if (event.GetEventObject().GetName() == "save"):
            data = {}
            for x in self.choices:
                data[x.GetName()] =  x.GetValue()
            self.saveCallback(True, data, self.name)
        else:
            self.saveCallback(False, "", "")

