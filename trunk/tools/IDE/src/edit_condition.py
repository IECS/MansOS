# -*- coding: utf-8 -*-
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
import wx.lib.scrolledpanel as scrolled

from structures import Expression, CodeBlock
from globals import * #@UnusedWildImport

class EditCondition(scrolled.ScrolledPanel):
    def __init__(self, parent, API, condition, saveCallback):
        super(EditCondition, self).__init__(parent, style = wx.RAISED_BORDER)
        self.saveCallback = saveCallback
        self.API = API
        # Just a shorter name
        self.tr = self.API.translater.translate
        self.main = wx.BoxSizer(wx.VERTICAL)
        self.data = wx.GridBagSizer(hgap = 2, vgap = 1)
        self.main.Add(self.data, 1, wx.EXPAND | wx.ALL, 5)

        self.next = None
        self.choices = []
        self.oldValues = {}
        self.text = []

        if condition == None:
            self.condition = (CodeBlock(CONDITION, Expression(None, None, ''), [], None), 0, 0, 0, 0)
            self.newMode = 2
        else:
            self.condition = condition
            self.newMode = 0
        self.generateWhenSelect()

        assert type(self.condition[0]) is CodeBlock, "Wrong type passed"

        self.generateElseWhenSelects()

        self.SetBackgroundColour("grey")

        self.SetSizer(self.main)
        self.SetAutoLayout(1)
        self.main.Fit(self)
        self.SetupScrolling()
        self.Show()

    def generateWhenSelect(self):
        # Generate all objects
        self.whenText = wx.StaticText(self, label = self.tr("When") + ":")
        self.when = wx.ComboBox(self, choices = self.API.getDefaultConditions(),
                                style = wx.CB_DROPDOWN, name = "when",
                                size = (200, 25))
        if self.condition[0].condition is not None:
            self.when.SetValue(self.condition[0].condition.getCode())
            self.oldValues['when'] = self.condition[0].condition.getCode()

        self.Bind(wx.EVT_COMBOBOX, self.updateOriginal, self.when)
        self.Bind(wx.EVT_TEXT, self.updateOriginal, self.when)

        # Add them to layout
        self.data.Add(self.whenText, pos = (0, 0))
        self.data.Add(self.when, pos = (0, 1))
        # Set used row count
        self.row = 1

    def generateElseWhenSelects(self):
        # Cycle all parameters and draw according boxes
        nextCond, condition = self.getNextCondition(self.condition[0].next)
        while nextCond is not None:
            self.text.append(wx.StaticText(self, label = self.tr("Elsewhen") + ":"))
            self.data.Add(self.text[-1], pos = (self.row, 0))
            self.choices.append(wx.ComboBox(self, style = wx.CB_DROPDOWN,
                            choices = self.API.getDefaultConditions(),
                            name = str(self.row), size = (200, 25)))
            self.Bind(wx.EVT_COMBOBOX, self.updateOriginal, self.choices[-1])
            self.Bind(wx.EVT_TEXT, self.updateOriginal, self.choices[-1])

            self.choices[-1].SetValue(condition)
            self.oldValues[str(self.row)] = str(condition)

            self.data.Add(self.choices[-1], pos = (self.row, 1))
            self.row += 1
            nextCond, condition = self.getNextCondition(nextCond)

    def getNextCondition(self, condition):
        if condition == None:
            return (None, '')
        if condition.next == None:
            return (None, '')
        return (condition.next, condition.condition.getCode())

    def updateOriginal(self, event = None):
        obj = event.GetEventObject()
        self.oldValues[obj.GetName()] = obj.GetValue()
        self.saveCallback(obj.GetName(), obj.GetValue())
