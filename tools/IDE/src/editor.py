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

from edit_condition import EditCondition
from structures import ComponentUseCase
from component_hierarchy import components
from re import finditer
from globals import * #@UnusedWildImport
import  keyword

class Editor(wx.stc.StyledTextCtrl):
    def __init__(self, parent, API):
        wx.stc.StyledTextCtrl.__init__(self, parent)
        self.API = API
        self.lastText = ''
        self.lastPanel = None
        self.newMode = False

        self.last = dict()
        self.lastCount = 0
        self.lastLine = -1
        self.lastType = ''
        self.lastName = ''
        self.lastLineNr = -1

        self._styles = [None] * 32
        self._free = 1

        self.SetEndAtLastLine(True)
        self.SetIndentationGuides(True)
        self.SetUseAntiAliasing(True)

        # Manage tabs
        self.SetTabWidth(4)
        self.SetTabIndents(True)

        # Set style
        self.StyleSetSpec(wx.stc.STC_STYLE_DEFAULT, "face:%s,size:10" % self.API.fontName)
        self.StyleSetSpec(wx.stc.STC_STYLE_LINENUMBER, "back:green,face:%s,size:10" % self.API.fontName)

        # Set captured events
        self.SetModEventMask(wx.stc.STC_PERFORMED_UNDO | wx.stc.STC_PERFORMED_REDO \
            | wx.stc.STC_MOD_DELETETEXT | wx.stc.STC_MOD_INSERTTEXT | wx.stc.STC_LEXER_START)

        self.Bind(wx.stc.EVT_STC_UPDATEUI, self.doGrammarCheck)

