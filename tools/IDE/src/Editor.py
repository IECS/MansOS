'''
Created on 2011. gada 19. dec.

@author: Janis Judvaitis
'''

import wx.stc
from math import sqrt
import sealStruct 
import editStatement
import editCondition

class CodeEditor(wx.stc.StyledTextCtrl):

    def __init__(self, parent, API, noUpdate = False):
        wx.stc.StyledTextCtrl.__init__(self, parent)
        #self.SetText("\n")
        self.activePoints = []
        self.activeRadius = 0
        self.API = API
        self.spaces = ''
        self.noUpdate = noUpdate
        
        # Set scroll bar range
        self.SetEndAtLastLine(True)
        
        # Set style
        font = wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT)
        self.StyleSetSpec(wx.stc.STC_STYLE_DEFAULT, "face:%s,size:10" % font.GetFaceName())
        self.StyleSetSpec(wx.stc.STC_STYLE_LINENUMBER, "back:green,face:%s,size:10" % font.GetFaceName())
        
        # Set captured events
        self.SetModEventMask(wx.stc.STC_PERFORMED_UNDO | wx.stc.STC_PERFORMED_REDO | wx.stc.STC_MOD_DELETETEXT | wx.stc.STC_MOD_INSERTTEXT | wx.stc.STC_LEXER_START)
        
        #self.UsePopUp(0)
        self.SetUseAntiAliasing(True)
        # On each repaint we add buttons on left side
        self.Bind(wx.stc.EVT_STC_PAINTED, self.on_paint)
        # Hack for really redrawing all window not part of it, which causes circles to be incorrect
        self.Bind(wx.stc.EVT_STC_CHANGE, self.doRefresh)
        # Bind button clicks
        self.Bind(wx.stc.EVT_STC_MARGINCLICK, self.getAction)
        self.Bind(wx.EVT_MOTION, self.manageMouse)
        # Add margin for buttons
        self.SetMarginType(1, wx.stc.STC_MARGIN_SYMBOL )
        self.SetMarginWidth(1, 10)
        self.SetMarginSensitive(1, True)           #this one needs to be mouse-aware
        
    def setLineNumbers(self, enable = True):
        if enable:
            width = len(str(self.GetLineCount()))
            self.SetMarginType(0, wx.stc.STC_MARGIN_NUMBER )
            self.SetMarginWidth(0, self.TextWidth(wx.stc.STC_STYLE_LINENUMBER, (width + 1) * '0'))
        else:
            self.SetMarginWidth(0, 0)

    def on_paint(self, event):
        self.activePoints = []
        dc = wx.PaintDC(self)
        # Get line number max width in pixels
        width = self.TextWidth(wx.stc.STC_STYLE_LINENUMBER,(len(str(self.GetLineCount()))+ 1) * '0')
        # Get line height in pixels
        lineH = (self.PointFromPosition(self.PositionFromLine(1)) - self.PointFromPosition(self.PositionFromLine(0)))[1]
        # Fix for empty document not adding second margin
        if lineH == 0:
            lineH = width
        # Adjust margins to current zoom
        self.SetMarginWidth(0, width)
        self.SetMarginWidth(1, lineH)
        #print width, lineH
        # Set x coordinate for Button Margin(1)
        x = width + lineH/2
        #print self.GetLineCount()
        # Cycle each line and add button if necessary
        for i in range(0, self.GetLineCount()):
            if self.API.getStatementType(self.GetLine(i)) < self.API.CONDITION_CONTINUE:
                #print "Added circle"
                dc.SetPen(wx.Pen("BLACK", 1))
                #print i,"=>",self.PointFromPosition(self.PositionFromLine(i-1)), self.GetLine(i-1)
                y = self.PointFromPosition(self.PositionFromLine(i))[1] + lineH/2
                # Draw Circle
                dc.DrawCirclePoint((x,y),lineH/3) 
                # Draw +
                dc.SetPen(wx.Pen("BLUE", 1))
                dc.DrawLine(x-lineH/4, y, x + lineH/4, y)
                dc.DrawLine(x, y-lineH/4, x, y + lineH/4)
                self.activePoints.append(((x, y), i))
        self.activeRadius = lineH/2
        #print lineH
        #print self.activePoints
    
    # Hack for really redrawing all window not part of it, which causes circles to be incorrect
    def doRefresh(self, event):
        self.Refresh(True)
        # Mark that file has changed
        if self.noUpdate == False:
            self.GetParent().saveState = False
            self.GetGrandParent().markAsUnsaved()
        
    def getAction(self, event):
        line = self.LineFromPosition(event.GetPosition())
        if self.API.getStatementType(self.GetLine(line)) < self.API.CONDITION_CONTINUE:
            # Get statement and corresponding lines
            statement = self.findAllStatement(line)
            # Get prefix(whitespaces), so output is correctly tabbed
            selectedLine = self.GetLine(statement[2])
            whitespaceCount = len(selectedLine) - len(selectedLine.lstrip())
            self.spaces = selectedLine[:whitespaceCount]
            
            self.SetSelection(self.PositionFromLine(statement[1]), self.PositionFromLine(statement[3] + 1))
            # Create new sealStruct instance only for this statement
            seal = sealStruct.Seal(self.API, statement[0])
            data = seal.getStruct()
            if seal.generateAllStatements() == "":
                #print "Condition"
                data = data['conditions'][0]
                self.disableRedraw = True
                self.dialog = editCondition.editDialog(self.GetGrandParent(), 
                                    self.API, data, self.conditionDialogClbk)
                self.dialog.ShowModal()
                self.dialog.Destroy()
                self.disableRedraw = False
            elif seal.generateAllConditions() == "":
                #print "Statement"
                data = data['statements'][0]
                self.dialog = editStatement.editDialog(None, 
                                    self.API, data, self.statementDialogClbk)
                self.dialog.ShowModal()
                self.dialog.Destroy()
            else:
                print "Weird..."
            #print seal.generateAllCode()
            #print data.getAll()
            
    def statementDialogClbk(self, action, statement):
        if action == True:
            self.ReplaceSelection(self.spaces + statement.getAll() + '\n')
        #print statement.getAll()
        self.dialog.Destroy()
        self.Enable()
    
    def conditionDialogClbk(self, action, statement):
        if action == True:
            self.ReplaceSelection(statement.getAll(self.spaces) + '\n')
        #print statement.getAll()
        self.dialog.Destroy()
        self.Enable()
        
    def addStatement(self):
        self.getPlaceForAdding()
        self.dialog = editStatement.editDialog(None, 
                             self.API, '', self.statementDialogClbk)
        self.dialog.ShowModal()
        self.dialog.Destroy()
    
    def addCondition(self):
        self.getPlaceForAdding()
        self.dialog = editCondition.editDialog(None, 
                            self.API, '', self.statementDialogClbk)
        self.dialog.ShowModal()
        self.dialog.Destroy()
    
    def calcDist(self, point1, point2):
        # Pitagor theorem for distance calculating between two points
        a = abs(point1[0] - point2[0])
        b = abs(point1[1] - point2[1])
        c = sqrt(a ** 2 + b ** 2)
        #print a,b,c
        return c
    
    def manageMouse(self, event):
        #self.doRefresh(None)
        x = event.GetPositionTuple()[0]
        # Guess we have to do this manually if we wan't to change something
        if x < self.GetMarginWidth(0):
            # For line numbers
            self.SetCursor(wx.StockCursor(wx.CURSOR_RIGHT_ARROW))
        elif x < self.GetMarginWidth(0) + self.GetMarginWidth(1):
            # Cycle all buttons checking if someone is under cursor
            for a in self.activePoints:
                if self.calcDist(event.GetPositionTuple(), a[0]) < self.activeRadius:
                    # Found button under cursor
                    self.SetCursor(wx.StockCursor(wx.CURSOR_HAND))
                    break
                else:
                    self.SetCursor(wx.StockCursor(wx.CURSOR_DEFAULT))
        else:
            # Here we must be on top of text
            self.SetCursor(wx.StockCursor(wx.CURSOR_CHAR))
        # Allow other functions binded to this event to be called
        event.Skip(True)
            
    def findAllStatement(self, lineNr):
        startNr = lineNr - 1
        endNr = lineNr
        # Search for comments before current line
        while (self.API.getStatementType(self.GetLine(startNr)) == self.API.IGNORE
              and startNr > -1 and self.GetLine(startNr) != '\n'):
            startNr -= 1
        startNr += 1
        
        # Search for end of statement
        # If this is simple statement, then end is @ this line
        if self.API.getStatementType(self.GetLine(lineNr)) == self.API.STATEMENT:
            endNr = lineNr
        else:
            # We have to search for end
            while (self.API.getStatementType(self.GetLine(endNr)) 
                   != self.API.CONDITION_END and endNr < self.GetLineCount()):
                endNr += 1
        # Get all lines between start and end
        statement = ""
        for x in range(startNr, endNr + 1):
            statement += self.GetLine(x)
        return (statement, startNr, lineNr,endNr)

    def getPlaceForAdding(self):
        self.LineEndDisplay()
        if self.GetCurLine()[0] == '\n':
            self.AddText("\n")
        else:
            self.AddText("\n\n")
        
