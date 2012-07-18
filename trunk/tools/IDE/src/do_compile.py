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

from os import chdir, path, getcwd
from generate_makefile import GenerateMakefile
from helperFunctions import doPopen
from myThread import MyThread

class DoCompile():
    def __init__(self, API):
        self.API = API
        self.generateMakefile = GenerateMakefile()
        self.editor = self.API.tabManager.getPageObject

    def doCompile(self):
        self.curPath = getcwd()
        chdir(path.split(path.realpath(self.editor().filePath))[0])

        self.generateMakefile.generate(self.editor().fileName,
                                       self.editor().projectType,
                                       self.API.pathToMansos)

        platform = self.API.getActivePlatform()
        thread = MyThread(doPopen, ["make", platform], \
                              self.dummy, True, False, "Compile")
        self.API.startThread(thread)

    def clean(self, data = None):
        self.API.startPopen(["make", "clean"], "Compile", None, False)
        chdir(self.curPath)

    # Passed to thread start as callback, because without callback 
    # there is no Done msg.
    def dummy(self, output):
        pass