###  From http://www.python-forum.org/pythonforum/viewtopic.php?f=2&t=10065
###  Makes editor a real editor

        self.SetProperty("fold", "1")
        self.SetMargins(0, 0)
        self.SetViewWhiteSpace(False)
        self.SetEdgeMode(wx.stc.STC_EDGE_BACKGROUND)
        self.SetEdgeColumn(78)
        self.SetCaretForeground("blue")

        # setup a margin to hold the fold markers
        self.SetMarginType(2, wx.stc.STC_MARGIN_SYMBOL)
        self.SetMarginMask(2, wx.stc.STC_MASK_FOLDERS)
        self.SetMarginSensitive(2, True)
        self.SetMarginWidth(2, 12)

        # fold markers use square headers
        self.MarkerDefine(wx.stc.STC_MARKNUM_FOLDEROPEN,
            wx.stc.STC_MARK_BOXMINUS, "white", "#808080")
        self.MarkerDefine(wx.stc.STC_MARKNUM_FOLDER,
            wx.stc.STC_MARK_BOXPLUS, "white", "#808080")
        self.MarkerDefine(wx.stc.STC_MARKNUM_FOLDERSUB,
            wx.stc.STC_MARK_VLINE, "white", "#808080")
        self.MarkerDefine(wx.stc.STC_MARKNUM_FOLDERTAIL,
            wx.stc.STC_MARK_LCORNER, "white", "#808080")
        self.MarkerDefine(wx.stc.STC_MARKNUM_FOLDEREND,
            wx.stc.STC_MARK_BOXPLUSCONNECTED, "white", "#808080")
        self.MarkerDefine(wx.stc.STC_MARKNUM_FOLDEROPENMID,
            wx.stc.STC_MARK_BOXMINUSCONNECTED, "white", "#808080")
        self.MarkerDefine(wx.stc.STC_MARKNUM_FOLDERMIDTAIL,
            wx.stc.STC_MARK_TCORNER, "white", "#808080")

        # Default font used for highlighting
        faces = {
            'mono' : self.API.fontName,
            'size' : 10,
            'size2': 8,
            }

        # make some general styles ...
        # global default styles for all languages
        # set default font
        self.StyleSetSpec(wx.stc.STC_STYLE_DEFAULT,
            "face:%(mono)s,size:%(size)d" % faces)
        # set default background color
        backgroundColor = '#F5F5DC'
        self.StyleSetBackground(style = wx.stc.STC_STYLE_DEFAULT,
            back = backgroundColor)
        # reset all to be like the default
        self.StyleClearAll()

        # more global default styles for all languages
        self.StyleSetSpec(wx.stc.STC_STYLE_LINENUMBER,
            "back:#C0C0C0,face:%(mono)s,size:%(size2)d" % faces)
        self.StyleSetSpec(wx.stc.STC_STYLE_CONTROLCHAR,
            "face:%(mono)s" % faces)
        self.StyleSetSpec(wx.stc.STC_STYLE_BRACELIGHT,
            "fore:#FFFFFF,back:#0000FF,bold")
        self.StyleSetSpec(wx.stc.STC_STYLE_BRACEBAD,
            "fore:#000000,back:#FF0000,bold")

        # make the Python styles ...
        # default
        self.StyleSetSpec(wx.stc.STC_P_DEFAULT,
            "fore:#000000,face:%(mono)s,size:%(size)d" % faces)
        # comments
        self.StyleSetSpec(wx.stc.STC_P_COMMENTLINE,
            "fore:#007F00,face:%(mono)s,size:%(size)d" % faces)
        # number
        self.StyleSetSpec(wx.stc.STC_P_NUMBER,
            "fore:#007F7F,size:%(size)d" % faces)
        # string
        self.StyleSetSpec(wx.stc.STC_P_STRING,
            "fore:#7F007F,face:%(mono)s,size:%(size)d" % faces)
        # single quoted string
        self.StyleSetSpec(wx.stc.STC_P_CHARACTER,
            "fore:#7F007F,face:%(mono)s,size:%(size)d" % faces)
        # keyword
        self.StyleSetSpec(wx.stc.STC_P_WORD,
            "fore:#00007F,bold,size:%(size)d" % faces)
        # triple quotes
        self.StyleSetSpec(wx.stc.STC_P_TRIPLE,
            "fore:#7F0000,size:%(size)d" % faces)
        # triple double quotes
        self.StyleSetSpec(wx.stc.STC_P_TRIPLEDOUBLE,
            "fore:#7F0000,size:%(size)d" % faces)
        # class name definition
        self.StyleSetSpec(wx.stc.STC_P_CLASSNAME,
            "fore:#0000FF,bold,underline,size:%(size)d" % faces)
        # function or method name definition
        self.StyleSetSpec(wx.stc.STC_P_DEFNAME,
            "fore:#007F7F,bold,size:%(size)d" % faces)
        # operators
        self.StyleSetSpec(wx.stc.STC_P_OPERATOR,
            "bold,size:%(size)d" % faces)
        # identifiers
        self.StyleSetSpec(wx.stc.STC_P_IDENTIFIER,
            "fore:#000000,face:%(mono)s,size:%(size)d" % faces)
        # comment-blocks
        self.StyleSetSpec(wx.stc.STC_P_COMMENTBLOCK,
            "fore:#7F7F7F,size:%(size)d" % faces)
        # end of line where string is not closed
        self.StyleSetSpec(wx.stc.STC_P_STRINGEOL,
            "fore:#000000,face:%(mono)s,back:#E0C0E0,eol,size:%(size)d"\
                % faces)

        # register some images for use in the AutoComplete box
        self.RegisterImage(1,
            wx.ArtProvider.GetBitmap(wx.ART_TIP, size = (16, 16)))
        self.RegisterImage(2,
            wx.ArtProvider.GetBitmap(wx.ART_NEW, size = (16, 16)))
        self.RegisterImage(3,
            wx.ArtProvider.GetBitmap(wx.ART_COPY, size = (16, 16)))

        # Bind some events ...
        self.Bind(wx.stc.EVT_STC_UPDATEUI, self.onUpdateUI)
        self.Bind(wx.stc.EVT_STC_MARGINCLICK, self.onMarginClick)
        self.Bind(wx.EVT_KEY_UP, self.doGrammarCheck)
        self.Bind(wx.EVT_LEFT_UP, self.doGrammarCheck)

    def onKeyPressed(self, event):
        if self.CallTipActive():
            self.CallTipCancel()
        key = event.GetKeyCode()
        if key == 32 and event.ControlDown():
            pos = self.GetCurrentPos()
            # tips
            if event.ShiftDown():
                self.CallTipSetBackground("yellow")
                self.CallTipShow(pos, 'show tip stuff')
            # code completion (needs more work)
            else:
                kw = keyword.kwlist[:]
                # optionally add more ...
                kw.append("__init__?3")
                # Python sorts are case sensitive
                kw.sort()
                # so this needs to match 
                self.AutoCompSetIgnoreCase(False)
                # registered images are specified with appended "?type"
                for i in range(len(kw)):
                    if kw[i] in keyword.kwlist:
                        kw[i] = kw[i] + "?1"
                self.AutoCompShow(0, " ".join(kw))
        else:
            event.Skip()

    def onUpdateUI(self, evt):
        """update the user interface"""
        # check for matching braces
        braceAtCaret = -1
        braceOpposite = -1
        charBefore = None
        caretPos = self.GetCurrentPos()
        if caretPos > 0:
            charBefore = self.GetCharAt(caretPos - 1)
            styleBefore = self.GetStyleAt(caretPos - 1)
        # check before
        if charBefore and chr(charBefore) in "[]{}()"\
                and styleBefore == wx.stc.STC_P_OPERATOR:
            braceAtCaret = caretPos - 1
        # check after
        if braceAtCaret < 0:
            charAfter = self.GetCharAt(caretPos)
            styleAfter = self.GetStyleAt(caretPos)

            if charAfter and chr(charAfter) in "[]{}()"\
                    and styleAfter == wx.stc.STC_P_OPERATOR:
                braceAtCaret = caretPos
        if braceAtCaret >= 0:
            braceOpposite = self.BraceMatch(braceAtCaret)
        if braceAtCaret != -1  and braceOpposite == -1:
            self.BraceBadLight(braceAtCaret)
        else:
            self.BraceHighlight(braceAtCaret, braceOpposite)

        if self.GetParent().projectType == SEAL_PROJECT:
            self.highlightSealManual()
        #self.doGrammarCheck(evt)

    def onMarginClick(self, evt):
        # fold and unfold as needed
        if evt.GetMargin() == 2:
            if evt.GetShift() and evt.GetControl():
                self.foldAll()
            else:
                lineClicked = self.LineFromPosition(evt.GetPosition())
                if self.GetFoldLevel(lineClicked) & \
                        wx.stc.STC_FOLDLEVELHEADERFLAG:
                    if evt.GetShift():
                        self.SetFoldexpanded(lineClicked, True)
                        self.expand(lineClicked, True, True, 1)
                    elif evt.GetControl():
                        if self.GetFoldexpanded(lineClicked):
                            self.SetFoldexpanded(lineClicked, False)
                            self.expand(lineClicked, False, True, 0)
                        else:
                            self.SetFoldexpanded(lineClicked, True)
                            self.expand(lineClicked, True, True, 100)
                    else:
                        self.ToggleFold(lineClicked)

    def foldAll(self):
        """folding folds, marker - to +"""
        lineCount = self.GetLineCount()
        expanding = True
        # find out if folding or unfolding
        for lineNum in range(lineCount):
            if self.GetFoldLevel(lineNum) & \
                    wx.stc.STC_FOLDLEVELHEADERFLAG:
                expanding = not self.GetFoldexpanded(lineNum)
                break;
        lineNum = 0
        while lineNum < lineCount:
            level = self.GetFoldLevel(lineNum)
            if level & wx.stc.STC_FOLDLEVELHEADERFLAG and \
               (level & wx.stc.STC_FOLDLEVELNUMBERMASK) == \
                    wx.stc.STC_FOLDLEVELBASE:
                if expanding:
                    self.SetFoldexpanded(lineNum, True)
                    lineNum = self.expand(lineNum, True)
                    lineNum = lineNum - 1
                else:
                    lastChild = self.GetLastChild(lineNum, -1)
                    self.SetFoldexpanded(lineNum, False)
                    if lastChild > lineNum:
                        self.HideLines(lineNum + 1, lastChild)
            lineNum = lineNum + 1

    def expand(self, line, doexpand, force = False, visLevels = 0, level = -1):
        """expanding folds, marker + to -"""
        lastChild = self.GetLastChild(line, level)
        line = line + 1
        while line <= lastChild:
            if force:
                if visLevels > 0:
                    self.ShowLines(line, line)
                else:
                    self.HideLines(line, line)
            else:
                if doexpand:
                    self.ShowLines(line, line)
            if level == -1:
                level = self.GetFoldLevel(line)
            if level & wx.stc.STC_FOLDLEVELHEADERFLAG:
                if force:
                    if visLevels > 1:
                        self.SetFoldexpanded(line, True)
                    else:
                        self.SetFoldexpanded(line, False)
                    line = self.expand(line, doexpand, force, visLevels - 1)
                else:
                    if doexpand and self.GetFoldexpanded(line):
                        line = self.expand(line, True, force, visLevels - 1)
                    else:
                        line = self.expand(line, False, force, visLevels - 1)
            else:
                line = line + 1;
        return line

