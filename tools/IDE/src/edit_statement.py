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

from statement import Statement

class EditStatement(scrolled.ScrolledPanel):
    def __init__(self, parent, API, statement, saveCallback):
        super(EditStatement, self).__init__(parent, style = wx.RAISED_BORDER)
        self.saveCallback = saveCallback
        self.API = API
        # Just a shorter name
        self.tr = self.API.translater.translate
        self.main = wx.BoxSizer(wx.VERTICAL)
        self.data = wx.GridBagSizer(hgap = 2, vgap = 1)
        self.main.Add(self.data, 1, wx.EXPAND | wx.ALL, 5)

        self.choices = []
        self.oldValues = {}
        self.text = []
        self.newMode = False

        self.statement = statement
        self.generateActuatorSelect()

        data = self.API.getActuatorInfo(statement.getMode())['parameters']
        self.generatePatameterSelects(data)

        self.SetBackgroundColour("black")

        self.SetSizer(self.main)
        self.SetAutoLayout(1)
        self.main.Fit(self)
        self.SetupScrolling()
        self.Show()

    def generateActuatorSelect(self):
        # Generate all objects
        self.actuatorText = wx.StaticText(self, label = self.tr("Edit actuator") + ":")
        self.objText = wx.StaticText(self, label = self.tr("Edit object") + ":")
        self.actuator = wx.ComboBox(self, choices = self.API.getAllStatementActuators(),
                                style = wx.CB_DROPDOWN, name = "actuator",
                                size = (150, 25))
        self.Bind(wx.EVT_COMBOBOX, self.onActuatorChange, self.actuator)
        self.Bind(wx.EVT_TEXT, self.onActuatorChange, self.actuator)
        self.actuator.SetValue(self.statement.getMode())
        self.oldValues['actuator'] = self.statement.getMode()
        choices = self.API.getActuatorInfo(self.statement.getMode())['objects']
        self.obj = wx.ComboBox(self, choices = choices, size = (150, 25),
                                style = wx.CB_DROPDOWN, name = "object")
        # Only for title change
        self.Bind(wx.EVT_COMBOBOX, self.updateOriginal, self.obj)
        self.Bind(wx.EVT_TEXT, self.updateOriginal, self.obj)

        self.obj.SetValue(self.statement.getObject())
        self.oldValues['object'] = self.statement.getObject()
        # Add them to layout
        self.data.Add(self.actuatorText, pos = (0, 0))
        self.data.Add(self.actuator, pos = (0, 1))
        self.data.Add(self.objText, pos = (1, 0))
        self.data.Add(self.obj, pos = (1, 1))
        # Set used row count
        self.row = 2
        if self.statement.getMode() == '':
            self.obj.Hide()
            self.objText.Hide()
            self.newMode = True

    def generatePatameterSelects(self, data):
        # Cycle all parameters and draw according boxes
        for parameter in data:
            self.text.append(wx.StaticText(self, label = self.tr(parameter.getName()) + ":"))
            self.data.Add(self.text[-1], pos = (self.row, 0))
            paramValue = self.statement.getParamValueByName(parameter.getName())
            if parameter.getValue() != None:
                self.choices.append(wx.ComboBox(self, choices = parameter.getValue(),
                                                style = wx.CB_DROPDOWN,
                                                name = parameter.getName(),
                                                size = (150, 25)))
                self.Bind(wx.EVT_COMBOBOX, self.updateOriginal, self.choices[-1])
                self.Bind(wx.EVT_TEXT, self.updateOriginal, self.choices[-1])
                if paramValue != None:
                    self.choices[-1].SetValue(str(paramValue))
                    self.oldValues[parameter.getName()] = str(paramValue)
            else:
                self.choices.append(wx.CheckBox(self, name = parameter.getName()))
                self.Bind(wx.EVT_CHECKBOX, self.updateOriginal, self.choices[-1])
                if paramValue != None:
                    self.choices[-1].SetValue(paramValue)
                self.oldValues[parameter.getName()] = paramValue

            self.data.Add(self.choices[-1], pos = (self.row, 1))
            self.row += 1

    def onActuatorChange(self, event):
        if event != None:
            wx.Yield()
        actuator = self.actuator.GetValue()
        self.updateOriginal(event)
        # Don't clear if this is already selected
        if actuator == self.statement.getMode():
            return
        allActuators = self.API.getActuatorInfo(actuator)
        if allActuators != None:
            # Generate new statement
            newStatement = Statement(actuator, self.obj.GetValue())
            newStatement.setComment(self.statement.getComment())
            self.statement = newStatement
            definition = self.API.getActuatorInfo(newStatement.getMode())
            data = definition['parameters']
            # Clear All
            self.clearAll()
            self.generateActuatorSelect()
            self.generatePatameterSelects(data)

            self.Layout()

    def clearAll(self):
        # Can't use for loop because it sometimes causes 
        # return of pointer to already destroyed object, because of rule that 
        # list can't be altered while looped with for
        while (len(self.choices) != 0):
            self.choices.pop().Destroy()
        while (len(self.text) != 0):
            self.text.pop().Destroy()
        self.actuator.Destroy()
        self.actuatorText.Destroy()
        self.objText.Destroy()
        self.obj.Destroy()
        self.data.Clear()

    def updateOriginal(self, event = None):
        if self.newMode and self.obj.GetValue() == '':
            return
        elif self.newMode:
            self.saveCallback(self.actuator.GetValue(), self.obj.GetValue(), '')
            self.newMode = False
            return
        name = event.GetEventObject().GetName()
        value = event.GetEventObject().GetValue().replace(",", "")
        event.GetEventObject().SetValue(value)
        oldVal = None
        if name in self.oldValues:
            oldVal = self.oldValues[name]
        self.oldValues[name] = value
        self.saveCallback(name, value, oldVal)
