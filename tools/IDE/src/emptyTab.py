import wx
import globals as g

class EmptyTab(wx.Panel):
    def __init__(self, parent):
        wx.Panel.__init__(self, parent)
        self.projectType = g.SEAL_PROJECT