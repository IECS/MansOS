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
from generate_makefile import GenerateMakefile

from globals import * #@UnusedWildImport

class DoUpload():
    def __init__(self, API):
        self.API = API
        self.generateMakefile = GenerateMakefile()
        self.editor = self.API.tabManager.getPageObject

    def doUpload(self):
        self.targets = list(self.API.targets)
        # If no target specified, use default
        if len(self.targets) == 0:
            self.targets.append(None)
        if self.API.targetType == SHELL:
            return self.shellUpload()
        else:
            os.unsetenv("BSLPORT")
            self.usbUpload()

    def usbUpload(self, data = None):
        self.curPath = os.getcwd()
        os.chdir(os.path.split(os.path.realpath(self.editor().filePath))[0])

        platform = self.API.platforms[self.API.activePlatform]
        if len(self.targets):
            target = self.targets.pop()
            self.changeTarget(target)
            self.API.startPopen(["make", platform, "upload"], "Upload", self.usbUpload, True)
        else:
            self.clean()

    def clean(self, data = None):
        self.API.startPopen(["make", "clean"], "Clean", None, False)
        os.chdir(self.curPath)

    def shellUpload(self, targets, platform):
        assert False, "Currently not supported"
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
                pass
                #upload = Popen(actualCmd, stdin = PIPE,
                #               stderr = STDOUT, stdout = PIPE)
                #
                #out = upload.communicate("program")[0]

                #if out.find("File uploaded") == -1:
                #    os.chdir(startDir)
                #    return [False, out]
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
            newTarget = self.API.motelist[newTarget][1]
            # Windows puts "\x00" at port end and it is not allowed in 
            # environment variable
            os.environ["BSLPORT"] = str(newTarget.strip().strip("\x00"))
