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

from globals import * #@UnusedWildImport
from Translater import localize

class EditStatement(wx.Panel):
    def __init__(self, parent, API):
        super(EditStatement, self).__init__(parent, style = wx.NO_BORDER)
        self.API = API

        self.mode = UNKNOWN
        # New mode flag
        self.newMode = 0

    def finishInit(self):
        # Make form look nice and show it
        self.SetBackgroundColour("white")
        self.SetSizer(self.main)
        self.SetAutoLayout(True)
        self.main.Fit(self)
        self.Show()

### Update

    def update(self):
        if self.API.tabManager.getPageObject() != None:
            self.editor = self.API.tabManager.getPageObject().code
            self.source = self.editor.checkForComment(self.editor.findStatement())
            if self.source is not None:
                if self.source['type'] == STATEMENT:
                    self.updateStatement()
                if self.source['type'] == CONDITION:
                    self.updateCondition()
                return
        self.clearStatement()
        self.clearCondition()
        self.mode = UNKNOWN

### Statement processing

    def initStatement(self):
        self.clearCondition()
        # Form layout
        self.main = wx.BoxSizer(wx.VERTICAL)
        self.data = wx.GridBagSizer(hgap = 2, vgap = 1)
        self.main.Add(self.data, 1, wx.EXPAND | wx.ALL, 5)

        # Type combo list
        self.type = None
        # Name combo list
        self.name = None
        # 3rd tier checkbox list[(text, checkbox), (text, checkbox), .. ]
        self.check = list()
        # 3rd tier combobox list[(text, combobox), (text, combobox), .. ]
        self.combo = list()
        # Force change on 3rd tier flag
        self.forceChange = False
        self.finishInit()

    def clearStatement(self):
        if self.mode == STATEMENT:
            self.main.Clear(True)
            del(self.main)
            del(self.data)
            del(self.type)
            del(self.name)
            del(self.combo)
            del(self.check)
            del(self.forceChange)
            self.mode = UNKNOWN

    def updateStatement(self):
        # Check if we need do do a full erase
        if self.mode != STATEMENT:
            self.initStatement()
        # Declare that we edit statement now
        self.mode = STATEMENT
        params = self.source['text'].split(",")
        temp = (params[0].strip() + " ").split(None, 1)
        if len(temp) > 0:
            self._type = temp[0]
        else:
            self._type = ""
        if len(temp) > 1:
            self._name = temp[1]
        else:
            self._name = ""
        self._type = self._type.strip()
        self._name = self._name.strip()
        parameters = dict()
        if len(params) > 1:
            for x in params[1:]:
                temp = (x.strip() + " ").split(None, 1)
                # only ','
                if len(temp) == 0:
                    pass
                # bool
                elif len(temp) == 1:
                    parameters[temp[0].strip().lower()] = ''
                else:
                    temp[1] = temp[1].strip()
                    if temp[1].startswith('"') or temp[1].startswith("'"):
                        # Take all in quotation marks
                        value = temp[1].split(temp[1][0])
                        if len(value) > 1:
                            value = value[1]
                        else:
                            value = ''
                    else:
                        # Take only first word as value.
                        # Ignore any whitespace chars between name and value.
                        value = (temp[1] + " ").split(None, 1)[0]
                    parameters[temp[0].strip().lower()] = value
        # Check if 3rd tier needs to be recreated or updated
        changed = True
        if self.newMode == 0:
            if self.type != self.name and self.name != None:
                changed = self._type.lower() != self.type.GetValue().lower() or \
                          self._name.lower() != self.name.GetValue().lower()
        # Forced recreate?
        changed = self.forceChange or changed
        # Clear flag
        self.forceChange = False
        # Clear everything because "Add statement pressed"
        #if statement['type'] == -1:
        #    self.clearAll()
        # Generate 1st tier, returns if second is needed

        if self.generateFirstTier(self._type):
            names = self.API.getKeywords(self._type)
            # Generate 2nd tier, returns if 3rd is needed
            if self.generateSecondTier(self._name, names):
                keywords = self.API.getKeywords(self._type, self._name)
                # Generate 3rd tier
                self.generateThirdTier(keywords, parameters, changed)
                self.newMode = 0
            else:
                # Clear 3rd tier
                self.generateThirdTier([], [], True)
        else:
            # Activate new mode, means that "Add statement" was pressed
            self.newMode = 2
        # Push changes to screen
        self.main.Fit(self)

        #self.Layout()
        self.Refresh()
        self.main.Layout()

    # Generate or update first comboBox, use, read output, etc...
    def generateFirstTier(self, value):
        if self.type is None:
            # Generate and place both 1st tier objects
            actuatorText = wx.StaticText(self, label = localize("Edit actuator") + ":")
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
                self.type.SetValue(str(value))
        else:
            self.type.SetValue('')
        # Return if 2nd tier needs to be generated
        return True #self.type.GetValue() != ''

    # Generate or update second comboBox, Led, Light, Radio, etc...
    def generateSecondTier(self, value, choices = None):
        if self.name is None:
            if choices is None:
                choices = list()
            # Generate and place both 2nd tier objects
            objText = wx.StaticText(self, label = localize("Edit object") + ":")
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
                if parameter[2] == True:
                    continue
                text = wx.StaticText(self, label = localize(parameter[0]) + ":")

                # Get real parameter associated with current name
                value = self.API.getParamByName(realParams, parameter[0])[1]
                try:
                    if len(parameter[1]) == 0:
                        parameter[1].append("")
                except Exception:
                    parameter[1] = [""]
                # Create combobox or checkbox
                if type(parameter[1][0]) is not bool:
                    box = wx.ComboBox(self, choices = [""] + parameter[1],
                                      style = wx.CB_DROPDOWN, size = (150, 25),
                                      name = str(parameter[0]))
                    self.Bind(wx.EVT_TEXT, self.onParamChange, box)
                    if value != None:
                        box.SetValue(str(value))
                    self.combo.append((text, box))
                else:
                    box = wx.CheckBox(self, name = str(parameter[0]))
                    self.Bind(wx.EVT_CHECKBOX, self.onParamChange, box)
                    if value != None:
                        box.SetValue(True)
                    else:
                        box.SetValue(False)
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
            for x in self.check:
                # Check that everything is ok before accessing values
                if x[1] is not None:
                    # Update if correct box found
                    if x[1].GetName().lower() in realParams:
                        x[1].SetValue(True)
                    else:
                        x[1].SetValue(False)
            for x in self.combo:
                    # Check that everything is ok before accessing values
                    if x[1] is not None:
                        # Update if correct box found
                        if x[1].GetName().lower() in realParams:
                            x[1].SetValue(str(realParams[x[1].GetName().lower()]))
                        else:
                            x[1].SetValue("")

    def onActuatorChange(self, event):
        actuator = self.type.GetValue()
        # Don't clear if this is already selected
        if self._type.startswith(actuator) or actuator.startswith("use"):
            space = len(self.source['text'].split(None, 1)[0])
            self.source['text'] = actuator + self.source['text'][space:]
        else:
            self.source['text'] = actuator
            # Force 3rd tier change
            self.forceChange = True
            self.updateStatement()
        self.editor.change(self.source)
        self.source['end'] = self.source['start'] + len(self.source['text'])

    def onObjChange(self, event = None):
        name = self.name.GetValue()
        # Don't clear if this is already selected
        if self._name.startswith(name):
            space = self.source['text'].split(None)
            self.source['text'] = self.source['text'].replace(\
                                          space[1].strip().strip(","), name)
        else:
            self.source['text'] = self.type.GetValue() + " " + name
            # Force 3rd tier change
            self.forceChange = True
            self.updateStatement()
        self.editor.change(self.source)
        self.source['end'] = self.source['start'] + len(self.source['text'])

    def onParamChange(self, event):
        value = event.GetEventObject().GetValue()
        if type(value) is unicode:
            # Can't allow this
            value = str(value.replace(",", "").replace(";", ""))
        name = event.GetEventObject().GetName()
        changed = False
        parts = self.source['text'].split(",")
        # Add new Bool value
        if value == True:
            self.source['text'] += ", " + name
            changed = True
        else:
            # Count part start position
            start = len(parts[0]) + 1
            for x in parts[1:]:
                # Remove Bool value
                if x.lower().strip() == name.lower():
                    if value == False:
                        self.source['text'] = self.source['text'][:start - 1] \
                            + self.source['text'][start + len(x):]
                        changed = True
                        break
                # Try to find edited parameter in this part
                temp = (x.strip() + " ").split(None, 1)
                if len(temp) == 0 or temp[0].lower() != name or len(temp) < 2:
                    start += len(x) + 1
                    continue
                temp[1] = temp[1].strip()
                haveQm = False
                if temp[1].startswith('"') or temp[1].startswith("'"):
                    # Take all in quotation marks
                    val = temp[1].split(temp[1][0])
                    if len(val) > 1:
                        val = val[1]
                    else:
                        val = ''
                    haveQm = True
                else:
                    # Take only first word as value.
                    # Ignore any whitespace chars between name and value.
                    val = (temp[1] + " ").split(None, 1)[0]
                # Remove if no value
                if value.strip() == '':
                    self.source['text'] = self.source['text'][:start - 1] + \
                        self.source['text'][start + len(x):]
                else:
                    # Do not allow space if there is no QM
                    if not haveQm:
                        # Here we have 2 options:
                        #    Discard all after first space.
                        #    Add QM.
                        value = (value + ' ').split(None, 1)[0]
                        # value = '"{}"'.format(value)
                    # Replace with new value
                    self.source['text'] = self.source['text'][:start] + \
                        x.replace(val, value) + self.source['text'][start + len(x):]
                changed = True
                start += len(x) + 1
        # Add parameter if no such found
        if not changed and value.strip() != '':
            self.source['text'] += ", {} {}".format(name, value)
        # Push changes to editor
        self.editor.change(self.source)
        # Update length
        self.source['end'] = self.source['start'] + len(self.source['text'])

