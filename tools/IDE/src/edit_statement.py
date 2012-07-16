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

from structures import Value

class EditStatement(wx.Panel):
    def __init__(self, parent, API):
        super(EditStatement, self).__init__(parent, style = wx.NO_BORDER)
        self.API = API
        # Just a shorter name
        self.tr = self.API.translater.translate
        # Form layout
        self.main = wx.BoxSizer(wx.VERTICAL)
        self.data = wx.GridBagSizer(hgap = 2, vgap = 1)
        self.main.Add(self.data, 1, wx.EXPAND | wx.ALL, 5)

        # type combo list
        self.type = None
        # name combo list
        self.name = None
        # 3rd tier checkbox list[(text, checkbox), (text, checkbox), .. ]
        self.check = list()
        # 3rd tier combobox list[(text, combobox), (text, combobox), .. ]
        self.combo = list()
        # Force change on 3rd tier flag
        self.forceChange = False
        # New mode flag
        self.newMode = 0

        # Make form look nice and show it
        self.SetBackgroundColour("white")
        self.SetSizer(self.main)
        self.SetAutoLayout(1)
        self.main.Fit(self)
        self.Show()

    def updateStatement(self, statement, saveCallback):
        self.statement = statement
        self.saveCallback = saveCallback
        _type = statement['statementStruct'].type
        _name = statement['statementStruct'].name

        # Check  if 3rd tier needs to be recreated or updated
        changed = True
        if self.newMode == 0:
            if self.type != self.name:
                changed = _type.lower() != self.type.GetValue().lower() or \
                          _name.lower() != self.name.GetValue().lower()
        # Forced recreate?
        changed = self.forceChange or changed
        # Clear flag
        self.forceChange = False
        # Don't remember why.. :(
        if statement['type'] == -1:
            self.data.Clear(True)
            self.type = self.name = None
            self.combo = self.check = list()
        # Generate 1st tier, returns if second is needed
        if self.generateFirstTier(_type):
            names = self.API.getKeywords(_type)
            # Generate 2nd tier, returns if 3rd is needed
            if self.generateSecondTier(_name, names):
                keywords = self.API.getKeywords(_type, _name)
                # Generate 3rd tier
                self.generateThirdTier(keywords, statement['statementStruct'].parameters, changed)
                self.newMode = 0
            else:
                # Clear 3rd tier
                self.generateThirdTier([], [], True)
        else:
            # Activate new mode, means that "Add statement" was pressed
            self.newMode = 2

        self.main.Fit(self)
        self.Layout()

    # Generate or update first comboBox, use, read output, etc...
    def generateFirstTier(self, value):
        if self.type is None:
            # Generate and place both 1st tier objects
            actuatorText = wx.StaticText(self, label = self.tr("Edit actuator") + ":")
            self.data.Add(actuatorText, pos = (0, 0))

            self.type = wx.ComboBox(self, choices = self.API.getKeywords(),
                                    style = wx.CB_DROPDOWN, name = "actuator",
                                    size = (150, 25))

            self.data.Add(self.type, pos = (0, 1))

            self.Bind(wx.EVT_COMBOBOX, self.onActuatorChange, self.type)
            self.Bind(wx.EVT_TEXT, self.onActuatorChange, self.type)

        # Update value if needed
        if value is not None:
            if value.lower() != self.type.GetValue().lower():
                self.type.SetValue(value)
        else:
            self.type.SetValue('')
        # Return if 2nd tier needs to be generated
        return self.type.GetValue() != ''

    # Generate or update second comboBox, Led, Light, Radio, etc...
    def generateSecondTier(self, value, choices = None):
        if self.name is None:
            if choices is None:
                choices = list()
            # Generate and place both 2nd tier objects
            objText = wx.StaticText(self, label = self.tr("Edit object") + ":")
            self.data.Add(objText, pos = (1, 0))

            self.name = wx.ComboBox(self, choices = choices, size = (150, 25),
                                    style = wx.CB_DROPDOWN, name = "object")
            self.data.Add(self.name, pos = (1, 1))
            # Only for title change
            self.Bind(wx.EVT_COMBOBOX, self.onObjChange, self.name)
            self.Bind(wx.EVT_TEXT, self.onObjChange, self.name)

        # Update choices if needed, must do before value update
        if choices != self.name.GetItems():
            self.name.Clear()
            self.name.AppendItems(choices)

        # Update value if needed, must do after choices update
        if value is not None:
            if value.lower() != self.name.GetValue().lower():
                self.name.SetValue(value)
        else:
            self.name.SetValue('')
        # Return if 3rd tier needs to be generated
        return self.name.GetValue() != ''

    def generateThirdTier(self, keywords, realParams, changed):
        # Create new objects
        if changed:
            # Destroy old objects
            for x in self.combo:
                self.data.Remove(x[0])
                self.data.Remove(x[1])
                x[0].Destroy()
                x[1].Destroy()
            for x in self.check:
                self.data.Remove(x[0])
                self.data.Remove(x[1])
                x[0].Destroy()
                x[1].Destroy()

            # Clear checkboxes and comboboxes, contains wrappers for deleted 
            # objects, hope python cleans them up after this :)
            self.check = list()
            self.combo = list()
            # Cycle all posible parameters and draw according boxes
            for parameter in keywords:
                # This is advanced parameter, don't display it
                if parameter[1] == []:
                    continue
                text = wx.StaticText(self, label = self.tr(parameter[0]) + ":")

                # Get real parameter associated with current name
                param = self.API.getParamByName(realParams, parameter[0])
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
                                      name = str(parameter[0]))
                    self.Bind(wx.EVT_TEXT, self.updateOriginal, box)
                    if value != None:
                        box.SetValue(str(value))
                    self.combo.append((text, box))
                else:
                    box = wx.CheckBox(self, name = str(parameter[0]))
                    self.Bind(wx.EVT_CHECKBOX, self.updateOriginal, box)
                    if value != None:
                        box.SetValue(value)
                    self.check.append((text, box))
            # Always starting @ 2nd row, enumeration starts from zero
            row = 2
            # Combo's first
            for x in self.combo:
                self.data.Add(x[0], pos = (row, 0))
                self.data.Add(x[1], pos = (row, 1))
                row += 1
            # Check's after, this way it looks nice
            for x in self.check:
                self.data.Add(x[0], pos = (row, 0))
                self.data.Add(x[1], pos = (row, 1))
                row += 1
        # Edit old objects
        else:
            # Cycle all posible parameters and update according boxe's values
            for parameter in keywords:
                # This is advanced parameter, don't display it, so don't update
                if parameter[1] == []:
                    continue
                # Get real parameter associated with current name
                param = self.API.getParamByName(realParams, parameter[0])
                value = None
                if param != None:
                    if param[1] == None:
                        value = True
                    else:
                        value = param[1].getCode()
                # Update checkbox
                if type(value) is bool:
                    # Search right box
                    for x in self.check:
                        # Check that everything is ok before accessing values
                        if x[1] is not None and param is not None:
                            # Update if correct box found
                            if x[1].GetName() == param[0]:
                                x[1].SetValue(value)
                # Update combobox
                else:
                    # Search right box
                    for x in self.combo:
                        # Check that everything is ok before accessing values
                        if x[1] is not None and param is not None:
                            # Update if correct box found
                            if x[1].GetName() == param[0]:
                                x[1].SetValue(str(value))

    def onActuatorChange(self, event):
        actuator = self.type.GetValue()
        # Don't clear if this is already selected
        if actuator == self.statement['statementStruct'].type:
            return
        # Update statement struct
        self.statement['statementStruct'].parameters = list()
        self.statement['statementStruct'].name = ''
        # Force 3rd tier change
        self.forceChange = True
        # Activate callback
        self.updateOriginal(event)

    def onObjChange(self, event = None):
        obj = self.name.GetValue()
        # Don't clear if this is already selected
        if obj == self.statement['statementStruct'].name:
            return
        # Update statement struct
        self.statement['statementStruct'].name = obj
        # Force 3rd tier change
        self.forceChange = True
        # Activate callback
        self.updateOriginal(event)

    def updateOriginal(self, event):
        obj = event.GetEventObject()
        # If we are in combo box, it would be nice to place cursor back after 
        # we mess with all the synchronizing, so save current cursor position
        if type(obj) != wx.CheckBox:
            insertionPoint = obj.GetInsertionPoint()
        # Process newMode aka "Add statement" pressed
        if self.newMode == 1:
            if self.type.GetValue() != '':
                self.statement['statementStruct'].type = self.type.GetValue()
                # Decrease new mode flag, means that type and name 
                # have been selected
                self.newMode -= 1
                self.saveCallback(self.statement)
            return

        name = obj.GetName()
        value = obj.GetValue()

        # Can't allow these values
        if type(value) != bool:
                value = value.replace(",", "")
                value = value.replace(";", "")
                obj.SetValue(value)

        # Find what was changed and update statement struct accordingly
        if name == "actuator":
            self.statement['statementStruct'].type = value
        elif name == "object":
            self.statement['statementStruct'].name = value
        else: # parameters
            found = False
            # try to find this parameter in struct, it might not be there if 
            # it is used for the first time
            for param in self.statement['statementStruct'].parameters:
                # Check if current parameter is the one changed
                if param[0] == name:
                    found = True
                    # check if parameter should be updated or removed
                    if value != '' and value != False:
                        # TODO: make suffix right
                        param[1].value = value
                        param[1].suffix = None
                    else:
                        self.statement['statementStruct'].parameters.remove(param)
                    continue
            # If such parameter don't exist we need to add it
            if not found:
                newParam = None
                # Boolean value
                if value == True:
                    newParam = (name, None)
                    self.statement['statementStruct'].parameters.append(newParam)
                # String value
                else:
                    # TODO: make suffix right
                    newParam = (name, Value(value, None))
                    self.statement['statementStruct'].parameters.append(newParam)
        self.saveCallback(self.statement)
        # Place cursor in last edited place(works only on manual edit)
        # TODO: sometimes hangs entire app... :(
        # "and obj" should check if it is not deleted
        if type(obj) != wx.CheckBox and obj:
            obj.SetFocus()
            obj.SetInsertionPoint(insertionPoint + 1)
