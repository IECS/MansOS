#!/usr/bin/env python

import os, sys

inputFileName = 'tests/91-comments.sl'
outputFileName = 'main.c'
architecture = 'test'

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
    sys.path.append('../seal/components')
    from seal import parser, codegen, structures

    # load available components
    structures.componentRegister.load(architecture)
    # parse input file (SEAL code)
    parser = parser.SealParser(printLine)
    parser.run(contents)
    # generate C code to an output file
    if outputFileName is None:
        codegen.generate(sys.stdout)
    else:
        with open(outputFileName, 'w') as outputFile:
            codegen.generate(outputFile)

if __name__ == '__main__':
    main()