### End of copy-paste code

    def highlightC(self):
        self.SetLexer(wx.stc.STC_LEX_CPP)
        self.SetKeyWords(0, " ".join(C_KEYWORDS))

    def highlightSeal(self):
        self.SetLexer(wx.stc.STC_LEX_PYTHON)
        self.SetKeyWords(0, " ".join(SEAL_KEYWORDS))
        self.highlightSealManual()

    def highlightSealManual(self):
        # TODO: filter out comment lines
        text = self.GetText().lower()
        for x in components:
            for a in list(finditer(" " + x.name.lower(), text)):
                self.StartStyling(a.start() + 1, 0x1F)
                self.SetStyling(len(x.name), wx.stc.STC_P_DEFNAME)

        for x in SEAL_PARAMETERS:
            for a in list(finditer(" " + x.lower() + " ", text)):
                self.StartStyling(a.start() + 1, 0x1F)
                self.SetStyling(len(x), wx.stc.STC_P_TRIPLEDOUBLE)
            for a in list(finditer(" " + x.lower() + ";", text)):
                self.StartStyling(a.start() + 1, 0x1F)
                self.SetStyling(len(x), wx.stc.STC_P_TRIPLEDOUBLE)
            for a in list(finditer(" " + x.lower() + ",", text)):
                self.StartStyling(a.start() + 1, 0x1F)
                self.SetStyling(len(x), wx.stc.STC_P_TRIPLEDOUBLE)

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
        if event is not None:
            event.Skip()
        # Mark that file has possibly changed
        self.GetParent().yieldChanges()
        # Update line numbers, needed if digit count changed, margin width changes
        if len(str(self.lastText.count("\n") + 1)) != \
           len(str(self.GetText().count("\n") + 1)):
            self.setLineNumbers()

        if self.GetParent().projectType == SEAL_PROJECT:
            if self.lastText != self.GetText():
                self.lastText = self.GetText()
                self.API.sealParser.run(self.lastText)
                self.lineTracking = self.API.sealParser.lineTracking
            if self.lastLineNr != self.GetCurLine():
                self.lastLineNr = self.GetCurLine()
                self.getAction(event)

    def getAction(self, event):
        if self.newMode:
            return
        # Get statement and corresponding lines
        self.lastEdit = self.findAllStatement(self.GetCurrentLine())

        if self.lastEdit['statementStruct'] != None:
            if self.lastEdit['type'] == STATEMENT:
                self.API.editWindow.updateStatement(self.lastEdit, self.statementUpdateClbk)
            elif self.lastEdit['type'] == CONDITION:
                EditCondition(self.API.editPanel,
                        self.API, self.lastEdit, self.conditionUpdateClbk)

                self.API.editWindow.updateStatement(self.lastEdit)

    def statementUpdateClbk(self, newStatement):
        if self.newMode:
            self.SetTargetStart(self.GetCurrentPos())
            self.SetTargetEnd(self.GetCurrentPos())
            self.ReplaceTarget(newStatement['statementStruct'].getCode(0))
            self.lastEdit = self.findAllStatement(self.GetCurrentLine())
            self.newMode = False
        else:
            self.SetTargetStart(self.PositionFromLine(newStatement['firstLine']))
            endPos = self.PositionFromLine(newStatement['lastLine'] + 1)
            if self.GetCharAt(endPos) == '\n':
                self.SetTargetEnd(endPos - 1)
                self.ReplaceTarget(newStatement['statementStruct'].getCode(0))
            else:
                self.SetTargetEnd(endPos)
                self.ReplaceTarget(newStatement['statementStruct'].getCode(0) + '\n')
            self.lastEdit = self.findAllStatement(newStatement['currentLine'])
        self.doGrammarCheck(None)

    def conditionUpdateClbk(self, name, value):
        if self.newMode:
            self.AddText("when " + value + ":\nend")
            self.doGrammarCheck(None)
            self.lastEdit = self.findAllStatement(self.GetCurrentLine())
            self.newMode = False
        else:
            # Attribute error here because of seal failing on unknown condition
            row = self.lastEdit['statementStruct'].getCode(0).rstrip()
            if name == 'when':
                start = row.find("when") + len("when")
                end = row[start:].find(":")
            else:
                name = int(name)
                start = 0
                while name > 0:
                    start += row[start:].find("elsewhen") + len("elsewhen")
                    name -= 1
                end = row[start:].find(":")
            # +1 is space after keyword
            row = "{} {}{}".format(row[:start], value, row[start + end:])
            self.SetTargetStart(self.PositionFromLine(self.lastEdit['firstLine']))
            self.SetTargetEnd(self.PositionFromLine(self.lastEdit['lastLine']) - 1)
            self.ReplaceTarget(row)
            # No need to recheck for condition, because no new lines are added
            # and no lines have been deleted.
            #self.lastEdit = self.findAllStatement(self.lastEdit[2])

    def addStatement(self):
        self.getPlaceForAdding()
        self.lastAutoEdit = time.time()
        self.newMode = True
        res = {
               "statementStruct": ComponentUseCase('', '', [], None),
               "firstLine":-1,
               "currentLine":-1,
               "lastLine":-1,
               "type":-1
               }
        self.API.editWindow.updateStatement(res, self.statementUpdateClbk)

    def addCondition(self):
        self.getPlaceForAdding()
        self.lastAutoEdit = time.time()
        self.newMode = True
        self.clearUpDialogs(EditCondition(self.API.editPanel,
                               self.API, None, self.conditionUpdateClbk), True)

    def findAllStatement(self, lineNr):
        res = {
               "statementStruct": None,
               "firstLine":-1,
               "currentLine": lineNr,
               "lastLine":-1,
               "type":-1
               }
        # Check for empty line
        line = self.GetLine(lineNr).strip()
        if line == '':
            return res
        # Try find statement in this line
        for x in self.lineTracking["Statement"]:
            if lineNr + 1 >= x[0] and lineNr + 1 <= x[1]:
                res['statementStruct'] = x[2]
                res['firstLine'] = x[0] - 1
                res['lastLine'] = x[1] - 1
                res['type'] = STATEMENT
                return res

        # If no component error is throwed we dont get so far :(
        statementType, actuator, obj = self.API.getStatementType(line)

        if statementType == STATEMENT:
            res['statementStruct'] = ComponentUseCase(actuator, obj, [], None)
            res['firstLine'] = lineNr
            res['lastLine'] = lineNr
            res['type'] = STATEMENT
            return res

        for x in self.lineTracking["Condition"]:
            if lineNr + 1 >= x[0] and lineNr + 1 <= x[1]:
                res['statementStruct'] = x[2]
                res['firstLine'] = x[0] - 1
                res['lastLine'] = x[1]
                res['type'] = CONDITION
                return res

        # Try find condition in this line
        # TODO

        return res

    def getPlaceForAdding(self):
        self.LineEndDisplay()
        self.AddText("\n\n")
        #TODO: Smtng's wrong here...

