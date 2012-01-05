import editStatement

class MainModule:
    
    def __init__(self, graphics, API):
        # get API core class
        self.API = API;
        self.graphics = graphics
        self.use = {}
        self.read = {}
        self.sink = {}
        self.initUse()
        self.initRead()
        self.initSink()
        self.graphics.redrawAll()
        
    def initUse(self):
        #print "useInit start... "
        for obj in self.API.getObjects("use"):
            self.use[obj] = self.graphics.addUse(obj, self.useBtnClk)
            #print "  Added "+obj
        #print "done!"
        #print self.use
        
    
    def initRead(self):
        #print "readInit start... "
        for obj in self.API.getObjects("read"):
            self.read[obj] = self.graphics.addRead(obj, self.readBtnClk)
            #print "  Added "+obj
        #print "done!"
    
    
    def initSink(self):
        #print "sinkInit start... "
        for obj in self.API.getObjects("output"):
            self.sink[obj] = self.graphics.addSink(obj, self.sinkBtnClk)
            #print "  Added "+obj
        #print "done!"
        
    def useBtnClk(self, event):
        #print "Button 'Edit "+event.GetEventObject().GetName()+"' clicked! "

        self.dialog = editDialog.editDialog(None, 'Edit ' + 
                                              event.GetEventObject().GetName(), 
                                              self.API.getParams('use'),
                                              self.useSaveDialog,
                                              event.GetEventObject().GetName(),
                                              self.API.useObjects[event.GetEventObject().GetName()])
        self.dialog.ShowModal()
        self.dialog.Destroy()
        
    def useSaveDialog(self, save, data, keyWord):
        if (save == True):
            self.API.keyWords['use']['obj'][keyWord] = data
            self.use[keyWord].SetValue(self.API.generateUseCmd(keyWord))
        self.dialog.Destroy()
        
    def readBtnClk(self, event):
        #print "Button 'Edit "+event.GetEventObject().GetName()+"' clicked! "
        #print self.API.readObjects
        self.dialog = editDialog.editDialog(None, 'Edit ' + 
                                              event.GetEventObject().GetName(), 
                                              self.API.getParams('read'),
                                              self.readSaveDialog,
                                              event.GetEventObject().GetName(),
                                              self.API.readObjects[event.GetEventObject().GetName()])
        self.dialog.ShowModal()
        self.dialog.Destroy()
        
    def readSaveDialog(self, save, data, keyWord):
        if (save == True):
            self.API.keyWords['read']['obj'][keyWord] = data
            self.read[keyWord].SetValue(self.API.generateReadCmd(keyWord))
        self.dialog.Destroy()
        
    def sinkBtnClk(self, event):
        #print "Button 'Edit "+event.GetEventObject().GetName()+"' clicked! "

        self.dialog = editDialog.editDialog(None, 'Edit ' + 
                                              event.GetEventObject().GetName(), 
                                              self.API.getParams('output'),
                                              self.sinkSaveDialog,
                                              event.GetEventObject().GetName(),
                                              self.API.sinkObjects[event.GetEventObject().GetName()])
        self.dialog.ShowModal()
        self.dialog.Destroy()
        
    def sinkSaveDialog(self, save, data, keyWord):
        if (save == True):
            self.API.keyWords['output']['obj'][keyWord] = data
            self.sink[keyWord].SetValue(self.API.generateSinkCmd(keyWord))
        self.dialog.Destroy()
