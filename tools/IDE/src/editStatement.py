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
import Parameter
import Statement

class editDialog(wx.Dialog):
    
    def __init__(self, parent, API, statement, saveCallback):
        super(editDialog, self).__init__(parent = parent, 
            size = (500, 400),
            style = wx.DEFAULT_DIALOG_STYLE | wx.RESIZE_BORDER)
        self.saveCallback = saveCallback
        self.API = API
        # Just a shorter name
        self.tr = self.API.translater.translate
        self.main = wx.BoxSizer(wx.VERTICAL)
        self.data = wx.GridBagSizer(hgap = 2, vgap = 1)
        #self.main.Add(wx.StaticText(self, label = "Edit " + statement.getTypeAndObject()))
        self.main.AddSpacer((10, 10))
        self.main.Add(self.data, 0)
        self.main.AddSpacer((10, 10))
        self.choices = []
        self.text = []
        
        self.statement = statement
        self.generateActuatorSelect()
        
        data = self.API.getActuatorInfo(statement.getMode())['parameters']
        self.generatePatameterSelects(data)
        
        self.buttonPane = wx.BoxSizer(wx.HORIZONTAL)
        self.save = wx.Button(self, label = self.tr("Save"),
                              size = (150, -1), name = "save")
        self.Bind(wx.EVT_BUTTON, self.saveClk, self.save)
        self.buttonPane.Add(self.save,
                      1,            # make vertically stretchable
                      wx.EXPAND |    # make horizontally stretchable
                      wx.ALL,        #   and make border all around
                      1 );         # set border width to 10))
        self.close = wx.Button(self, label = self.tr("Close"),
                               size = (150, -1), name = "close")
        self.Bind(wx.EVT_BUTTON, self.saveClk, self.close)
        self.buttonPane.Add(self.close,
                      0,            # make vertically stretchable
                      wx.EXPAND |    # make horizontally stretchable
                      wx.ALL,        #   and make border all around
                      1 );         # set border width to 10))
        self.main.Add(self.buttonPane,
                      0,            # make vertically stretchable
                      wx.EXPAND |    # make horizontally stretchable
                      wx.ALL,        #   and make border all around
                      1 );         # set border width to 10))
        
        self.updateTitle()
        self.SetSizer(self.main)
        self.main.Fit(self)
        self.Show()
    
    def saveClk(self, event):
        if (event.GetEventObject().GetName() == "save"):
            statement = Statement.Statement(self.actuator.GetValue(),
                                            self.obj.GetValue())
            statement.addComment(self.statement.getComment())
            for x in self.choices:
                if x.GetValue() != '' and x.GetValue() != False:
                    statement.addParameter(Parameter.Parameter(x.GetName(), x.GetValue()))
            self.saveCallback(True, statement)
        else:
            self.saveCallback(False, None)

    def generateActuatorSelect(self):
        # Generate all objects
        self.actuatorText = wx.StaticText(self, label = self.tr("Edit actuator") + ":")
        self.objText = wx.StaticText(self, label = self.tr("Edit object") + ":")
        self.actuator = wx.ComboBox(self, choices = self.API.getAllStatementActuators(), 
                                style = wx.CB_DROPDOWN, name = "actuator")
        self.Bind(wx.EVT_COMBOBOX, self.onActuatorChange, self.actuator)
        self.Bind(wx.EVT_TEXT, self.onActuatorChange, self.actuator)
        self.actuator.SetValue(self.statement.getMode())
        self.obj = wx.ComboBox(self, choices = self.API.getActuatorInfo(self.statement.getMode())['objects'], 
                                style = wx.CB_DROPDOWN, name = "object")
        # Only for title change,
        self.Bind(wx.EVT_COMBOBOX, self.updateTitle, self.obj)
        self.Bind(wx.EVT_TEXT, self.updateTitle, self.obj)
        self.obj.SetValue(self.statement.getObject())
        # Add them to layout
        self.data.Add(self.actuatorText, pos = (0, 0))
        self.data.Add(self.actuator, pos = (0, 1))
        self.data.Add(self.objText, pos = (1, 0))
        self.data.Add(self.obj, pos = (1, 1))
        # Set used row count
        self.row = 2
    
    def generatePatameterSelects(self, data):
        # Cycle all parameters and draw according boxes
        for parameter in data:
            self.text.append(wx.StaticText(self, label = self.tr(parameter.getName()) + ":"))
            self.data.Add(self.text[-1], pos = (self.row,0))
            if parameter.getValue() != None:
                self.choices.append(wx.ComboBox(self, choices = parameter.getValue(), 
                                                style = wx.CB_DROPDOWN, name = parameter.getName()))
            else:
                self.choices.append(wx.CheckBox(self, name = parameter.getName()))
            paramValue = self.statement.getParamValueByName(parameter.getName())
            if paramValue != None:
                self.choices[-1].SetValue(paramValue)
            self.data.Add(self.choices[-1], pos = (self.row, 1))
            self.row += 1
    
    def onActuatorChange(self, event):
        actuator = self.actuator.GetValue()
        self.updateTitle()
        # Don't clear if this is already selected
        if actuator == self.statement.getMode():
            return
        allActuators = self.API.getActuatorInfo(actuator)
        if allActuators != None:
            # Generate new statement
            statement = Statement.Statement(actuator,
                                            self.obj.GetValue())
            statement.addComment(self.statement.getComment())
            self.statement = statement
            definition = self.API.getActuatorInfo(statement.getMode())
            data = definition['parameters']
            # Clear All
            self.clearAll()
            self.generateActuatorSelect()
            self.generatePatameterSelects(data)
            self.data.Layout()
            self.main.Fit(self)
            self.main.Fit(self)
            self.main.Layout()
    
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
    
    def updateTitle(self, event = None):
        self.SetTitle(self.tr("Edit") + " \"" + 
                self.actuator.GetValue() +" " + self.obj.GetValue() + "\"")