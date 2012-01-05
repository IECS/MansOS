import wx
import translater
import Parameter
import Statement

class editDialog(wx.Dialog):
    
    def __init__(self, parent, title, API, statement, saveCallback):
        super(editDialog, self).__init__(parent = parent, 
            size = (500, 400),
            style=wx.DEFAULT_DIALOG_STYLE | wx.RESIZE_BORDER)
        self.saveCallback = saveCallback
        self.API = API
        self.main = wx.BoxSizer(wx.VERTICAL)
        self.data = wx.GridBagSizer(hgap = 2, vgap = 1)
        #self.main.Add(wx.StaticText(self, label = "Edit " + statement.getTypeAndObject()))
        self.main.AddSpacer((10, 10))
        self.main.Add(self.data, 0)
        self.main.AddSpacer((10, 10))
        self.choices = []
        self.text = []
        self.statement = statement
        definition = self.API.getActuatorInfo(statement.getType())
        data = definition['parameters']
        
        # Generate all
        self.generateActuatorSelect(statement)
        self.generatePatameterSelects(data, statement)
        
        self.buttonPane = wx.BoxSizer(wx.HORIZONTAL)
        self.save = wx.Button(self, label = "Save", size = (150, -1), name = "save")
        self.Bind(wx.EVT_BUTTON, self.saveClk, self.save)
        self.buttonPane.Add(self.save,
                      1,            # make vertically stretchable
                      wx.EXPAND |    # make horizontally stretchable
                      wx.ALL,        #   and make border all around
                      1 );         # set border width to 10))
        self.close = wx.Button(self, label = "Close", size = (150, -1), name = "close")
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
                                            self.obj.GetValue(),
                                            [],
                                            self.statement.getComments())
            for x in self.choices:
                print x.GetName()
                if x.GetValue() != '':
                    statement.addParameter(Parameter.Parameter(x.GetName(), x.GetValue()))
                else:
                    pass
                    #print x.GetName()," wasn't selected"
            self.saveCallback(True, statement)
        else:
            self.saveCallback(False, None)

    def generateActuatorSelect(self, statement):
        # Generate all objects
        self.actuatorText = wx.StaticText(self, label = translater.translate("Edit actuator:"))
        self.objText = wx.StaticText(self, label = translater.translate("Edit object:"))
        self.actuator = wx.ComboBox(self, choices = self.API.getAllStatementActuators(), 
                                style = wx.CB_DROPDOWN, name = "actuator")
        self.Bind(wx.EVT_COMBOBOX, self.onActuatorChange, self.actuator)
        self.Bind(wx.EVT_TEXT, self.onActuatorChange, self.actuator)
        self.actuator.SetValue(statement.getType())
        self.obj = wx.ComboBox(self, choices = self.API.getActuatorInfo(statement.getType())['objects'], 
                                style = wx.CB_DROPDOWN, name = "object")
        # Only for title change,
        self.Bind(wx.EVT_COMBOBOX, self.updateTitle, self.obj)
        self.Bind(wx.EVT_TEXT, self.updateTitle, self.obj)
        self.obj.SetValue(statement.getObject())
        # Add them to layout
        self.data.Add(self.actuatorText, pos = (0, 0))
        self.data.Add(self.actuator, pos = (0, 1))
        self.data.Add(self.objText, pos = (1, 0))
        self.data.Add(self.obj, pos = (1, 1))
        # Set used row count
        self.row = 2
    
    def generatePatameterSelects(self, data, statement):
        # Cycle all parameters and draw according boxes
        for parameter in data:
            self.text.append(wx.StaticText(self, label = translater.translate(parameter.getName()) + ":"))
            self.data.Add(self.text[-1], pos = (self.row,0))
            if parameter.getValue() != None:
                self.choices.append(wx.ComboBox(self, choices = parameter.getValue(), 
                                                style = wx.CB_DROPDOWN, name = parameter.getName()))
            else:
                self.choices.append(wx.CheckBox(self, name = parameter.getName()))
            #print values
            if statement.getParamValueByName(parameter.getName()) != -1:
                # This should be None not '', can't find why it actually is ''
                if statement.getParamValueByName(parameter.getName()) == '':
                    self.choices[-1].SetValue(True)
                else:
                    self.choices[-1].SetValue(statement.getParamValueByName(parameter.getName()))
            self.data.Add(self.choices[-1], pos = (self.row, 1))
            self.row += 1
    
    def onActuatorChange(self, event):
        actuator = self.actuator.GetValue()
        self.updateTitle()
        # Don't clear if this is already selected
        if actuator == self.statement.getType():
            return
        allActuators = self.API.getActuatorInfo(actuator)
        if allActuators != None:
            # Generate new statement
            statement = Statement.Statement(actuator,
                                            self.obj.GetValue(),
                                            [],
                                            self.statement.getComments())
            self.statement = statement
            definition = self.API.getActuatorInfo(statement.getType())
            data = definition['parameters']
            # Clear All
            self.clearAll()
            self.generateActuatorSelect(statement)
            self.generatePatameterSelects(data, statement)
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
        self.SetTitle("Edit \"" + self.actuator.GetValue() +" " + self.obj.GetValue() + "\"")