'''
Created on 2012. gada 24. febr.

@author: Janis Judvaitis
'''
import threading
import time
import globals as g

class threadRunner ( threading.Thread ):
    def __init__(self, function, data, API):
        threading.Thread.__init__(self)
        self.function = function
        # If mutable value passed it works as reference, so we can pass values back
        self.data = data
        self.API = API
              
    def run (self):
        locTime = time.time()
        try:
            self.data += self.function()
        except TypeError:
            self.API.logMsg(g.WARNING, "In thread running "+ str(self.function) + "type mismatch happened!")
            self.data += [False, "Type error!"]
            print self.function()
        self.data += [time.time() - locTime]
        
