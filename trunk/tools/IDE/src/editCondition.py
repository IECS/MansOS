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
import editorManager
import globals as g
import condContainer
import Condition

class editDialog(wx.Dialog):
    def __init__(self, parent, API, condition, saveCallback):
        super(editDialog, self).__init__(parent = parent, 
             style = wx.DEFAULT_DIALOG_STYLE | wx.RESIZE_BORDER)
        self.API = API
        # Just a shorter name
        self.tr = self.API.translater.translate
        
        self.box = []
        self.elseWhenCode = []
        self.SetTitle(self.tr("Edit condition"))
        self.saveCallback = saveCallback
        if condition == '':
            condition = condContainer.condContainer()
            condition.setWhen(Condition.Condition("when"))
        self.condition = condition
        # Global sizer for current tab
        self.main = wx.BoxSizer(wx.VERTICAL)
        self.main.AddSpacer((0,10))
        self.whenSizer = wx.BoxSizer(wx.VERTICAL)
        self.elseWhenSizer = wx.BoxSizer(wx.VERTICAL)
        self.elseSizer = wx.BoxSizer(wx.VERTICAL)
        self.main.Add(self.whenSizer, 1, wx.EXPAND | wx.ALL, 1);
        
        self.whenCode = editorManager.EditorManager(self, self.API, True)
        code = ''
        for statement in condition.getWhen().getStatements():
            code += statement.getCode('') + "\n"
        self.whenCode.changeCode(code, True)
        
        self.elseCode = editorManager.EditorManager(self, self.API, True)
        if condition.getElse() == None:
            self.elseCode.changeCode("", True)
        else:
            self.elseCode.changeCode(condition.getElse().getStatementCode(''), True)
        
        self.box.append(self.addStatementField(self.API.getDefaultConditions(), 
                                               "When", self.whenSizer, 
                                               self.whenCode, None))
        self.box[0].SetValue(condition.getWhen().getCondition())
        
        noElseWhen = True
        for x in condition.getElseWhen():
            noElseWhen = False
            sizer = wx.BoxSizer(wx.VERTICAL)
            self.elseWhenCode.append(editorManager.EditorManager(self, self.API, True))
            self.box.append(self.addStatementField(self.API.getDefaultConditions(), 
                                               "elsewhen", sizer, 
                                               self.elseWhenCode[-1], None))
            self.box[-1].SetValue(x.getCondition())
            code = ''
            for statement in x.getStatements():
                code += statement.getCode('') + "\n"
            self.elseWhenCode[-1].changeCode(code, True)
            self.elseWhenSizer.Add(sizer, 1, wx.EXPAND | wx.ALL, 1);
        if not noElseWhen:
            self.main.Add(self.elseWhenSizer, 1, wx.EXPAND | wx.ALL, 1);
            
        self.main.Add(self.elseSizer, 1, wx.EXPAND | wx.ALL, 1);
        
        self.box.append(self.addStatementField(None, "else", self.elseSizer, 
                                               self.elseCode, None))
                      
        self.buttonPane = wx.BoxSizer(wx.HORIZONTAL)
        self.save = wx.Button(self, label = self.tr("Save"), size = (150, -1), name = "save")
        self.Bind(wx.EVT_BUTTON, self.saveClk, self.save)
        self.buttonPane.Add(self.save, 0)
        self.close = wx.Button(self, label = self.tr("Close"), size = (150, -1), name = "close")
        self.Bind(wx.EVT_BUTTON, self.saveClk, self.close)
        self.buttonPane.Add(self.close, 0)
        self.main.Add(self.buttonPane, 0)
        
        self.SetSizer(self.main)
        self.Show()

    def addStatementField(self, choices, label, contSizer, editor, callback):
        sizer = wx.BoxSizer(wx.HORIZONTAL)
        sizer.Add(wx.StaticText(self, label = label + " "))
        if choices != None:
            box = wx.ComboBox(self, choices = choices, style = wx.CB_DROPDOWN)
            sizer.Add(box)
        else:
            box = None
        sizer.Add(wx.StaticText(self, label = ":"))
        contSizer.Add(sizer)
        contSizer.Add(editor, 1, wx.EXPAND | wx.ALL, 5);
        return box
    
    def saveClk(self, event):
        if (event.GetEventObject().GetName() == "save"):
            # Prepare input for parsing, tabs etc. are not important here!
            whenComment = self.condition.getWhen().getComment()
            result = whenComment.getPreComments("")
            result += '\nwhen ' + self.box[0].GetValue() + ': ' 
            result += whenComment.getPostComment(True)
            result += '\n' + self.whenCode.code.GetText()
            
            elseWhen = self.condition.getElseWhen()
            for x in range(0, len(self.elseWhenCode)):
                result += elseWhen[x].getComment().getPreComments("") + '\n'
                result += "elsewhen " + self.box[x + 1].GetValue() + ': '
                result += elseWhen[x].getComment().getPostComment(True) + '\n'
                result += self.elseWhenCode[x].code.GetText()
            elseText = self.elseCode.code.GetText()
            if  elseText!= "":
                else_ = self.condition.getElse()
                if else_ != None:
                    result += else_.getComment().getPreComments("")
                result += '\nelse: ' 
                if self.condition.getElse() != None:
                    result += else_.getComment().getPostComment(True) + '\n'
                result += elseText
            
            endComment = self.condition.getEndComment()
            result += endComment.getPreComments("") + '\nend '
            result += endComment.getPostComment(True)

            data = self.API.sealParser.run(result)
            
            # returning [0] because parser should only return one element
            if len(data) == 1:
                self.saveCallback(True, data[0])
            else:
                self.API.logMsg(g.ERROR, "Parsing error in:\n" + result)
        else:
            self.saveCallback(False, None)
