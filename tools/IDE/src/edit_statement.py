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

from structures import Value, ComponentUseCase

class EditStatement(wx.Panel):
    def __init__(self, parent, API, statement, saveCallback):
        super(EditStatement, self).__init__(parent, style = wx.NO_BORDER)
        self.saveCallback = saveCallback
        self.API = API
        # Just a shorter name
        self.tr = self.API.translater.translate
        self.main = wx.BoxSizer(wx.VERTICAL)
        self.data = wx.GridBagSizer(hgap = 2, vgap = 1)
        self.main.Add(self.data, 1, wx.EXPAND | wx.ALL, 5)

        self.choices = list()
        self.oldValues = dict()
        self.text = list()

        if statement == None:
            self.statement = (ComponentUseCase('', '', list(), None), 0, 0, 0, 0)
            self.newMode = 2
        else:
            self.statement = statement
            self.newMode = 0

        assert type(self.statement[0]) is ComponentUseCase, "Wrong type"

        self.generateActuatorSelect()

        self.generatePatameterSelects()

        self.SetBackgroundColour("white")

        self.SetSizer(self.main)
        self.SetAutoLayout(1)
        self.main.Fit(self)
#        self.SetupScrolling()
        #self.Show()
        self.Show()

    def generateActuatorSelect(self):
        # Generate all objects
        self.actuatorText = wx.StaticText(self, label = self.tr("Edit actuator") + ":")
        self.objText = wx.StaticText(self, label = self.tr("Edit object") + ":")
        self.actuator = wx.ComboBox(self, choices = self.API.getKeywords(),
                                style = wx.CB_DROPDOWN, name = "actuator",
                                size = (150, 25))
        self.Bind(wx.EVT_COMBOBOX, self.onActuatorChange, self.actuator)
        self.Bind(wx.EVT_TEXT, self.onActuatorChange, self.actuator)
        self.actuator.SetValue(self.statement[0].type)
        self.oldValues['actuator'] = self.statement[0].type

        choices = self.API.getKeywords(self.statement[0].type)
        for x in choices:
            if x.lower() == self.statement[0].name.lower():
                self.statement[0].name = x
                break;
        self.obj = wx.ComboBox(self, choices = choices, size = (150, 25),
                                style = wx.CB_DROPDOWN, name = "object")
        # Only for title change
        self.Bind(wx.EVT_COMBOBOX, self.onObjChange, self.obj)
        self.Bind(wx.EVT_TEXT, self.onObjChange, self.obj)

        self.obj.SetValue(self.statement[0].name)
        self.oldValues['object'] = self.statement[0].name
        # Add them to layout
        self.data.Add(self.actuatorText, pos = (0, 0))
        self.data.Add(self.actuator, pos = (0, 1))
        self.data.Add(self.objText, pos = (1, 0))
        self.data.Add(self.obj, pos = (1, 1))
        # Set used row count
        self.row = 2
        if self.newMode == 2:
            self.obj.Hide()
            self.objText.Hide()
            self.newMode -= 1

    def generatePatameterSelects(self):
        # Cycle all posible parameters and draw according boxes
        for parameter in self.API.getKeywords(self.statement[0].type, self.statement[0].name):
            if parameter[1] == []:
                continue
            self.text.append(wx.StaticText(self, label = self.tr(parameter[0]) + ":"))
            self.data.Add(self.text[-1], pos = (self.row, 0))

            # Get real parameter associated with current name
            param = self.API.getParamByName(self.statement[0].parameters,
                                                      parameter[0])
            value = None

            if param != None:
                if param[1] == None:
                    value = True
                else:
                    value = param[1].getCode()

            # Create combobox or checkbox
            if type(parameter[1][0]) is not bool:
                box = wx.ComboBox(self, choices = [""] + parameter[1],
                                  style = wx.CB_DROPDOWN, size = (150, 25),
                                  id = len(self.choices),
                                  name = str(parameter[0]))
                self.Bind(wx.EVT_TEXT, self.updateOriginal, box)
                if value != None:
                    box.SetValue(str(value))
                    self.oldValues[parameter[0]] = str(value)
            else:
                box = wx.CheckBox(self, id = len(self.choices),
                                  name = str(parameter[0]))
                self.Bind(wx.EVT_CHECKBOX, self.updateOriginal, box)
                if value != None:
                    box.SetValue(value)
                    self.oldValues[parameter[0]] = value
            # Add created object and real parameter if exists
            self.choices.append(param)

            self.data.Add(box, pos = (self.row, 1))
            self.row += 1

    def onActuatorChange(self, event):
        actuator = self.actuator.GetValue()
        # Don't clear if this is already selected
        if actuator == self.statement[0].type:
            return
        self.statement[0].parameters = list()
        self.statement[0].name = ''
        self.updateOriginal(event)

    def onObjChange(self, event = None):
        obj = self.obj.GetValue()
        # Don't clear if this is already selected
        if obj == self.statement[0].name:
            return


        self.statement[0].name = obj
        self.updateOriginal(event)

    def updateOriginal(self, event = None):
        obj = event.GetEventObject()
        if type(obj) != wx.CheckBox:
            insertionPoint = obj.GetInsertionPoint()
        # Process newMode
        if self.newMode == 1:
            if self.actuator.GetValue() != '':
                self.obj.Show()
                self.objText.Show()
                self.statement[0].type = self.actuator.GetValue()
                self.newMode -= 1
                self.saveCallback(self.statement)
            return

        name = obj.GetName()
        value = obj.GetValue()

        # Can't allow this
        if type(value) != bool:
                value = value.replace(",", "")
                value = value.replace(";", "")
                obj.SetValue(value)

        if name == "actuator":
            self.statement[0].type = value
        elif name == "object":
            self.statement[0].name = value
        else: # parameters
            nr = obj.GetId()
            param = self.choices[nr]
            if param is not None:
                # TODO: make suffix right!
                if value != '' and value != False:
                    param[1].value = value
                    param[1].suffix = None
                    self.choices[nr] = param
                else:
                    self.statement[0].parameters.remove(param)
                    self.choices[nr] = None
            else:
                newParam = None
                if value == True:
                    newParam = (name, None)
                    self.statement[0].parameters.append(newParam)
                else:
                    newParam = (name, Value(value, None))
                    self.statement[0].parameters.append(newParam)
                self.choices[nr] = newParam
        self.saveCallback(self.statement)
        print "got here"
        # Place cursor in last edited place(works only on manual edit)
        #if type(obj) != wx.CheckBox:
        #    obj.SetFocus()
        #    obj.SetInsertionPoint(insertionPoint + 1)
        print "got here2"
        return
