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

import wx.stc
import time
import re

from seal_struct import SealStruct
from edit_statement import EditStatement
from statement import Statement
from condition_container import ConditionContainer
from edit_condition import EditCondition
from globals import * #@UnusedWildImport

class Editor(wx.stc.StyledTextCtrl):
    def __init__(self, parent, API, noUpdate = False):
        wx.stc.StyledTextCtrl.__init__(self, parent)
        self.activePoints = []
        self.activeRadius = 0
        self.API = API
        self.spaces = ''
        self.noUpdate = noUpdate
        self.lastText = ''
        self.lastAutoEdit = 0
        self.newMode = False

        # Set scroll bar range
        self.SetEndAtLastLine(True)
        self.SetIndentationGuides(True)

        # Set style
        font = wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT)
        self.StyleSetSpec(wx.stc.STC_STYLE_DEFAULT, "face:%s,size:10" % font.GetFaceName())
        self.StyleSetSpec(wx.stc.STC_STYLE_LINENUMBER, "back:green,face:%s,size:10" % font.GetFaceName())

        # Set captured events
        self.SetModEventMask(wx.stc.STC_PERFORMED_UNDO | wx.stc.STC_PERFORMED_REDO | wx.stc.STC_MOD_DELETETEXT | wx.stc.STC_MOD_INSERTTEXT | wx.stc.STC_LEXER_START)

        self.SetUseAntiAliasing(True)

        self.Bind(wx.stc.EVT_STC_MODIFIED, self.doGrammarCheck)
        self.Bind(wx.stc.EVT_STC_UPDATEUI, self.getAction)

    def setLineNumbers(self, enable = True):
        if enable:
            width = len(str(self.GetLineCount()))
            self.SetMarginType(0, wx.stc.STC_MARGIN_NUMBER)
            self.SetMarginWidth(0, self.TextWidth(wx.stc.STC_STYLE_LINENUMBER,
                                                  (width + 1) * '0'))
        else:
            self.SetMarginWidth(0, 0)
        # Disable second margin
        self.SetMarginWidth(1, 0)

    def doGrammarCheck(self, event):
        if event != None:
            wx.Yield()
        if self.GetParent().emmbeddedMode == False:
            # Mark that file has changed
            # TODO: fix undo not reverting this
            if not self.noUpdate:
                self.GetParent().saveState = False
                self.API.tabManager.markAsUnsaved()
            if self.GetParent().projectType == SEAL_PROJECT:
                if self.lastText != self.GetText():
                    self.lastText = self.GetText()
                    self.API.sealParser.run(self.lastText)
                else:
                    self.API.clearInfoArea()
            else:
                self.API.clearInfoArea()

    def getAction(self, event):
        if event != None:
            wx.Yield()
        # Filter unneccessary triggering
        if time.time() - self.lastAutoEdit < 1:
            return
        if self.API.editorSplitter.doSplit == None:
            return
        line = self.GetCurrentLine()
        dialog = None
        if self.API.getStatementType(self.GetLine(line)) <= CONDITION:
            # Get statement and corresponding lines
            self.lastEdit = self.findAllStatement(line)

            # Create new sealStruct instance only for this statement
            seal = SealStruct(self.API, self.lastEdit[0], None)
            predictedType = seal.getPredictedType()
            if predictedType == STATEMENT:
                dialog = EditStatement(self.API.editorSplitter,
                                    self.API, seal, self.statementUpdateClbk)
            elif predictedType == CONDITION:
                dialog = EditCondition(self.API.editorSplitter,
                                    self.API, seal, self.conditionUpdateClbk)
        self.API.editorSplitter.doSplit(dialog)

    def statementUpdateClbk(self, name, value, oldValue = None):
        self.lastAutoEdit = time.time()
        if self.newMode:
            self.SetTargetStart(self.GetCurrentPos())
            self.SetTargetEnd(self.GetCurrentPos())
            self.ReplaceTarget(name + ' ' + value + ';')
            self.lastEdit = self.findAllStatement(self.GetCurrentLine())
            self.newMode = False
        else:
            row = self.lastEdit[0].rstrip()
            if name == 'actuator' or name == 'object':
                row = re.sub("(?i){0}".format(oldValue), value, row)
            elif value == '':
                row = re.sub("(?i), ?{0} {1}".format(name, oldValue), '', row)
            elif value == False:
                row = re.sub("(?i), ?{0}".format(name), '', row)
            elif value == True:
                row = "{0}, {1};".format(row.rstrip("; "), name)
            elif oldValue == None:
                row = "{0}, {1} {2};".format(row.rstrip("; "), name, value)
            else:
                row = re.sub("(?i){0} {1}".format(name, oldValue),
                             "{0} {1}".format(name, value), row)

            self.SetTargetStart(self.PositionFromLine(self.lastEdit[1]))
            self.SetTargetEnd(self.PositionFromLine(self.lastEdit[3] + 1) - 1)
            self.ReplaceTarget(row)
            self.lastEdit = self.findAllStatement(self.lastEdit[2])
        print "#####################################"

    def conditionUpdateClbk(self, name, value, oldValue = None):
        self.lastAutoEdit = time.time()
        if self.newMode:
            self.AddText("when " + value + ":\n\nend")
            self.lastEdit = self.findAllStatement(self.GetCurrentLine())
            self.newMode = False
        else:
            row = self.lastEdit[0].rstrip()
            if name == 'when':
                start = row.find(name)
                end = row[start:].find(":")
            else:
                name = int(name)
                start = 0
                while name > 0:
                    start += row[start:].find("elsewhen") + len("elsewhen")
                    name -= 1
                    print start, name
                end = row[start:].find(":")

            newPart = row[start:start + end].replace(oldValue, value)
            row = row[:start] + newPart + row[start + end:]

            self.SetTargetStart(self.PositionFromLine(self.lastEdit[1]))
            self.SetTargetEnd(self.PositionFromLine(self.lastEdit[3] + 1))
            self.ReplaceTarget(row)
            self.lastEdit = self.findAllStatement(self.lastEdit[2])

    def addStatement(self):
        self.getPlaceForAdding()
        self.lastAutoEdit = time.time()
        dialog = EditStatement(self.API.editorSplitter,
                            self.API, Statement(), self.statementUpdateClbk)
        self.API.editorSplitter.doSplit(dialog)
        self.newMode = True

    def addCondition(self):
        self.getPlaceForAdding()
        self.lastAutoEdit = time.time()
        dialog = EditCondition(self.API.editorSplitter,
                            self.API, ConditionContainer(), self.conditionUpdateClbk)
        self.API.editorSplitter.doSplit(dialog)
        self.newMode = True

    def findAllStatement(self, lineNr):
        startNr = lineNr - 1
        endNr = lineNr
        # Search for comments before current line
        while (self.API.getStatementType(self.GetLine(startNr)) == UNKNOWN
              and startNr > -1 and self.GetLine(startNr) != '\n'):
            startNr -= 1
        startNr += 1

        # Search for end of statement
        # If this is simple statement, then end is @ this line
        if self.API.getStatementType(self.GetLine(lineNr)) == STATEMENT:
            endNr = lineNr
        else:
            endNr += 1
            # We have to search for end, noting that there might be other when's
            whens = 1
            ends = 0
            while whens != ends and endNr < self.GetLineCount():
                if (self.API.getStatementType(self.GetLine(endNr)) == END) :
                    ends += 1
                elif(self.API.getStatementType(self.GetLine(endNr)) == CONDITION):
                    whens += 1

                endNr += 1
            endNr -= 1
        # Get all lines between start and end
        statement = ""
        for x in range(startNr, endNr + 1):
            statement += self.GetLine(x)
        return (statement, startNr, lineNr, endNr)

    def getPlaceForAdding(self):
        self.LineEndDisplay()
        self.AddText("\n\n")
        # Smtng's wrong here...
