import wx
import subprocess
import serial
import time

class ListenModule(wx.Dialog):
    def __init__(self, parent, title, API):
        super(ListenModule, self).__init__(parent = parent, 
            title = title, size = (500, 400), style=wx.DEFAULT_DIALOG_STYLE | wx.RESIZE_BORDER)
        self.API = API
        # Just a shorter name
        self.tr = self.API.translater.translate
        self.haveMote = False
        self.listening = False
        self.baudrate = 38400
        self.serialPort = '/dev/ttyUSB0'
        
        self.main = wx.BoxSizer(wx.VERTICAL)
        self.listenControls = wx.BoxSizer(wx.HORIZONTAL)
        
        self.output = wx.TextCtrl(self, style = wx.EXPAND | wx.ALL, size = (500, 400))
        self.output.SetBackgroundColour("Black")
        self.output.SetForegroundColour("White")
        
        self.ports = wx.ComboBox(self, choices = [], size = (300, -1))
        self.clear = wx.Button(self, label=self.tr("Start listening"))
        self.refresh = wx.Button(self, label=self.tr("Refresh"))
        
        self.listenControls.Add(self.ports)
        self.listenControls.Add(self.refresh)
        self.listenControls.Add(self.clear)
        
        self.main.Add(self.listenControls, 
                      0,            # make vertically unstretchable
                      wx.EXPAND |   # make horizontally stretchable
                      wx.ALL,       #   and make border all around
                      3 );
        self.main.Add(self.output, 
                      1,            # make vertically stretchable
                      wx.EXPAND |   # make horizontally stretchable
                      wx.ALL,       #   and make border all around
                      10 );         # set border width to 10)
        
        self.getMotelist()
        
        self.Bind(wx.EVT_BUTTON, self.doClear, self.clear)
        self.Bind(wx.EVT_BUTTON, self.getMotelist, self.refresh)
        self.Bind(wx.EVT_COMBOBOX, self.changeTarget, self.ports)

        self.SetSizer(self.main)
        self.main.Fit(self)
        self.Show()

    def doClear(self, event = None):
        global BAUDRATE
        global serialPort

        self.listening = not self.listening
        if self.listening:
            self.clear.SetLabel(self.tr('Stop listening'))
        else:
            self.clear.SetLabel(self.tr('Start listening'))
        # Redraw button if size have changed
        self.clear.SetSize(self.clear.GetEffectiveMinSize())
        
        if self.listening:
            if self.listenSerialPort() == False:
                self.updateStatus(self.tr("Error conecting to device")+ '\n', False)
                self.doClear(None)


    def listenSerialPort(self):
        try:
            ser = serial.Serial(self.serialPort, self.baudrate, timeout=0, 
                               parity=serial.PARITY_NONE, rtscts=1)
            while self.listening:
                s = ser.read(1000)
                if len(s) > 0:
                    self.updateStatus(s, False)
                # Keep interface alive, long sleep makes it to delay responses.
                # Listen once every .2 sec
                for _ in range(0, 200):
                    wx.Yield()
                    time.sleep(0.001)
            ser.close()
            return True
        except serial.SerialException, ( msg ):
            print "\nSerial exception:\n\t", msg
            return False


    def getMotelist(self, event = None):
        motelist = []
        # get motelist output as string... |-(
        process = subprocess.Popen([self.API.path + "/../../mos/make/scripts/motelist"], 
                                      stderr = subprocess.STDOUT,
                                      stdout = subprocess.PIPE)
        motes, err = process.communicate()
        
        # remove table header and split into lines, ("-----") + 6 used to be 
        # sure it's end of header, not random symbol in data
        self.ports.Clear()
        if motes.find("No devices found") == False:
            self.haveMote = False
            self.ports.SetValue(self.tr("No devices found"))
            return None
        
        for x in motes[motes.rfind("-----") + 6: ].split("\n"):
            # seperate ID, port, description
            y = x.split(None, 2)
            if y != []:
                motelist.append(y)
        for x in motelist:
            if len(x) > 2:
                self.ports.Append(x[1]+" - "+ x[2])
        self.haveMote = True
        self.ports.SetValue(self.tr("Use default device"))
        self.serialPort = motelist[0][1]
        return motelist
    
    def changeTarget(self, event):
        global serialPort
        target = event.GetEventObject().GetValue()
        serialPort = target[:target.find(" ")]
        self.serialPortChanged = True

    def updateStatus(self, message, overwrite = True):
        if overwrite:
            self.output.SetValue(message)
        else:
            self.output.SetValue(message + self.output.GetValue())
        wx.Yield()