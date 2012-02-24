'''
Created on 2012. gada 24. febr.

@author: Janis Judvaitis
'''
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
            return [False, '']
        if targetType == g.SHELL:
            return self.shellUpload(targets, platform)
        else:
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
            