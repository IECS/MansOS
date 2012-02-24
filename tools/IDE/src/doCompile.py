'''
Created on 2012. gada 24. febr.

@author: Janis Judvaitis
'''

from os import chdir
from subprocess import Popen, PIPE, STDOUT

import generateMakefile

class DoCompile():
    def __init__(self, API):
        self.API = API
        self.generateMakefile = generateMakefile.GenerateMakefile()
    
    def doCompile(self, sourceFileName, platform, path, projectType, 
                  cleanAfter = True):
        
        goodPath = path.rfind('/')
        
        if goodPath != -1:
            chdir(path[:goodPath])
        else:
            chdir(self.API.path)
            
        self.generateMakefile.generate(sourceFileName, projectType, 
                                       self.API.path + "/../../")
        try:
            upload = Popen(["make", platform], 
                                      stderr = STDOUT,
                                      stdout = PIPE)
            out = upload.communicate()[0]
            
            if cleanAfter:
                self.clean()
                
            if out.rfind("saving Makefile.platform") == -1:
                return [False, out]
            else:
                return [True, out]

        except OSError, e:
            print "execution failed @ compile:", e
            return [e]
        
    def clean(self):
        clean = Popen(["make", "clean"], stderr = STDOUT, stdout = PIPE)
        clean.communicate()