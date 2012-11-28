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

from multiprocessing import Pipe
import threading
import wx
from time import time

class ResultEvent(wx.PyEvent):
    """Simple event to carry arbitrary result data."""
    def __init__(self, data, EVT_RESULT_ID):
        """Init Result Event."""
        wx.PyEvent.__init__(self)
        self.SetEventType(EVT_RESULT_ID)
        self.data = data

class MyThread():
    def __init__(self, target = None, args = None, callbackFunction = None,
                 printToInfo = False, printToListen = False, name = '',
                 printFunction = None):
        # Initial values
        self.target = target
        self.args = args
        self.callbackFunction = callbackFunction
        self.printToInfo = printToInfo
        self.printToListen = printToListen
        self.printFunction = printFunction
        self.name = name if name != '' else str(callbackFunction)

        # Background values
        self.EVT_ID = -1
        self.output = str()
        self.notifyWindow = None
        self.process = None
        # This is used to terminate thread, simply set to True.
        self.stop = False
        # Stop signal time, process have 0.5s to stop or it will be terminated.
        self.stopSignalTime = 0

    # Check that all needed values are acceptable for thread to run!
    def isRunnable(self):
        return (self.target is not None and self.EVT_ID != -1 and not self.stop \
                and self.notifyWindow is not None and self.process is None)

    # Run this thread
    def run(self):
        if not self.isRunnable():
            print "Can't run thread '{}'".format(self.name)
            print self.target, self.EVT_ID, self.notifyWindow, self.process
            return
        try:
            parentPipe, childPipe = Pipe()
            #self.process = Process(target = self.target, args = (childPipe, self.args))
            self.process = threading.Thread(target = self.target,
               name = "Serial communication thread",
               kwargs = {"pipe": childPipe,
                         "args": self.args})
            self.process.name = self.name
            self.process.daemon = True
            self.process.start()
            while self.process.isAlive() or parentPipe.poll(0.001):
                if parentPipe.poll(0.001):
                    out = parentPipe.recv()
                    wx.PostEvent(self.notifyWindow, ResultEvent(out, self.EVT_ID))
                if self.stop:
                    if self.stopSignalTime == 0:
                        self.stopSignalTime = time() + 0.5
                        parentPipe.send([False])
                    if time() > self.stopSignalTime:
                        self.process._Thread__stop()
                wx.YieldIfNeeded()
                #sleep(0.01)
        except OSError, e:
                wx.PostEvent(self.notifyWindow, ResultEvent(\
                "Execution failed in thread '{}', message: {}".format(\
                                                self.name, e), self.EVT_ID))
        finally:
            wx.PostEvent(self.notifyWindow, ResultEvent(None, self.EVT_ID))

