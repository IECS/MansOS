#!/usr/bin/env python

import os, sys

inputFileName = 'tests/91-comments.sl'

def importsOk():
    plyModuleOK = True     # Python Lex Yacc - for compilation
    
    try:
        import ply
    except ImportError:
        plyModuleOK = False

    if not plyModuleOK:
        if os.name == 'posix': 
            installStr = "Make sure you have installed required modules. Run:\n\tapt-get install"
        else: 
            installStr = "Make sure you have installed modules:"
            
        print "Cannot run SEAL parser:"

        if not plyModuleOK:
            print "\tPLY module not found"
            installStr += " python-ply"
            
        print installStr
        return False
    return True


def printLine(line):
    print line


def parseCommandLine(argv):
    global inputFileName
    if len(argv) > 1:
        inputFileName = argv[-1]
    # TODO

def main():
    if not importsOk():
        exit(1)

    parseCommandLine(sys.argv)

    # read file to-be-parsed
    with open(inputFileName, 'r') as inputFile:
        contents = inputFile.read()
    if contents == None:
        print 'Failed to read file', inputFileName
        exit(1)

    # import pathname where seal package is located
    sys.path.append('..')
    from seal import sealParser_yacc

    # parse the file
    parser = sealParser_yacc.SealParser(printLine)
    parser.run(contents)


if __name__ == '__main__':
    main()
