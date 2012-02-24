# -*- coding: UTF-8 -*-
#
# Copyright (c) 2008-2012 the MansOS team. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#  * Redistributions of source code must retain the above copyright notice,
#    this list of  conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

import wx
import Editor
import sealStruct
import Condition

class editDialog(wx.Dialog):
    def __init__(self, parent, API, condition, saveCallback):
        super(editDialog, self).__init__(parent = parent, 
             style = wx.DEFAULT_DIALOG_STYLE | wx.RESIZE_BORDER)
        self.API = API
        # Just a shorter name
        self.tr = self.API.translater.translate
        
        self.SetTitle(self.tr("Edit condition"))
        self.saveCallback = saveCallback
        if condition == '':
            condition = Condition.Condition()
        self.condition = condition
        # Global sizer for current tab
        self.main = wx.BoxSizer(wx.VERTICAL)
        self.main.AddSpacer((0,20))
        self.whenSizer = wx.BoxSizer(wx.VERTICAL)
        self.elseSizer = wx.BoxSizer(wx.VERTICAL)
        self.elseSizer.Add(wx.StaticText(self, label = "Else:"))
        self.main.Add(self.whenSizer,
                      1,            # make vertically stretchable
                      wx.EXPAND |    # make horizontally stretchable
                      wx.ALL,        #   and make border all around
                      1 );         # set border width to 10))
        self.main.Add(self.elseSizer,
                      1,            # make vertically stretchable
                      wx.EXPAND |    # make horizontally stretchable
                      wx.ALL,        #   and make border all around
                      1 );         # set border width to 10))
        
        self.whenCode = Editor.CodeEditor(self, self.API, True)
        self.whenCode.AddText(condition.getWhenStatementsCode())
        self.elseCode = Editor.CodeEditor(self, self.API, True)
        self.elseCode.AddText(condition.getElseStatementsCode())
        self.box = self.addStatementField(self.API.getDefaultConditions(), None)
        self.box.SetValue(condition.getCondition())
        
        self.buttonPane = wx.BoxSizer(wx.HORIZONTAL)
        self.save = wx.Button(self, label = self.tr("Save"), size = (150, -1), name = "save")
        self.Bind(wx.EVT_BUTTON, self.saveClk, self.save)
        self.buttonPane.Add(self.save, 0)
        self.close = wx.Button(self, label = self.tr("Close"), size = (150, -1), name = "close")
        self.Bind(wx.EVT_BUTTON, self.saveClk, self.close)
        self.buttonPane.Add(self.close, 0)
        self.main.Add(self.buttonPane, 0)
        
        self.SetSizer(self.main)
        #self.main.Fit(self)
        self.Show()

    def addStatementField(self, choices, callback):
        sizer = wx.BoxSizer(wx.HORIZONTAL)
        box = wx.ComboBox(self, choices = choices, style = wx.CB_DROPDOWN)
        #self.Bind(wx.EVT_TEXT_ENTER, callback, box)
        #self.Bind(wx.EVT_COMBOBOX, callback, box)
        sizer.Add(wx.StaticText(self, label = "When "))
        sizer.Add(box)
        sizer.Add(wx.StaticText(self, label = ":"))
        self.whenSizer.Add(sizer)
        self.whenSizer.Add(self.whenCode,
                      1,            # make vertically stretchable
                      wx.EXPAND |    # make horizontally stretchable
                      wx.ALL,        #   and make border all around
                      10 );         # set border width to 10))
        self.elseSizer.Add(self.elseCode, 
                      1,            # make vertically stretchable
                      wx.EXPAND |    # make horizontally stretchable
                      wx.ALL,        #   and make border all around
                      10 );         # set border width to 10))
        return box
    
    def saveClk(self, event):
        if (event.GetEventObject().GetName() == "save"):
            result = self.condition.getComments() + 'when ' + self.box.GetValue() + ':\n' + self.whenCode.GetText() + 'else\n' + self.elseCode.GetText() + 'end'
            data = sealStruct.Seal(self.API, result)
            self.saveCallback(True, data.getStruct()['conditions'][0])
        else:
            self.saveCallback(False, None)