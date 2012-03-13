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
from subprocess import Popen, PIPE, STDOUT

import doCompile
import globals as g

class DoUpload():
    def __init__(self, pathToMansos):
        self.compiler = doCompile
        self.pathToMansos = pathToMansos
    
    def doUpload(self, targets, compiler, targetType, platform):
        res = compiler()
        if res[0] == False:
            return res
        if targetType == g.SHELL:
            return self.shellUpload(targets, platform)
        else:
            os.unsetenv("BSLPORT")
            return self.usbUpload(targets, platform)
        
    def usbUpload(self, targets, platform):
        for target in targets:
            if target != []:
                self.changeTarget(target)
            try:
                upload = Popen(["make", platform, "upload"], 
                                          stderr = STDOUT,
                                          stdout = PIPE)
                out = upload.communicate()[0]

                haveUploadError = out.rfind("An error occoured")
                if haveUploadError != -1:
                    return [False, out[haveUploadError:]]
                
                haveCompileError = out.rfind("Error")
                if haveCompileError != -1:
                    return [False, out]

            except OSError, e:
                print "execution failed:", e
                return [False, e]
        
        return [True, '']
        
    def shellUpload(self, targets, platform):
        startDir = os.getcwd()
        os.chdir(self.pathToMansos + "/tools/shell")
        ihex = startDir + "/build/" + platform + "/image.ihex"
        if not os.path.exists(ihex) or not os.path.isfile(ihex):
            return [False, "Mistake with ihex file location."]
        
        cmd = ['./shell', '-l', ihex]
        for target in targets:
            actualCmd = cmd
            if target == None:
                target = "default device"
            else:
                actualCmd.append("-s")
                actualCmd.append(target)
            try:
                upload = Popen(actualCmd, stdin = PIPE, 
                               stderr = STDOUT, stdout = PIPE)
                
                out = upload.communicate("program")[0]
                
                if out.find("File uploaded") == -1:
                    os.chdir(startDir)
                    return [False, out]
            except OSError, e:
                print "execution failed:", e
                os.chdir(startDir)
                return [False, e]
            
        os.chdir(startDir)
        return [True, '']
    
    def changeTarget(self, newTarget = None):
        if newTarget == None:
            os.unsetenv("BSLPORT")
        else:
            os.environ["BSLPORT"] = newTarget
            