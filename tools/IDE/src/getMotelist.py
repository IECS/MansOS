'''
Created on 2012. gada 24. febr.

@author: jj
'''
import os
from subprocess import Popen, PIPE, STDOUT

class GetMotelist(object):
    '''
    classdocs
    '''


    def __init__(self, pathToMansos, shellRegex):
        self.pathToMansos = pathToMansos
        self.shellRegex = shellRegex
        
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
    