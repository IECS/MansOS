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


import os
from re import compile
from subprocess import Popen, PIPE, STDOUT

class GetMotelist(object):

    def __init__(self, pathToMansos):
        self.pathToMansos = pathToMansos
        self.shellRegex = compile(
                        r"Mote type: \".\" \((.*)\)\n PAN address: \"(.*)\"")
        
    def getShellMotelist(self):
        motelist = []
        oldPath = os.getcwd()
        try:
            os.chdir(self.pathToMansos + "/tools/shell/")
            # Check for shell to be build
            if not os.path.exists("./shell") or not os.path.isfile("./shell"):
                make = Popen(["make"], stdin = PIPE, 
                           stderr = STDOUT, stdout = PIPE)
                out = make.communicate(input = "")[0]
                
            upload = Popen(["./shell"], stdin = PIPE, 
                           stderr = STDOUT, stdout = PIPE)
            out = upload.communicate(input = "ls")[0]

            os.chdir(oldPath)
            if out.find("timeout") == -1:
                res = self.shellRegex.search(out)
                while res != None:
                    motelist.append([res.group(2), "Shell", res.group(1)])
                    out = out[res.end():]
                    res = self.shellRegex.search(out)
                return [True, motelist]
            else:
                return [False, '']
            
        except OSError, e:
            print "execution failed:", e
            os.chdir(oldPath)
            return [False, e]
    
    def getMotelist(self):
        motelist = []
        try:
            # Get motelist output as string... |-(
            process = Popen([self.pathToMansos + "/mos/make/scripts/motelist", "-c"], 
                                          stderr = STDOUT,
                                          stdout = PIPE)
            motes = process.communicate()[0]
            
            if motes.find("No devices found") == False:
                return [False, '']
            
            for line in motes.split("\n"):
                # Seperate ID, port, description
                data = line.split(',')
                if data != ['']:
                    motelist.append(data)
            return [True, motelist]
        
        except OSError, e:
            print "execution failed:", e
            return [False, e]
    