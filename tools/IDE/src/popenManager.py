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

from subprocess import Popen, PIPE, STDOUT
import wx

class ResultEvent(wx.PyEvent):
    """Simple event to carry arbitrary result data."""
    def __init__(self, data, EVT_RESULT_ID):
        """Init Result Event."""
        wx.PyEvent.__init__(self)
        self.SetEventType(EVT_RESULT_ID)
        self.data = data

class PopenManager():
    def __init__(self, API, notify_window, EVT_ID, target, name):
        self.API = API
        self._notify_window = notify_window
        self.EVT_ID = EVT_ID
        self.target = target
        self.name = name

    def run(self):
        try:
            self.proc = Popen(self.target, stderr = STDOUT, stdout = PIPE)
            out = self.proc.stdout.readline()
            while out:
                wx.PostEvent(self._notify_window, ResultEvent(out, self.EVT_ID))
                wx.YieldIfNeeded()
                out = self.proc.stdout.readline()
            self.proc.wait()
            wx.PostEvent(self._notify_window, ResultEvent([self.proc.returncode], self.EVT_ID))
        except OSError, e:
                wx.PostEvent(self._notify_window, ResultEvent(\
                "Execution failed in thread '{}', message: {}".format(\
                                                self.name, e), self.EVT_ID))
        finally:
            wx.PostEvent(self._notify_window, ResultEvent(None, self.EVT_ID))
