import editDialog

class ConditionalModule:
    def __init__(self, graphics, API, changeTitle):
        # get API core class
        self.API = API;
        self.graphics = graphics
        self.whenActions = []
        self.elseActions = []
        self.condition = self.API.addCondition()
        self.condition['when'] = []
        self.condition['else'] = []
        self.editedObj = None
        self.changeTitle = changeTitle
        self.graphics.addStatementField(self.API.getDefaultConditions(), 
                                        self.whenChoosed)

    def whenChoosed(self, event):
        if len(self.whenActions) == 0:
            self.addWhenAction()
            self.addElseAction()
        self.condition['condition'] = event.GetEventObject().GetValue()
        self.changeTitle("When " + event.GetEventObject().GetValue() + ":")

    def addWhenAction(self):
        self.whenActions.append(self.graphics.addWhenAction(
                                                self.boxClbk, 
                                                self.fieldClbk, 
                                                self.buttonClbk, 
                                                self.API.getActionKeyWords()))
        self.condition['when'].append([])
    
    def addElseAction(self):
        self.elseActions.append(self.graphics.addElseAction(
                                                self.boxClbk, 
                                                self.fieldClbk, 
                                                self.buttonClbk, 
                                                self.API.getActionKeyWords()))
        self.condition['else'].append([])
        
    def boxClbk(self, event):
        print event.GetEventObject().GetValue()+" selected!"  
        if event.GetEventObject().GetName() == "when":
            # If last row in this section then we add another row
            if self.whenActions[-1][0] == event.GetEventObject():
                self.addWhenAction()
        else:
            # If last row in this section then we add another row
            if self.elseActions[-1][0] == event.GetEventObject():
                self.addElseAction()
        field = self.getActionData(event.GetEventObject())[0][2]
        field.SetValue(event.GetEventObject().GetValue() + ";")
        self.editedObjData = self.getActionData(event.GetEventObject())
        self.condition[self.editedObjData[3]][self.editedObjData[2]] = [None,event.GetEventObject().GetValue()]
        
    def buttonClbk(self, event):
        print "Button 'Edit "+event.GetEventObject().GetName()+"' clicked! "  
        self.editedObjData = self.getActionData(event.GetEventObject())
        if self.editedObjData[1] not in self.API.keyWords:
            print "No action choosed, ignore!"
            return
        #passedData = None
        #if self.condition[self.editedObjData[3]][self.editedObjData[2]] != []:
        passedData = self.condition[self.editedObjData[3]][self.editedObjData[2]][0]
        self.dialog = editDialog.editDialog(None, 'Edit ' + 
                                              event.GetEventObject().GetName(), 
                                              self.API.keyWords[self.editedObjData[1]]['param'],
                                              self.dialogSave,
                                              event.GetEventObject().GetName(),
                                              passedData)
        self.dialog.ShowModal()
        self.dialog.Destroy()
        
    def fieldClbk(self, event):
        print "Field '"+event.GetEventObject().GetValue()+"' edited! "

        self.dialog = editDialog.editDialog(None, 'Edit ' + 
                                              event.GetEventObject().GetName(), 
                                              self.API.keyWords['use']['param'],
                                              self.dialogSave,
                                              event.GetEventObject().GetName())
        self.dialog.ShowModal()
        self.dialog.Destroy()

    def dialogSave(self, save, data, name):
        print data
        print name
        if (save == True):
            self.condition[self.editedObjData[3]][self.editedObjData[2]][0] = data
            self.editedObjData[0][2].SetValue(self.API.generateCmd(data, self.editedObjData[0][0].GetValue()))
        self.dialog.Destroy()
        self.API.printConditions()
        
    def getActionData(self, obj):
        # Find all info we might need about object
        for x,y in enumerate(self.whenActions):
            if obj in y:
                return [self.whenActions[x], 
                        y[0].GetValue()[:y[0].GetValue().find(" ")],
                        x, 'when']
        for x,y in enumerate(self.elseActions):
            if obj in y:
                return [self.elseActions[x], 
                        y[0].GetValue()[:y[0].GetValue().find(" ")],
                        x, 'else']
        return None