### Condition processing

    def initCondition(self):
        self.clearStatement()
        self.main = wx.BoxSizer(wx.VERTICAL)
        self.condCreatorSizer = wx.BoxSizer(wx.HORIZONTAL)
        self.main.Add(self.condCreatorSizer, 1, wx.EXPAND | wx.ALL, 5)
        self.condCreador = None
        self.updateCondCreator(None)
        nextEditable = wx.Button(self, label = localize("Find next parameter"))
        self.Bind(wx.EVT_BUTTON, self.launchNextEditFinder, nextEditable)
        self.main.Add(nextEditable, 0, wx.EXPAND | wx.ALL, 5)
        self.textBox = None
        self.finishInit()

    def clearCondition(self):
        if self.mode == CONDITION:
            self.main.Clear(True)
            del(self.main)
            self.mode = UNKNOWN

    def updateCondition(self):
        self.activeField = None
        # Check if we need do do a full erase
        if self.mode != CONDITION:
            self.initCondition()
        # Declare that we edit statement now
        self.mode = CONDITION
        # Save passed values
        # Process "Add condition" pressed
        if self.source['type'] == UNKNOWN:
            self.clearAll()
            self.newMode = CONDITION

        text, value = "when", ""
        temp = self.source['text'].split(None, 1)
        if len(temp) > 0:
            text = temp[0]
        if len(temp) > 1:
            value = temp[1]
        self.generateFirstLine(text, value)
        self.updateCondCreator(None)
        #self.generateElseWhenSelects()

        # Push changes to screen
        self.main.Fit(self)
        self.condCreatorSizer.Layout()
        self.Layout()

    def generateFirstLine(self, text, value):
        # Generate text and combobox
        if self.textBox is None:
            self.text = wx.StaticText(self, label = localize(text) + ":")
            self.textBox = wx.TextCtrl(self, name = "first", size = (100, 100),
                            style = wx.TE_PROCESS_TAB | wx.TE_PROCESS_ENTER | wx.TE_MULTILINE | wx.TE_AUTO_SCROLL)
            self.textBox.Bind(wx.EVT_CHAR, self.processKeyUp)
            self.textBox.Bind(wx.EVT_TEXT, self.onTextChange)
            # Add them to layout
            self.condCreatorSizer.Add(self.text)
            self.condCreatorSizer.Add(self.textBox, 10, wx.EXPAND | wx.ALL, 5)

        # Update entry with new value
        if text is not None:
            self.text.SetLabel(text)
        if value is not None:
            self.textBox.SetValue(value)
        self.activeField = self.textBox

    def updateCondCreator(self, event):
        if self.condCreador is None:
            condCreatorText = wx.StaticText(self, label = localize("Choose function") + ":")
            self.condCreador = wx.ComboBox(self, choices = [""] + self.API.sealSyntax.getFunctionBodys(),
                                      style = wx.CB_DROPDOWN | wx.CB_READONLY | wx.CB_SIMPLE,
                                      name = "CondCreator")

            self.Bind(wx.EVT_COMBOBOX, self.ekeCondition, self.condCreador)
            self.main.Add(condCreatorText)
            self.main.Add(self.condCreador, 0, wx.EXPAND | wx.ALL, 5)
        else:
            # In future I plan to adjust proposed function depending on selection
            pass

    def ekeCondition(self, event):
        if self.textBox is not None:
            start, end = self.textBox.GetSelection()#
            text = self.textBox.GetString(start, end)
            #print "@eke", "'" + str([text]) + "'", str([self.textBox.GetValue()]), len(self.textBox.GetValue())
            newVal = self.condCreador.GetValue()
            if text.strip() != '' and text.find("[") + text.find("]") == -2:
                start_ = newVal.find("[")
                end_ = newVal.find("]") + 1
                newVal = newVal[:start_] + text + newVal[end_:]
            self.textBox.Replace(start, end, newVal)
            self.setNextEditingSelection(start - 1)
            self.onTextChange(None)

    def setNextEditingSelection(self, start = 0):
        text = self.textBox.GetValue()
        # Weird, but start comes with double newlines counted in
        newLines = text.count("\n")
        start = start - newLines
        if start < 0:
            start = 0
        first = text.find("[", start)
        last = text.find("]", first) + 1
        if first == -1 or last == 0:
            return
        newLines = self.textBox.GetValue()[:last].count("\n")
        self.textBox.SetSelection(first + newLines, last + newLines)
        self.textBox.SetFocus()

    # Catch TAB pressed
    def processKeyUp(self, event):
        keycode = event.GetKeyCode()
        if keycode == wx.WXK_TAB:
            self.launchNextEditFinder()
        else:
            event.Skip()

    def launchNextEditFinder(self, event = None):
        self.setNextEditingSelection(self.textBox.GetInsertionPoint())

    def onTextChange(self, event):
        self.source['text'] = "{} {}".format(self.text.GetLabel(), \
             "false" if self.textBox.GetValue() == "" else self.textBox.GetValue())
        self.editor.change(self.source)
        self.source['end'] = self.source['start'] + len(self.source['text'])
